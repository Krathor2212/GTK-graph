#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <memory> // For smart pointers
#include <vector>
#include <algorithm>

using namespace std;

class Node {
public:
    int id;
    unordered_set<string> characteristics;

    Node(int id) : id(id) {}
};

class SocialNetwork {
public:
    void addNode(int id, const unordered_set<string>& characteristics) {
        nodes.emplace(id, make_shared<Node>(id)); // Using smart pointer
        nodes[id]->characteristics = characteristics;
    }

    void addEdge(int id1, int id2) {
        adjList[id1].insert(id2);
        adjList[id2].insert(id1);
    }

    void postMessage(const string& keyword) {
        unordered_set<int> reachedNodes;
        unordered_set<int> notReachedNodes;
        unordered_map<string, int> characteristicCounts;

        for (const auto& pair : nodes) {
            const Node& node = *(pair.second);
            if (node.characteristics.count(keyword)) {
                reachedNodes.insert(node.id);
            } else {
                notReachedNodes.insert(node.id);
            }

            // Update characteristic counts
            for (const string& characteristic : node.characteristics) {
                characteristicCounts[characteristic]++;
            }
        }

        // Print nodes that received the post
        cout << "Nodes that received the post:" << endl;
        for (int nodeId : reachedNodes) {
            cout << "Node " << nodeId << endl;
        }

        // Print nodes that did not receive the post
        cout << "\nNodes that did not receive the post:" << endl;
        for (int nodeId : notReachedNodes) {
            cout << "Node " << nodeId << endl;
        }

        // Print reach count by characteristics
        cout << "\nReach count by characteristics:" << endl;
        for (const auto& pair : characteristicCounts) {
            cout << pair.first << ": " << pair.second << endl;
        }
    }

    void targetAds(const unordered_set<string>& targetCharacteristics) {
        cout << "\nTargeted Ads based on Characteristics:" << endl;
        for (const auto& pair : nodes) {
            const Node& node = *(pair.second);
            bool matchesTarget = true;
            for (const string& target : targetCharacteristics) {
                if (node.characteristics.find(target) == node.characteristics.end()) {
                    matchesTarget = false;
                    break;
                }
            }
            if (matchesTarget) {
                cout << "Node " << node.id << " matches the target characteristics." << endl;
                // You can implement code here to display or record targeted ads for this node
            }
        }
    }

     void calculateDominanceAndInfluence() {
        vector<pair<int, int>> dominanceLevels;
        unordered_map<string, int> characteristicInfluence;

        for (const auto& pair : adjList) {
            int node = pair.first;
            int connections = pair.second.size();
            dominanceLevels.push_back({node, connections});
        }

        // Sort nodes by connection count
        sort(dominanceLevels.begin(), dominanceLevels.end(),
             [](const pair<int, int>& a, const pair<int, int>& b) {
                 return a.second > b.second;
             });

        // Calculate influence levels based on characteristics
        for (const auto& pair : nodes) {
            const Node& node = *(pair.second);
            for (const string& characteristic : node.characteristics) {
                characteristicInfluence[characteristic]++;
            }
        }

        // Print dominance levels
        cout << "\nDominance Levels:" << endl;
        for (const auto& pair : dominanceLevels) {
            cout << "Node " << pair.first << ": " << pair.second << " connections" << endl;
        }

        // Print influence levels by characteristics
        cout << "\nInfluence Levels by Characteristics:" << endl;
        for (const auto& pair : characteristicInfluence) {
            cout << pair.first << ": " << pair.second << endl;
        }
    }

    void readFromFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error opening file: " << filename << endl;
            return;
        }

        string line;
        bool readingNodes = true;
        int lineNumber = 0; // Track line number for debugging
        while (getline(file, line)) {
            lineNumber++; // Increment line number
            if (line == "edges") {
                readingNodes = false;
                continue;
            }

            istringstream iss(line);
            if (readingNodes) {
                int id;
                if (!(iss >> id)) {
                    cerr << "Error reading node ID at line " << lineNumber << endl;
                    continue;
                }
                unordered_set<string> characteristics;
                string characteristic;
                while (iss >> characteristic) {
                    characteristics.insert(characteristic);
                }
                addNode(id, characteristics);
            } else {
                int id1, id2;
                if (!(iss >> id1 >> id2)) {
                    cerr << "Error reading edge at line " << lineNumber << endl;
                    continue;
                }
                addEdge(id1, id2);
            }
        }
    }

private:
    unordered_map<int, shared_ptr<Node>> nodes; // Smart pointer
    unordered_map<int, unordered_set<int>> adjList;
};

int main() {
    SocialNetwork network;

    // Read from file
    network.readFromFile("nodes.txt");

    // Menu driven program
    bool exitProgram = false;
    while (!exitProgram) {
        int choice;
        cout << "\nMenu:\n";
        cout << "1. Post message\n";
        cout << "2. Target Ads based on Characteristics\n";
        cout << "3. Exit program\n";
        cout << "4. Dominance\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1: {
                string keyword;
                cout << "Enter the keyword: ";
                cin >> keyword;
                network.postMessage(keyword);
                break;
            }

            case 2: {
                cout << "Enter target characteristics separated by spaces: ";
                string characteristicsStr;
                getline(cin >> ws, characteristicsStr);
                istringstream iss(characteristicsStr);
                unordered_set<string> targetCharacteristics;
                string characteristic;
                while (iss >> characteristic) {
                    targetCharacteristics.insert(characteristic);
                }
                network.targetAds(targetCharacteristics);
                break;
            }

            case 3: {
                exitProgram = true;
                break;
            }

            case 4: {
                network.calculateDominanceAndInfluence();
                break;
            }

            default: {
                cout << "Invalid choice. Please try again.\n";
                break;
            }
        }
    }

    return 0;
}
