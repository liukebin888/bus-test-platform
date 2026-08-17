// node.h - Network node object model (Simulation Setup: Bus -> Node -> Signal)
#pragma once

#include <cstdint>
#include <string>

namespace bt {

class Node {
public:
    Node(std::string name, uint8_t channel_id, std::string script_path = {});

    const std::string& name() const { return name_; }
    uint8_t channel_id() const { return channel_id_; }
    const std::string& script_path() const { return script_path_; }

    void set_script_path(std::string p) { script_path_ = std::move(p); }
    void set_active(bool active) { active_ = active; }
    bool active() const { return active_; }

private:
    std::string name_;
    uint8_t channel_id_;
    std::string script_path_;
    bool active_ = true;
};

}  // namespace bt
