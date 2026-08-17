// dbc_parser.h - DBC database file parser (subset)
//
// Supports the canonical Vector DBC grammar subset needed by REQ-BUS-002:
//   BO_ <id> <name>: <dlc> <tx_node>
//   SG_ <name> [mux] : <start>|<len>@<byteorder><sign> (<factor>,<offset>)
//        [<min>|<max>] "<unit>" <receivers>
//   CM_ / BA_ / VAL_ are parsed leniently (skipped) so real files load.
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "core/signal.h"

namespace bt {

struct DbcMessage {
    uint32_t id = 0;
    std::string name;
    uint8_t dlc = 0;
    std::string tx_node;
    std::vector<Signal> signals;
};

class DbcParser {
public:
    // Parse DBC text. Returns false and fills *error on structural failure.
    bool parse(const std::string& text, std::string* error = nullptr);

    const std::vector<DbcMessage>& messages() const { return messages_; }
    size_t message_count() const { return messages_.size(); }

    const DbcMessage* find_message(uint32_t id) const;
    const DbcMessage* find_message(const std::string& name) const;

    void clear();

private:
    bool parse_bo(const std::string& line, std::string* error);
    bool parse_sg(const std::string& line, DbcMessage* msg, std::string* error);

    std::vector<DbcMessage> messages_;
};

}  // namespace bt
