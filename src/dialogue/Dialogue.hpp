#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

struct DialogueOption {
    std::string text;
    int targetNodeId;
};

struct DialogueNode {
    int id;
    std::string speaker;
    std::string text;
    int nextNodeId; // -1 means end dialogue if options is empty
    std::vector<DialogueOption> options;
    bool isBlackout = false;
    bool isMemory = false;
};

struct DialogueTree {
    int startNodeId = 0;
    std::unordered_map<int, DialogueNode> nodes;

    const DialogueNode* GetNode(int id) const {
        auto it = nodes.find(id);
        if (it != nodes.end()) {
            return &it->second;
        }
        return nullptr;
    }
};

namespace Dialogues {
    inline DialogueTree FromJson(const nlohmann::json& j) {
        DialogueTree tree;
        if (j.contains("start_node_id")) {
            tree.startNodeId = j["start_node_id"].get<int>();
        }
        if (j.contains("nodes") && j["nodes"].is_array()) {
            for (const auto& nodeJson : j["nodes"]) {
                DialogueNode node;
                node.id = nodeJson.value("id", 0);
                node.speaker = nodeJson.value("speaker", "");
                node.text = nodeJson.value("text", "");
                node.nextNodeId = nodeJson.value("next_id", -1);
                node.isBlackout = nodeJson.value("is_blackout", false);
                node.isMemory = nodeJson.value("is_memory", node.isBlackout);

                if (nodeJson.contains("options") && nodeJson["options"].is_array()) {
                    for (const auto& optJson : nodeJson["options"]) {
                        DialogueOption opt;
                        opt.text = optJson.value("text", "");
                        opt.targetNodeId = optJson.value("target_id", -1);
                        node.options.push_back(opt);
                    }
                }
                tree.nodes[node.id] = node;
            }
        }
        return tree;
    }
}
