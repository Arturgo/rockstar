#ifndef RESULT_H
#define RESULT_H

#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct ShopSolution {
    std::vector<int> entry;
    std::vector<int> exit;

    json to_json() const {
        std::vector<int> shifted_entry;
        std::vector<int> shifted_exit;
        // 1 - index
        for (int i: entry)
            shifted_entry.push_back(i+1);
        for (int j: exit)
            shifted_exit.push_back(j+1);
        return json{
            {"entry", shifted_entry},
            {"exit", shifted_exit}
        };
    }
};

struct Solution {
    std::unordered_map<std::string, ShopSolution> shop_results;

    json to_json() const {
        json j;
        for (const auto& [shop_name, shop_result] : shop_results) {
            j[shop_name] = shop_result.to_json();
        }
        return j;
    }

    void save_to_file(const std::string& filename) const {
        std::ofstream file(filename);
        if (file.is_open()) {
            file << to_json().dump(4);
            file.close();
        }
    }
};

#endif /* ! RESULT_H */
