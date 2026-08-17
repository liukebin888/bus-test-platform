#include "core/node.h"

namespace bt {

Node::Node(std::string name, uint8_t channel_id, std::string script_path)
    : name_(std::move(name)),
      channel_id_(channel_id),
      script_path_(std::move(script_path)) {}

}  // namespace bt
