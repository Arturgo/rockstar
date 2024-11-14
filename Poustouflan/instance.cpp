#include "instance.h"

// Factory function for creating the correct constraint type based on JSON
std::shared_ptr<Constraint> Constraint::create_constraint(const json& j, const std::vector<Shop>& all_shops, int n) {
    std::string type = j.at("type").get<std::string>();

    if (type == "batch_size") {
        auto constraint = std::make_shared<BatchSizeConstraint>();
        constraint->from_json(j, all_shops, n);
        return constraint;
    }
    if (type == "lot_change") {
        auto constraint = std::make_shared<LotChangeConstraint>();
        constraint->from_json(j, all_shops, n);
        return constraint;
    }
    if (type == "rolling_window") {
        auto constraint = std::make_shared<RollingWindowConstraint>();
        constraint->from_json(j, all_shops, n);
        return constraint;
    }

    throw std::runtime_error("Unknown constraint type: " + type);
}
