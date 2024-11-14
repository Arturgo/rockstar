#ifndef INSTANCE_H
#define INSTANCE_H

#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>
#include <memory>

using namespace std;
using json = nlohmann::json;

// Shop struct
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

// Vehicle struct
struct Vehicle {
    int id;
    std::string type;

    static Vehicle from_json(const json& j) {
        Vehicle vehicle;
        j.at("id").get_to(vehicle.id);
        j.at("type").get_to(vehicle.type);
        return vehicle;
    }
};

// Base Constraint class
class Constraint {
public:
    int id;
    std::string type;
    std::string shop;
    int cost;

    virtual ~Constraint() = default;

    virtual void from_json(const json& j) = 0;  // Pure virtual function to parse from JSON
    virtual json to_json() const = 0;           // To JSON representation of the constraint

    static std::shared_ptr<Constraint> create_constraint(const json& j);  // Declaration here
};

// BatchSizeConstraint class derived from Constraint
class BatchSizeConstraint : public Constraint {
public:
    int min_vehicles;
    int max_vehicles;
    std::vector<int> vehicles;

    void from_json(const json& j) override {
        j.at("id").get_to(id);
        j.at("type").get_to(type);
        j.at("shop").get_to(shop);
        j.at("cost").get_to(cost);
        j.at("min_vehicles").get_to(min_vehicles);
        j.at("max_vehicles").get_to(max_vehicles);
        j.at("vehicles").get_to(vehicles);
    }

    json to_json() const override {
        return json{
            {"id", id},
            {"type", type},
            {"shop", shop},
            {"cost", cost},
            {"min_vehicles", min_vehicles},
            {"max_vehicles", max_vehicles},
            {"vehicles", vehicles}
        };
    }
};

// LotChangeConstraint class derived from Constraint
class LotChangeConstraint : public Constraint {
public:
    std::vector<std::vector<int>> partition;

    void from_json(const json& j) override {
        j.at("id").get_to(id);
        j.at("type").get_to(type);
        j.at("shop").get_to(shop);
        j.at("cost").get_to(cost);
        j.at("partition").get_to(partition);
    }

    json to_json() const override {
        return json{
            {"id", id},
            {"type", type},
            {"shop", shop},
            {"cost", cost},
            {"partition", partition}
        };
    }
};

// RollingWindowConstraint class derived from Constraint
class RollingWindowConstraint : public Constraint {
public:
    int window_size;
    int max_vehicles;
    std::vector<int> vehicles;

    void from_json(const json& j) override {
        j.at("id").get_to(id);
        j.at("type").get_to(type);
        j.at("shop").get_to(shop);
        j.at("cost").get_to(cost);
        j.at("window_size").get_to(window_size);
        j.at("max_vehicles").get_to(max_vehicles);
        j.at("vehicles").get_to(vehicles);
    }

    json to_json() const override {
        return json{
            {"id", id},
            {"type", type},
            {"shop", shop},
            {"cost", cost},
            {"window_size", window_size},
            {"max_vehicles", max_vehicles},
            {"vehicles", vehicles}
        };
    }
};

// Instance class containing constraints
struct Instance {
    std::vector<Shop> shops;
    int two_tone_delta;
    int resequencing_cost;
    std::vector<Vehicle> vehicles;
    std::vector<std::shared_ptr<Constraint>> constraints;  // Vector of shared pointers to constraints

    Instance() = default;

    static Instance from_json(const json& j) {
        Instance instance;

        for (const auto& shop_json : j.at("shops")) {
            instance.shops.push_back(Shop::from_json(shop_json));
        }

        instance.two_tone_delta = j.at("parameters").at("two_tone_delta").get<int>();
        instance.resequencing_cost = j.at("parameters").at("resequencing_cost").get<int>();

        for (const auto& vehicle_json : j.at("vehicles")) {
            instance.vehicles.push_back(Vehicle::from_json(vehicle_json));
        }

        for (const auto& constraint_json : j.at("constraints")) {
            instance.constraints.push_back(Constraint::create_constraint(constraint_json));
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
