#include <iostream>
#include <nlohmann/json.hpp>
#include "instance.h"
#include "solution.h"
#include "solver.h"

using json = nlohmann::json;

int main() {
    // Read JSON input from stdin
    json input_json;
    std::cin >> input_json;

    // Parse JSON input to create an Instance object
    Instance instance = Instance::from_json(input_json);

    // Solve the problem
    Solution result = solve(instance);

    // Output the result JSON to stdout
    std::cout << result.to_json() << std::endl;

    return 0;
}
