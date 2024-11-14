#ifndef RESULT_H
#define RESULT_H

#include <iostream>
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

    int compute_score(const Instance& input) {
        int resequencing_cost = 0;
        for (int i = 1; i < input.shops.size(); i++) {
            Shop exited_shop = input.shops[i - 1];
            string shop_out = exited_shop.name;
            string shop_in = input.shops[i].name;
            int lag = input.shops[i].resequencing_lag;
            const ShopSolution& cars_out = shop_results.find(shop_out)->second;
            const ShopSolution& cars_in = shop_results.find(shop_in)->second;
            vector<int> timer_out;
            timer_out.resize(input.n);
            for (int i = 0; i < cars_out.exit.size(); i++) {
                timer_out[cars_out.exit[i]] = i;
            }
            for (int i = 0; i < cars_in.entry.size(); i++) {
                int time_out = timer_out[cars_in.entry[i]];
                resequencing_cost += max(0, i - exited_shop.resequencing_lag - time_out);
            }
        }
        int total_cost = resequencing_cost * input.resequencing_cost;
        for (auto constraint : input.constraints) {
            const vector<int>& cars = shop_results.find(constraint->shop->name)->second.entry;
            int c = constraint->get_cost(cars);
            total_cost += c;
        }
        return total_cost;
    };
};

#endif /* ! RESULT_H */
