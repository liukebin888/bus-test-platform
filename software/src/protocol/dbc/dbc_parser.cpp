// dbc_parser.cpp - DBC database file parser (subset)
//
// Grammar subset (see dbc_parser.h):
//   BO_ <id> <name>: <dlc> <tx_node>
//   SG_ <name> [M|m<n>] : <start>|<len>@<order><sign> (<factor>,<offset>)
//        [<min>|<max>] "<unit>" <receivers>
// Everything else (VERSION / NS_ / BS_ / BU_ / CM_ / BA_ / VAL_) is
// skipped so real Vector exports load without choking.
#include "dbc_parser.h"

#include <cctype>
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "bus/bus_types.h"

namespace bt {

namespace {

std::string trim(const std::string& s) {
    const std::size_t b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) {
        return {};
    }
    const std::size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1U);
}

bool parse_u32(const std::string& tok, uint32_t* out) {
    if (tok.empty() || out == nullptr) {
        return false;
    }
    try {
        std::size_t pos = 0;
        const unsigned long v = std::stoul(tok, &pos, 0);  // base 0: 0x ok
        if (pos != tok.size()) {
            return false;
        }
        *out = static_cast<uint32_t>(v);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool parse_double(const std::string& tok, double* out) {
    if (tok.empty() || out == nullptr) {
        return false;
    }
    try {
        std::size_t pos = 0;
        *out = std::stod(tok, &pos);
        return pos == tok.size();
    } catch (const std::exception&) {
        return false;
    }
}

void set_error(std::string* error, const std::string& what) {
    if (error != nullptr) {
        *error = what;
    }
}

}  // namespace

bool DbcParser::parse(const std::string& text, std::string* error) {
    clear();
    std::istringstream iss(text);
    std::string line;
    while (std::getline(iss, line)) {
        const std::string t = trim(line);
        if (t.empty() || t.rfind("//", 0) == 0) {
            continue;
        }
        if (t.rfind("BO_ ", 0) == 0) {
            if (!parse_bo(t, error)) {
                return false;
            }
        } else if (t.rfind("SG_ ", 0) == 0) {
            if (messages_.empty()) {
                set_error(error, "SG_ line before any BO_ message");
                return false;
            }
            if (!parse_sg(t, &messages_.back(), error)) {
                return false;
            }
        }
        // Other keywords are tolerated and skipped.
    }
    return true;
}

bool DbcParser::parse_bo(const std::string& line, std::string* error) {
    // BO_ <id> <name>: <dlc> [<tx_node>]  (name and ':' may be glued)
    std::istringstream iss(line.substr(4));
    std::string id_tok;
    std::string name_tok;
    std::string dlc_tok;
    if (!(iss >> id_tok) || !(iss >> name_tok)) {
        set_error(error, "malformed BO_ line: " + line);
        return false;
    }
    if (!name_tok.empty() && name_tok.back() == ':') {
        name_tok.pop_back();  // "EngineData:"
    } else {
        std::string colon_tok;
        if (!(iss >> colon_tok) || colon_tok != ":") {
            set_error(error, "malformed BO_ line: " + line);
            return false;
        }
    }
    if (!(iss >> dlc_tok)) {
        set_error(error, "malformed BO_ line: " + line);
        return false;
    }

    DbcMessage m;
    if (!parse_u32(id_tok, &m.id)) {
        set_error(error, "bad BO_ id: " + id_tok);
        return false;
    }
    m.name = name_tok;
    uint32_t dlc = 0;
    if (!parse_u32(dlc_tok, &dlc) || dlc > BT_BUS_MAX_PAYLOAD) {
        set_error(error, "bad BO_ dlc: " + dlc_tok);
        return false;
    }
    m.dlc = static_cast<uint8_t>(dlc);
    if (iss >> m.tx_node) {
        // single-word tx node captured; multi-word names are rare
    }
    messages_.push_back(std::move(m));
    return true;
}

bool DbcParser::parse_sg(const std::string& line, DbcMessage* msg,
                         std::string* error) {
    // SG_ <name> [M | m<n>] : <start>|<len>@<order><sign> (<factor>,<offset>)
    //      [<min>|<max>] "<unit>" <receivers>
    if (msg == nullptr) {
        return false;
    }
    std::istringstream iss(line.substr(4));
    std::string name_tok;
    if (!(iss >> name_tok)) {
        set_error(error, "malformed SG_ line: " + line);
        return false;
    }

    if (!name_tok.empty() && name_tok.back() == ':') {
        name_tok.pop_back();  // "Name:" glued
    } else {
        std::string tok;
        if (iss >> tok) {
            if (tok == ":") {
                // no mux marker
            } else if (tok == "M" || (!tok.empty() && tok[0] == 'm')) {
                if (!(iss >> tok) || tok != ":") {
                    set_error(error, "malformed SG_ mux marker: " + line);
                    return false;
                }
            } else if (tok != ":") {
                set_error(error, "unexpected SG_ token: " + tok);
                return false;
            }
        } else {
            set_error(error, "SG_ line missing geometry: " + line);
            return false;
        }
    }

    Signal sig;
    sig.name = name_tok;

    std::string geom;
    if (!(iss >> geom)) {
        set_error(error, "SG_ line missing geometry: " + line);
        return false;
    }
    const std::size_t bar = geom.find('|');
    const std::size_t at = geom.find('@');
    if (bar == std::string::npos || at == std::string::npos || at < bar) {
        set_error(error, "bad SG_ geometry: " + geom);
        return false;
    }
    uint32_t start = 0;
    uint32_t len = 0;
    if (!parse_u32(geom.substr(0, bar), &start) ||
        !parse_u32(geom.substr(bar + 1U, at - bar - 1U), &len)) {
        set_error(error, "bad SG_ start/len: " + geom);
        return false;
    }
    sig.start_bit = static_cast<uint8_t>(start & 0xFFU);
    sig.length = (len > 64U) ? 64U : static_cast<uint8_t>(len);
    if (at + 2U >= geom.size()) {
        set_error(error, "bad SG_ byte order/sign: " + geom);
        return false;
    }
    const char order = geom[at + 1U];
    const char sign = geom[at + 2U];
    sig.little_endian = (order == '0');
    sig.is_signed = (sign == '-');

    std::string phys;
    if (!(iss >> phys)) {
        set_error(error, "SG_ line missing factor/offset: " + line);
        return false;
    }
    const std::size_t lp = phys.find('(');
    const std::size_t rp = phys.find(')');
    if (lp == std::string::npos || rp == std::string::npos || rp <= lp) {
        set_error(error, "bad SG_ factor/offset: " + phys);
        return false;
    }
    const std::string inner = phys.substr(lp + 1U, rp - lp - 1U);
    const std::size_t comma = inner.find(',');
    if (comma == std::string::npos ||
        !parse_double(inner.substr(0, comma), &sig.factor) ||
        !parse_double(inner.substr(comma + 1U), &sig.offset)) {
        set_error(error, "bad SG_ factor/offset: " + phys);
        return false;
    }

    // Optional [min|max] range.
    std::string range;
    if (iss >> range && !range.empty() && range.front() == '[') {
        const std::size_t rb = range.find(']');
        if (rb != std::string::npos) {
            const std::string inner2 = range.substr(1U, rb - 1U);
            const std::size_t bar2 = inner2.find('|');
            if (bar2 != std::string::npos) {
                double mn = 0.0;
                double mx = 0.0;
                if (parse_double(inner2.substr(0, bar2), &mn) &&
                    parse_double(inner2.substr(bar2 + 1U), &mx)) {
                    sig.min_value = mn;
                    sig.max_value = mx;
                }
            }
        }
    }

    // Optional "<unit>" (may contain spaces) - read the line remainder.
    std::string rest;
    std::getline(iss, rest);
    rest = trim(rest);
    if (!rest.empty() && rest.front() == '"') {
        const std::size_t q = rest.find('"', 1U);
        if (q != std::string::npos) {
            sig.unit = rest.substr(1U, q - 1U);
        }
    }

    msg->signals.push_back(std::move(sig));
    return true;
}

const DbcMessage* DbcParser::find_message(uint32_t id) const {
    for (const DbcMessage& m : messages_) {
        if (m.id == id) {
            return &m;
        }
    }
    return nullptr;
}

const DbcMessage* DbcParser::find_message(const std::string& name) const {
    for (const DbcMessage& m : messages_) {
        if (m.name == name) {
            return &m;
        }
    }
    return nullptr;
}

void DbcParser::clear() { messages_.clear(); }

}  // namespace bt
