// workspace.h - Object model container (Bus -> Channel -> Node + databases)
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "core/channel.h"
#include "core/node.h"

namespace bt {

// Holds channels / nodes; later grows to own DBC/LDF/ODX resource manager
// (v3.0: Database 管理器 with versioned resources).
class Workspace {
public:
    Channel* add_channel(uint8_t id, bt_bus_type_t type,
                         const std::string& name);
    Channel* find_channel(uint8_t id);
    const Channel* find_channel(uint8_t id) const;
    std::vector<Channel*> channels();
    size_t channel_count() const { return channels_.size(); }

    Node* add_node(const std::string& name, uint8_t channel_id,
                   const std::string& script_path = {});
    Node* find_node(const std::string& name);
    size_t node_count() const { return nodes_.size(); }

    void reset_all_stats();

private:
    std::vector<std::unique_ptr<Channel>> channels_;
    std::vector<std::unique_ptr<Node>> nodes_;
};

}  // namespace bt
