#ifndef INSTANCE_H
#define INSTANCE_H

#include <algorithm>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>
#include <memory>
#include <stdexcept>
#include <iostream>

using namespace std;
using json = nlohmann::json;

// Forward declaration of Shop to use in Constraint class
class Shop;

struct Shop {
    std::string name;
    int resequencing_lag;

    static Shop from_json(const json& j) {
        Shop shop;
        j.at("name").get_to(shop.name);
        j.at("resequencing_lag").get_to(shop.resequencing_lag);
        return shop;
    }
};

struct Vehicle {
    int id;
    bool two_tone;

    static Vehicle from_json(const json& j) {
        Vehicle vehicle;
        j.at("id").get_to(vehicle.id);
        vehicle.id--;
        std::string type;
        j.at("type").get_to(type);
        vehicle.two_tone = (type == "two-tone");
        return vehicle;
    }
};

// Base Constraint class
class Constraint {
public:
    int id;
    std::string type;
    Shop* shop;
    int cost;

    virtual ~Constraint() = default;

    void from_json_base(const json& j, const std::vector<Shop>& all_shops) {
        j.at("id").get_to(id);
        j.at("type").get_to(type);
        j.at("cost").get_to(cost);

        std::string shop_name;
        j.at("shop").get_to(shop_name);
        for (const auto& s : all_shops) {
            if (s.name == shop_name) {
                shop = const_cast<Shop*>(&s);
                return;
            }
        }
        throw std::runtime_error("Shop not found: " + shop_name);
    }

    virtual int get_cost(const vector<int> cars) = 0;
    virtual void from_json(const json& j, const std::vector<Shop>& all_shops, int n) = 0;
    static std::shared_ptr<Constraint> create_constraint(const json& j, const std::vector<Shop>& all_shops, int n);
};

class BatchSizeConstraint : public Constraint {
public:
    int min_vehicles;
    int max_vehicles;
    std::vector<bool> vehicles;

    void from_json(const json& j, const std::vector<Shop>& all_shops, int n) override {
        from_json_base(j, all_shops);
        j.at("min_vehicles").get_to(min_vehicles);
        j.at("max_vehicles").get_to(max_vehicles);

        std::vector<int> raw_vehicles;
        j.at("vehicles").get_to(raw_vehicles);
        vehicles.resize(n);

        for (int v : raw_vehicles)
            vehicles[v - 1] = true;
    }

    int get_cost(const vector<int> cars) {
        int batch = 0;
        int current_cost = 0;
        for (int car : cars) {
            if (vehicles[car]) {
                batch += 1;
            } else if (batch != 0) {
                int d = max(0, max(min_vehicles - batch, batch - max_vehicles));
                current_cost += d * d;
                batch = 0;
            }
        }
        if (batch > 0) {
            int d = max(0, max(min_vehicles - batch, batch - max_vehicles));
            current_cost += d * d;
        }
        return current_cost * cost;
    }
};

class LotChangeConstraint : public Constraint {
public:
    std::vector<int> partition; // partition[i] = partition[j] iff vehicles (i, j) same group

    void from_json(const json& j, const std::vector<Shop>& all_shops, int n) override {
        from_json_base(j, all_shops);

        std::vector<std::vector<int>> raw_partition;
        j.at("partition").get_to(raw_partition);

        partition.resize(n);
        for (int i = 0; i < raw_partition.size(); i++)
        {
            for (int vehicle : raw_partition[i])
            {
                partition[vehicle - 1] = i;
            }
        }
    }

    int get_cost(const vector<int> cars) {
        int current_cost = 0;
        int lot = partition[cars[0]];
        for (int car : cars) {
            int lot2 = partition[car];
            if (lot2 != lot) {
                current_cost++;
            };
            lot = lot2;
        }
        return cost * current_cost;
    }
};

class RollingWindowConstraint : public Constraint {
public:
    int window_size;
    int max_vehicles;
    std::vector<bool> vehicles;

    void from_json(const json& j, const std::vector<Shop>& all_shops, int n) override {
        from_json_base(j, all_shops);  // Use base class to parse common fields
        j.at("window_size").get_to(window_size);
        j.at("max_vehicles").get_to(max_vehicles);

        std::vector<int> raw_vehicles;
        j.at("vehicles").get_to(raw_vehicles);
        vehicles.resize(n);

        for (int v : raw_vehicles)
            vehicles[v - 1] = true;
    }

    int get_cost(const vector<int> cars) {
        int current_cost = 0;
        int nb_in_window = 0;
        for (int i = 0; i < window_size; i++) {
            if (vehicles[cars[i]]) {
                nb_in_window += 1;
            }
        }
        if (nb_in_window > max_vehicles) {
            current_cost += (nb_in_window - max_vehicles) * (nb_in_window - max_vehicles);
        }
        for (int i = window_size; i < cars.size(); i++) {
            if (vehicles[cars[i]]) {
                nb_in_window += 1;
            }
            if (vehicles[cars[i - window_size]]) {
                nb_in_window -= 1;
            }
            if (nb_in_window > max_vehicles) {
                current_cost += (nb_in_window - max_vehicles) * (nb_in_window - max_vehicles);
            }
        }
        return current_cost * cost;
    }
};

struct Instance {
    std::vector<Shop> shops;
    int n;
    int two_tone_delta;
    int resequencing_cost;
    std::vector<Vehicle> vehicles;
    std::vector<std::shared_ptr<Constraint>> constraints;

    Instance() = default;

    static Instance from_json(const json& j) {
        Instance instance;

        // Load shops
        for (const auto& shop_json : j.at("shops")) {
            instance.shops.push_back(Shop::from_json(shop_json));
        }

        instance.two_tone_delta = j.at("parameters").at("two_tone_delta").get<int>();
        instance.resequencing_cost = j.at("parameters").at("resequencing_cost").get<int>();

        instance.n = j.at("vehicles").size();
        instance.vehicles.resize(instance.n);

        // Load vehicles
        for (const auto& vehicle_json : j.at("vehicles"))
        {
            Vehicle vehicle = Vehicle::from_json(vehicle_json);
            instance.vehicles[vehicle.id] = vehicle;
        }

        // Load constraints and associate with shops
        for (const auto& constraint_json : j.at("constraints")) {
            instance.constraints.push_back(Constraint::create_constraint(constraint_json, instance.shops, instance.n));
        }

        return instance;
    }

    static Instance load_from_file(const std::string& filename) {
        std::ifstream file(filename);
        json j;
        file >> j;
        return from_json(j);
    }
};

#endif /* !INSTANCE_H */
