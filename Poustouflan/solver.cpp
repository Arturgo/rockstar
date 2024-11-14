#include "solver.h"
#include <algorithm>
#include <random>
#include <iostream>
#include <list>

vector<int> paint_reorder(const vector<int>& p, const Instance& instance) {
    list<int> elems(p.begin(), p.end());
    vector<bool> seen(instance.n, false);

    vector<int> pp;

    auto itInsert = elems.begin(); 
    for(int _ = 0;_ <= instance.two_tone_delta;_++) {
        if(itInsert != elems.end())
            itInsert++;
    }

    while(!elems.empty()) {
        int val = *elems.begin();
        elems.erase(elems.begin());

        if(seen[val] || instance.vehicles[val].two_tone) {
            pp.push_back(val);

            if(itInsert != elems.end())
                itInsert++;
        } else {
            seen[val] = true;
            elems.insert(itInsert , val);
        }
    }

    return pp;
}


Solution dumb_solution(const Instance& inst) {
    Solution sol;
    vector<int> id(inst.n);
    iota(id.begin(), id.end(), 0);
    vector<int> pid = paint_reorder(id, inst);

    sol.shop_results["body"].entry = id;
    sol.shop_results["body"].exit = id;
    sol.shop_results["paint"].entry = id;
    sol.shop_results["paint"].exit = pid;
    sol.shop_results["assembly"].entry = pid;
    sol.shop_results["assembly"].exit = pid;
    return sol;
}

Solution solve(const Instance& inst) {
    Solution best = dumb_solution(inst);
    double best_score = best.compute_score(inst);

    vector<vector<int>> inputs = {
        best.shop_results["body"].entry,
        best.shop_results["paint"].entry,
        best.shop_results["assembly"].entry
    };

    double T = 1e3;
    int i = 0;
    while(T >= 1e-6) {
        vector<vector<int>> old_inputs = inputs;

        if(rand() % 2 == 0) {
            int station = rand() % 3;
            int id1 = rand() % inst.n;
            int id2 = rand() % inst.n;
            swap(inputs[station][id1], inputs[station][id2]);
        } else {
            int id1 = rand() % inst.n;
            int id2 = rand() % inst.n;

            int pos1, pos2;
            for(int iStation = 0;iStation < 3;iStation++) {
                for(int iVehicle = 0;iVehicle < inst.n;iVehicle++) {
                    if(inputs[iStation][iVehicle] == id1) {
                        pos1 = iVehicle;
                    }
                    if(inputs[iStation][iVehicle] == id2) {
                        pos2 = iVehicle;
                    }
                }
                swap(inputs[iStation][pos1], inputs[iStation][pos2]);
            }
        }

        Solution sol;
        sol.shop_results["body"].entry = inputs[0];
        sol.shop_results["body"].exit = inputs[0];
        sol.shop_results["paint"].entry = inputs[1];
        sol.shop_results["paint"].exit = paint_reorder(inputs[1], inst);
        sol.shop_results["assembly"].entry = inputs[2];
        sol.shop_results["assembly"].exit = inputs[2];

        double score = sol.compute_score(inst);
        double D = best_score - score;

        if(D > 0 || rand() / (double)RAND_MAX <= exp(D / T)) {
            best_score = score;
            best = sol;
        } else {
            inputs = old_inputs;
        }
        i++;
        if(i % 100 == 0) T *= 0.999;

        if(i % 100 == 0) cerr << i << " " << T << " " << best_score << endl;
    }

    return best;
}
