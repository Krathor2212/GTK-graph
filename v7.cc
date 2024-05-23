/*
Compile using [g++ v4.cpp -o gui `pkg-config --cflags --libs gtkmm-3.0`]
*/

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
        for (const auto& characteristic : characteristics) {
            availableCharacteristics.insert(characteristic);
        }
    }

    void addEdge(int id1, int id2) {
        adjList[id1].insert(id2);
        adjList[id2].insert(id1);
    }

    vector<pair<int, string>> postMessage(const string& keyword) {
        vector<pair<int, string>> result;
        unordered_set<int> reachedNodes;
        unordered_set<int> notReachedNodes;

        for (const auto& pair : nodes) {
            const Node& node = *(pair.second);
            if (node.characteristics.count(keyword)) {
                reachedNodes.insert(node.id);
            } else {
                notReachedNodes.insert(node.id);
            }
        }

        for (int nodeId : reachedNodes) {
            result.push_back({nodeId, "Received"});
        }

        for (int nodeId : notReachedNodes) {
            result.push_back({nodeId, "Not Received"});
        }

        return result;
    }

    vector<int> targetAds(const unordered_set<string>& targetCharacteristics) {
        vector<int> result;
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
                result.push_back(node.id);
            }
        }
        return result;
    }

    vector<pair<int, vector<int>>> calculateDominanceAndInfluence(const unordered_set<string>& targetCharacteristics = {}) {
        vector<pair<int, vector<int>>> dominanceLevels;

        for (const auto& pair : adjList) {
            int node = pair.first;
            if (!targetCharacteristics.empty()) {
                const Node& nodeObj = *(nodes[node]);
                bool matchesTarget = true;
                for (const string& target : targetCharacteristics) {
                    if (nodeObj.characteristics.find(target) == nodeObj.characteristics.end()) {
                        matchesTarget = false;
                        break;
                    }
                }
                if (!matchesTarget) {
                    continue;
                }
            }

            vector<int> connections(pair.second.begin(), pair.second.end());
            dominanceLevels.push_back({node, connections});
        }

        sort(dominanceLevels.begin(), dominanceLevels.end(),
             [](const pair<int, vector<int>>& a, const pair<int, vector<int>>& b) {
                 return a.second.size() > b.second.size();
             });

        return dominanceLevels;
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

    const unordered_set<string>& getAvailableCharacteristics() const {
        return availableCharacteristics;
    }

private:
    unordered_map<int, shared_ptr<Node>> nodes;
    unordered_map<int, unordered_set<int>> adjList;
    unordered_set<string> availableCharacteristics;
};

SocialNetwork network;

class MainWindow : public Gtk::Window {
public:
    MainWindow() {
        set_title("Social Network");
        set_default_size(600,200);

        add(grid);

        // Post message section
        post_message_label.set_text("Enter keyword for post message:");
        grid.attach(post_message_label, 0, 0, 1, 1);

        grid.attach(post_message_entry, 1, 0, 1, 1);

        post_message_button.set_label("Post Message");
        post_message_button.set_name("post_message");
        post_message_button.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_post_message_clicked));
        grid.attach(post_message_button, 2, 0, 1, 1);

        // Target ads section
        target_ads_label.set_text("Select or enter target characteristics:");
        grid.attach(target_ads_label, 0, 1, 1, 1);

        grid.attach(target_ads_combobox, 1, 1, 1, 1);
        grid.attach(target_ads_entry, 1, 2, 1, 1);

        target_ads_button.set_label("Target Ads");
        target_ads_button.set_name("target_ads");
        target_ads_button.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_target_ads_clicked));
        grid.attach(target_ads_button, 2, 1, 1, 1);

        // Populate combobox with available characteristics
        for (const auto& characteristic : network.getAvailableCharacteristics()) {
            target_ads_combobox.append(characteristic);
        }

        // Dominance section
        dominance_button.set_label("Calculate Dominance and Influence");
        dominance_button.set_name("dominance");
        dominance_button.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_dominance_clicked));
        grid.attach(dominance_button, 0, 3, 3, 1);

        // Quit button
        quit_button.set_label("Quit");
        quit_button.set_name("quit");
        quit_button.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_quit_clicked));
        grid.attach(quit_button, 0, 4, 3, 1);

        // Results section
        grid.attach(scrolled_window, 0, 10, 3, 1);
        scrolled_window.add(tree_view);
        scrolled_window.set_min_content_height(300);
        list_store = Gtk::ListStore::create(columns);
        tree_view.set_model(list_store);

        tree_view.append_column("Node ID", columns.col_id);
        tree_view.append_column("Status/Connections", columns.col_status);

        apply_css("stl.css");

        show_all_children();
    }

protected:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        ModelColumns() {
            add(col_id);
            add(col_status);
        }

        Gtk::TreeModelColumn<int> col_id;
        Gtk::TreeModelColumn<Glib::ustring> col_status;
    };

    void on_post_message_clicked() {
        string keyword = post_message_entry.get_text();
        auto result = network.postMessage(keyword);

        list_store->clear();
        for (const auto& entry : result) {
            Gtk::TreeModel::Row row = *(list_store->append());
            row[columns.col_id] = entry.first;
            row[columns.col_status] = entry.second;
        }
    }

    void on_target_ads_clicked() {
        string characteristicsStr = target_ads_entry.get_text();
        string selectedCharacteristic = target_ads_combobox.get_active_text();

        unordered_set<string> targetCharacteristics;
        if (!characteristicsStr.empty()) {
            istringstream iss(characteristicsStr);
            string characteristic;
            while (iss >> characteristic) {
                targetCharacteristics.insert(characteristic);
            }
        } else if (!selectedCharacteristic.empty()) {
            targetCharacteristics.insert(selectedCharacteristic);
        }

        auto result = network.targetAds(targetCharacteristics);

        list_store->clear();
        for (int nodeId : result) {
            Gtk::TreeModel::Row row = *(list_store->append());
            row[columns.col_id] = nodeId;
            row[columns.col_status] = "Targeted";
        }
    }

    void on_dominance_clicked() {
        string characteristicsStr = target_ads_entry.get_text();
        string selectedCharacteristic = target_ads_combobox.get_active_text();

        unordered_set<string> targetCharacteristics;
        if (!characteristicsStr.empty()) {
            istringstream iss(characteristicsStr);
            string characteristic;
            while (iss >> characteristic) {
                targetCharacteristics.insert(characteristic);
            }
        } else if (!selectedCharacteristic.empty()) {
            targetCharacteristics.insert(selectedCharacteristic);
        }

        auto result = network.calculateDominanceAndInfluence(targetCharacteristics);

        list_store->clear();
        for (const auto& entry : result) {
            Gtk::TreeModel::Row row = *(list_store->append());
            row[columns.col_id] = entry.first;
            stringstream ss;
            for (int conn : entry.second) {
                ss << conn << " ";
            }
            row[columns.col_status] = ss.str();
        }
    }

    void on_quit_clicked() {
        hide();
    }

    void apply_css(const std::string& css_file) {
        Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
        css_provider->load_from_path(css_file);
        Glib::RefPtr<Gtk::StyleContext> style_context = get_style_context();
        style_context->add_provider_for_screen(Gdk::Screen::get_default(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
    }

private:
    Gtk::Grid grid;

    Gtk::Label post_message_label;
    Gtk::Entry post_message_entry;
    Gtk::Button post_message_button;

    Gtk::Label target_ads_label;
    Gtk::ComboBoxText target_ads_combobox;
    Gtk::Entry target_ads_entry;
    Gtk::Button target_ads_button;

    Gtk::Button dominance_button;
    Gtk::Button quit_button;

    Gtk::ScrolledWindow scrolled_window;
    Gtk::TreeView tree_view;
    Glib::RefPtr<Gtk::ListStore> list_store;
    ModelColumns columns;
};

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create(argc, argv, "org.gtkmm.example");

    // Read the social network data from the file
    network.readFromFile("nodes.txt");

    MainWindow window;

    return app->run(window);
}
