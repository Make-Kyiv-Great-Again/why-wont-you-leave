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
    inline DialogueTree CreateSapphireCubeDialogue() {
        DialogueTree tree;
        tree.startNodeId = 0;

        tree.nodes[0] = {
            0,
            "THE VOICE",
            "Ah, a curious wanderer approaches the Sapphire Prism. Can you hear the whispers vibrating across the floor?",
            1,
            {}
        };

        tree.nodes[1] = {
            1,
            "THE VOICE",
            "Tell me, traveler. What seekest thou in this boundless expanse of white?",
            -1,
            {
                { "I seek knowledge about this corridor.", 2 },
                { "I'm looking for an exit.", 3 },
                { "Just exploring randomly.", 4 }
            }
        };

        tree.nodes[2] = {
            2,
            "THE VOICE",
            "Knowledge is a heavy burden. The corridor connects echoes of rooms long forgotten...",
            5,
            {}
        };

        tree.nodes[3] = {
            3,
            "THE VOICE",
            "An exit? All doors lead forward, yet all lead back to where you started.",
            5,
            {}
        };

        tree.nodes[4] = {
            4,
            "THE VOICE",
            "Exploration without purpose often uncovers the most hidden truths.",
            5,
            {}
        };

        tree.nodes[5] = {
            5,
            "THE VOICE",
            "Remember this: not all who wander these chambers are truly alone.",
            -1,
            {}
        };

        return tree;
    }

    inline DialogueTree CreateEmeraldCubeDialogue() {
        DialogueTree tree;
        tree.startNodeId = 0;

        tree.nodes[0] = {
            0,
            "THE VOICE",
            "You touch the Emerald Relic. A soft, soothing hum resonates through your fingertips.",
            1,
            {}
        };

        tree.nodes[1] = {
            1,
            "THE VOICE",
            "Does the sheer silence of this chamber calm your spirit, or does it disturb you?",
            -1,
            {
                { "It feels quiet and peaceful.", 2 },
                { "It feels eerie and unnatural.", 3 }
            }
        };

        tree.nodes[2] = {
            2,
            "THE VOICE",
            "Peace is rare in these domains. Treasure this serenity before your next journey.",
            -1,
            {}
        };

        tree.nodes[3] = {
            3,
            "THE VOICE",
            "Unnatural? Perhaps. But fear is merely an illusion cast by unfamiliar spaces.",
            -1,
            {}
        };

        return tree;
    }

    inline DialogueTree CreateAmethystCubeDialogue() {
        DialogueTree tree;
        tree.startNodeId = 0;

        tree.nodes[0] = {
            0,
            "THE VOICE",
            "The Amethyst Core pulses with ancient energy. You have ventured deep into Room 2.",
            1,
            {}
        };

        tree.nodes[1] = {
            1,
            "THE VOICE",
            "Answer this riddle: which inner compass guides your path through the rooms?",
            -1,
            {
                { "1. Logic and Reason.", 2 },
                { "2. Instinct and Emotion.", 3 },
                { "3. Pure Luck.", 4 },
                { "4. Insatiable Curiosity.", 5 }
            }
        };

        tree.nodes[2] = {
            2,
            "THE VOICE",
            "A calculated mind builds sturdy bridges across the unknown.",
            -1,
            {}
        };

        tree.nodes[3] = {
            3,
            "THE VOICE",
            "Passion burns bright, though it can blind you to lurking shadows.",
            -1,
            {}
        };

        tree.nodes[4] = {
            4,
            "THE VOICE",
            "Fortune favors the bold, but tests the unprepared.",
            -1,
            {}
        };

        tree.nodes[5] = {
            5,
            "THE VOICE",
            "Curiosity is the flame that brought you here. Never allow it to be extinguished.",
            -1,
            {}
        };

        return tree;
    }

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
