#include "solver.h"
#include <algorithm>
#include <random>
#include <iostream>

std::vector<int> paint_perm(std::vector<int>& input_perm, const Instance& instance)
{
    // input_perm is destroyed 

    int n = instance.n;
    int delta = instance.two_tone_delta;

    std::vector<int> result;
    std::vector<int> remaining_paints;
    for (int i = 0; i < n; i++)
    {
        remaining_paints.push_back(
            instance.vehicles[i].two_tone ? 2 : 1
        );
    }

    int front = 0;

    while (front < n)
    {
        int v = input_perm[front];
        if (--remaining_paints[v] > 0)
        {
            for (int i = front; i < front + delta && i < n - 1; i++)
                std::swap(input_perm[i], input_perm[i+1]);
        }
        else
            result.push_back(input_perm[front++]);
    }
    return result;

}


Solution solve(const Instance& instance) {
    Solution result;
    int n = instance.vehicles.size();

    std::vector<int> permutation;
    for (int i = 0; i < n; i++)
        permutation.push_back(i);

    auto rng = std::default_random_engine {};
    std::shuffle(permutation.begin(), permutation.end(), rng);

    for (const Shop& shop : instance.shops)
    {
        result.shop_results[shop.name].entry = permutation;
        if (shop.name == "paint")
            permutation = paint_perm(permutation, instance);
        result.shop_results[shop.name].exit = permutation;
    }

    std::cerr << result.compute_score(instance) << std::endl;

    return result;
}
