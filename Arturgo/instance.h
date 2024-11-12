#ifndef INSTANCE_H
#define INSTANCE_H

#include "../json/single_include/nlohmann/json.hpp"
using nlohmann::json;

#include <bits/stdc++.h>
using namespace std;

/* Prototypes */

struct Solution;
struct Instance;

struct Solution {
    Instance* inst;

    /* TODO : Attributs à adapter au problème */
        int nbMembres;
    /* TODOEND */

    json to_json();
    double score();
};

struct Instance {
    /* TODO : Attributs à adapter au problème */
        vector<string> membres;
    /* TODOEND */

    Instance(json data);
    Solution solve();
};



/* Logique des fonctions */

Instance::Instance(json data) {
    /* TODO : Initialisation des attributs à partir de data */
        for(string membre : data["membres"]) {
            membres.push_back(membre);
        }
    /* TODOEND */
}

Solution Instance::solve() {
    Solution sol; sol.inst = this;

    /* TODO : Crée une solution naïve au problème */
        sol.nbMembres = membres.size();
    /* TODOEND */

    return sol;
}

json Solution::to_json() {
    json data;

    /* TODO : Convertir la solution en JSON */
        data = {
            {"nb_membres", nbMembres}
        };
    /* TODOEND */

    return data;
}

double Solution::score() {
    double total = 0.;

    /* TODO : Calcule le score de la solution */
        total = nbMembres;
    /* TODOEND */

    return total;
}

#endif