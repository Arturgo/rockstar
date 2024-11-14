#ifndef INSTANCE_H
#define INSTANCE_H

#include "../json/single_include/nlohmann/json.hpp"
using nlohmann::json;

#include <bits/stdc++.h>
using namespace std;

/* Prototypes */

struct Solution;
struct Instance;

struct Ordre {
    vector<int> entry, exit;
};

struct Station {
    string name;
};

struct Solution {
    Instance* inst;

    vector<Ordre> ordres;
    
    json to_json();
    double score();
};

struct Instance {
    int delta;
    vector<Station> stations;
    vector<bool> two_tones;

    int nb_vehicules() { return two_tones.size(); }
    vector<int> paint_reorder(const vector<int>& p);

    Instance(json data);
    Solution solve();
};

/* Logique des fonctions */

vector<int> Instance::paint_reorder(const vector<int>& p) {
    list<int> elems(p.begin(), p.end());
    vector<bool> seen(nb_vehicules(), false);

    vector<int> pp;

    auto itInsert = elems.begin(); 
    for(int _ = 0;_ <= delta;_++) {
        if(itInsert != elems.end())
            itInsert++;
    }

    while(!elems.empty()) {
        int val = *elems.begin();
        elems.erase(elems.begin());

        if(seen[val] || !two_tones[val]) {
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

Instance::Instance(json data) {
    for(json shop : data["shops"]) {
        stations.push_back({(string)shop["name"]});
    }

    two_tones.resize(data["vehicles"].size());
    for(json vehicle : data["vehicles"]) {
        two_tones[(int)vehicle["id"] - 1] = ((string)vehicle["type"] == "two-tone");
    }

    delta = (int)data["parameters"]["two_tone_delta"];
}

Solution Instance::solve() {
    Solution sol; sol.inst = this;

    vector<int> id(nb_vehicules());
    iota(id.begin(), id.end(), 0);

    vector<int> pid = paint_reorder(id);

    sol.ordres.push_back({id, id});
    sol.ordres.push_back({id, pid});
    sol.ordres.push_back({pid, pid});

    return sol;
}

json Solution::to_json() {
    json data;

    for(int iStation = 0;iStation < (int)inst->stations.size();iStation++) {
        json entry_json = json::array(), exit_json = json::array();

        for(int id : ordres[iStation].entry) {
            entry_json.push_back(1 + id);
        }

        for(int id : ordres[iStation].exit) {
            exit_json.push_back(1 + id);
        }

        data[inst->stations[iStation].name]["entry"] = entry_json;
        data[inst->stations[iStation].name]["exit"] = exit_json;
    }

    return data;
}

double Solution::score() {
    double total = 0.;

    /* TODO : Calcule le score de la solution */
    /* TODOEND */

    return total;
}

#endif