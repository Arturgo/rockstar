#ifndef INSTANCE_H
#define INSTANCE_H

#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>

using namespace std;
using json = nlohmann::json;

class Instance {
public:
    std::string nom;
    std::vector<std::string> membres;
    int score;

    Instance() = default;

    // Parse an Instance object from JSON
    static Instance from_json(const json& j) {
        Instance instance;
        j.at("nom").get_to(instance.nom);
        j.at("membres").get_to(instance.membres);
        j.at("score").get_to(instance.score);
        return instance;
    }

    // Load an Instance from a file
    static Instance load_from_file(const std::string& filename) {
        std::ifstream file(filename);
        json j;
        file >> j;
        return from_json(j);
    }
};

#endif // INSTANCE_H
