#include <gtkmm.h>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <memory>
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
        nodes.emplace(id, make_shared<Node>(id));
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

            for (const string& characteristic : node.characteristics) {
                characteristicCounts[characteristic]++;
            }
        }

        cout << "Nodes that received the post:" << endl;
        for (int nodeId : reachedNodes) {
            cout << "Node " << nodeId << endl;
        }

        cout << "\nNodes that did not receive the post:" << endl;
        for (int nodeId : notReachedNodes) {
            cout << "Node " << nodeId << endl;
        }

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

        sort(dominanceLevels.begin(), dominanceLevels.end(),
             [](const pair<int, int>& a, const pair<int, int>& b) {
                 return a.second > b.second;
             });

        for (const auto& pair : nodes) {
            const Node& node = *(pair.second);
            for (const string& characteristic : node.characteristics) {
                characteristicInfluence[characteristic]++;
            }
        }

        cout << "\nDominance Levels:" << endl;
        for (const auto& pair : dominanceLevels) {
            cout << "Node " << pair.first << ": " << pair.second << " connections" << endl;
        }

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
        int lineNumber = 0;
        while (getline(file, line)) {
            lineNumber++;
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
    unordered_map<int, shared_ptr<Node>> nodes;
    unordered_map<int, unordered_set<int>> adjList;
};

// Global SocialNetwork instance
SocialNetwork network;

class MainWindow : public Gtk::Window {
public:
    MainWindow() {
        set_title("Social Network");
        set_default_size(400, 300);

        add(vbox);

        post_message_label.set_text("Enter keyword for post message:");
        vbox.pack_start(post_message_label);

        vbox.pack_start(post_message_entry);

        post_message_button.set_label("Post Message");
        post_message_button.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_post_message_clicked));
        vbox.pack_start(post_message_button);

        target_ads_label.set_text("Enter target characteristics (separated by spaces):");
        vbox.pack_start(target_ads_label);

        vbox.pack_start(target_ads_entry);

        target_ads_button.set_label("Target Ads");
        target_ads_button.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_target_ads_clicked));
        vbox.pack_start(target_ads_button);

        dominance_button.set_label("Calculate Dominance and Influence");
        dominance_button.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_dominance_clicked));
        vbox.pack_start(dominance_button);

        quit_button.set_label("Quit");
        quit_button.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_quit_clicked));
        vbox.pack_start(quit_button);

        show_all_children();
    }

protected:
    void on_post_message_clicked() {
        string keyword = post_message_entry.get_text();
        network.postMessage(keyword);
    }

    void on_target_ads_clicked() {
        string characteristicsStr = target_ads_entry.get_text();

        istringstream iss(characteristicsStr);
        unordered_set<string> targetCharacteristics;
        string characteristic;
        while (iss >> characteristic) {
            targetCharacteristics.insert(characteristic);
        }
        network.targetAds(targetCharacteristics);
    }

    void on_dominance_clicked() {
        network.calculateDominanceAndInfluence();
    }

    void on_quit_clicked() {
        hide();
    }

    Gtk::Box vbox{Gtk::ORIENTATION_VERTICAL};
    Gtk::Label post_message_label;
    Gtk::Entry post_message_entry;
    Gtk::Button post_message_button;

    Gtk::Label target_ads_label;
    Gtk::Entry target_ads_entry;
    Gtk::Button target_ads_button;

    Gtk::Button dominance_button;
    Gtk::Button quit_button;
};

int main(int argc, char *argv[]) {
    auto app = Gtk::Application::create(argc, argv, "org.gtkmm.example");

    network.readFromFile("nodes.txt");

    MainWindow window;
    return app->run(window);
}
