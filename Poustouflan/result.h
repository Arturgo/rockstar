#ifndef RESULT_H
#define RESULT_H

#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Result {
public:
    std::string nom;
    std::vector<int> taille_membres;
    int nombre_membres;

    Result() = default;

    // Convert Result to JSON
    json to_json() const {
        return json{
            {"nom", nom},
            {"taille_membres", taille_membres},
            {"nombre_membres", nombre_membres}
        };
    }

    // Write Result to file
    void save_to_file(const std::string& filename) const {
        std::ofstream file(filename);
        file << to_json();
    }
};

#endif // RESULT_H
