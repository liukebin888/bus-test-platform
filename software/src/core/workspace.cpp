#include "core/workspace.h"

namespace bt {

Channel* Workspace::add_channel(uint8_t id, bt_bus_type_t type,
                                const std::string& name) {
    if (find_channel(id) != nullptr) {
        return nullptr;  // duplicate id rejected
    }
    channels_.push_back(std::make_unique<Channel>(id, type, name));
    return channels_.back().get();
}

Channel* Workspace::find_channel(uint8_t id) {
    for (auto& c : channels_) {
        if (c->id() == id) return c.get();
    }
    return nullptr;
}

const Channel* Workspace::find_channel(uint8_t id) const {
    for (const auto& c : channels_) {
        if (c->id() == id) return c.get();
    }
    return nullptr;
}

std::vector<Channel*> Workspace::channels() {
    std::vector<Channel*> out;
    out.reserve(channels_.size());
    for (auto& c : channels_) out.push_back(c.get());
    return out;
}

Node* Workspace::add_node(const std::string& name, uint8_t channel_id,
                          const std::string& script_path) {
    if (find_node(name) != nullptr) {
        return nullptr;
    }
    nodes_.push_back(
        std::make_unique<Node>(name, channel_id, script_path));
    return nodes_.back().get();
}

Node* Workspace::find_node(const std::string& name) {
    for (auto& n : nodes_) {
        if (n->name() == name) return n.get();
    }
    return nullptr;
}

void Workspace::reset_all_stats() {
    for (auto& c : channels_) c->reset_stats();
}

}  // namespace bt
