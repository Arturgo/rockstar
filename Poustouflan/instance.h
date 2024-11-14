#ifndef INSTANCE_H
#define INSTANCE_H

#include <algorithm>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>
#include <memory>
#include <stdexcept>

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

    virtual void from_json(const json& j, const std::vector<Shop>& all_shops) = 0;
    static std::shared_ptr<Constraint> create_constraint(const json& j, const std::vector<Shop>& all_shops);
};

class BatchSizeConstraint : public Constraint {
public:
    int min_vehicles;
    int max_vehicles;
    std::vector<int> vehicles;

    void from_json(const json& j, const std::vector<Shop>& all_shops) override {
        from_json_base(j, all_shops);
        j.at("min_vehicles").get_to(min_vehicles);
        j.at("max_vehicles").get_to(max_vehicles);
        j.at("vehicles").get_to(vehicles);
        std::sort(vehicles.begin(), vehicles.end());
    }
};

class LotChangeConstraint : public Constraint {
public:
    std::vector<int> partition; // partition[i] = partition[j] iff vehicles (i, j) same group

    void from_json(const json& j, const std::vector<Shop>& all_shops) override {
        from_json_base(j, all_shops);

        std::vector<std::vector<int>> raw_partition;
        j.at("partition").get_to(raw_partition);

        for (int i = 0; i < raw_partition.size(); i++)
        {
            for (int vehicle : raw_partition[i])
            {
                if (partition.size() < vehicle)
                    partition.resize(vehicle); // ugly
                partition[vehicle - 1] = i;
            }
        }
    }
};

class RollingWindowConstraint : public Constraint {
public:
    int window_size;
    int max_vehicles;
    std::vector<int> vehicles;

    void from_json(const json& j, const std::vector<Shop>& all_shops) override {
        from_json_base(j, all_shops);  // Use base class to parse common fields
        j.at("window_size").get_to(window_size);
        j.at("max_vehicles").get_to(max_vehicles);
        j.at("vehicles").get_to(vehicles);
        std::sort(vehicles.begin(), vehicles.end());
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
            instance.constraints.push_back(Constraint::create_constraint(constraint_json, instance.shops));
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
