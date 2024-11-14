#include "solver.h"

Result solve(const Instance& instance) {
    Result result;
    int n = instance.vehicles.size();

    for (const Shop& shop : instance.shops)
    {
        for (int i = 1; i <= n; i++)
        {
            result.shop_results[shop.name].entry.push_back(i);
            result.shop_results[shop.name].exit.push_back(i);
        }
    }

    return result;
}
