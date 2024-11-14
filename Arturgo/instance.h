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
    string name; int Delta;
};

struct Partition {
    int id;
    int iStation, cost;
    vector<int> partition;
};

struct RollingWindow {
    int iStation, cost, window_size, maximum;
    vector<bool> is_in;
};

struct Batch {
    int iStation, cost, minimum, maximum;
    vector<bool> is_in;
};

struct Stat {
    double sequencing;
    double partition;
    double window;
    double batch;
    void display();
};

struct Solution {
    Instance* inst;

    vector<Ordre> ordres;
    
    json to_json();
    double score();
    Stat stat(string path, bool display_part);
};

struct Instance {
    int delta, rcost;
    vector<Station> stations;
    vector<bool> two_tones;
    vector<Partition> partitions;
    vector<RollingWindow> windows;
    vector<Batch> batches;

    int id_station(string name);

    int nb_vehicules() { return two_tones.size(); }
    vector<int> paint_reorder(const vector<int>& p);

    int resequencing_cost(const vector<int>& input, const vector<int>& output, int Delta);
    int partition_cost(const vector<int>& partition, const vector<int>& input);
    int window_cost(const vector<bool>& is_in, int size, int maximum, const vector<int>& input);
    int batch_cost(const vector<bool>& is_in, int minimum, int maximum, const vector<int>& input);

    void display_partition(string path, int id, const vector<int>& partition, const vector<int>& input);

    Instance(json data);
    Solution solve();
    Solution dumb_solution();
};

/* Logique des fonctions */

int Instance::id_station(string name) {
    for(int i = 0;i < (int)stations.size();i++) {
        if(name == stations[i].name) return i;
    }
    return -1;
}

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

int Instance::resequencing_cost(const vector<int>& input, const vector<int>& output, int Delta) {
    vector<int> pos_in(nb_vehicules()), pos_out(nb_vehicules());

    for(int i = 0;i < nb_vehicules();i++) {
        pos_in[input[i]] = i;
        pos_out[output[i]] = i;
    }

    int total = 0;
    for(int i = 0;i < nb_vehicules();i++) {
        total += max(0, pos_in[i] - Delta - pos_out[i]);
    }
    return total;
}

Instance::Instance(json data) {
    for(json shop : data["shops"]) {
        stations.push_back({(string)shop["name"], (int)shop["resequencing_lag"]});
    }

    two_tones.resize(data["vehicles"].size());
    for(json vehicle : data["vehicles"]) {
        two_tones[(int)vehicle["id"] - 1] = ((string)vehicle["type"] == "two-tone");
    }

    delta = (int)data["parameters"]["two_tone_delta"];
    rcost = (int)data["parameters"]["resequencing_cost"];

    for(json constraint : data["constraints"]) {
        string type = constraint["type"];
        if(type == "lot_change") {
            Partition cstr;
            cstr.iStation = id_station(constraint["shop"]);
            cstr.cost = constraint["cost"];
            cstr.id = constraint["id"];

            vector<int> partition(nb_vehicules(), 0);
            int iPart = 0;
            for(json part : constraint["partition"]) {
                for(int vid : part) {
                    partition[vid - 1] = iPart;
                }
                iPart++;
            }

            cstr.partition = partition;
            partitions.push_back(cstr);
        } else if(type == "rolling_window") {
            RollingWindow cstr;
            cstr.iStation = id_station(constraint["shop"]);
            cstr.cost = constraint["cost"];
            cstr.window_size = constraint["window_size"];
            cstr.maximum = constraint["max_vehicles"];

            vector<bool> is_in(nb_vehicules(), false);
            for(int vid : constraint["vehicles"]) {
                is_in[vid - 1] = true;
            }
            cstr.is_in = is_in;

            windows.push_back(cstr);
        } else if(type == "batch_size") {
            Batch cstr;
            cstr.iStation = id_station(constraint["shop"]);
            cstr.cost = constraint["cost"];
            cstr.minimum = constraint["min_vehicles"];
            cstr.maximum = constraint["max_vehicles"];

            vector<bool> is_in(nb_vehicules(), false);
            for(int vid : constraint["vehicles"]) {
                is_in[vid - 1] = true;
            }
            cstr.is_in = is_in;

            batches.push_back(cstr);
        }
    }
}

Solution Instance::dumb_solution() {
    Solution sol; sol.inst = this;
    vector<int> id(nb_vehicules());
    iota(id.begin(), id.end(), 0);
    vector<int> pid = paint_reorder(id);
    sol.ordres.push_back({id, id});
    sol.ordres.push_back({id, pid});
    sol.ordres.push_back({pid, pid});
    return sol;
}

Solution Instance::solve() {
    mutex mtx;
    Solution best = dumb_solution();
    double best_score = best.score();

    double T = 5e4;
    int bonus = 0;

    auto lambda = [&](int id) {
        while(T > 1e-8) {
            mtx.lock();
            vector<vector<int>> inputs = {
                best.ordres[0].entry,
                best.ordres[1].entry,
                best.ordres[2].entry
            };
            mtx.unlock();

            if(rand() % 3 == 0) {
                int station = rand() % 3;
                int id1 = rand() % nb_vehicules();
                int id2 = rand() % nb_vehicules();
                swap(inputs[station][id1], inputs[station][id2]);
            } else if(rand() % 2 == 0) {
                int id1 = rand() % nb_vehicules();
                int id2 = rand() % nb_vehicules();

                int pos1, pos2;
                for(int iStation = 0;iStation < 3;iStation++) {
                    for(int iVehicle = 0;iVehicle < nb_vehicules();iVehicle++) {
                        if(inputs[iStation][iVehicle] == id1) {
                            pos1 = iVehicle;
                        }
                        if(inputs[iStation][iVehicle] == id2) {
                            pos2 = iVehicle;
                        }
                    }
                    swap(inputs[iStation][pos1], inputs[iStation][pos2]);
                }
            } else {
                int id1 = rand() % nb_vehicules();
                int id2 = rand() % nb_vehicules();
                int id3 = rand() % nb_vehicules();

                int pos1, pos2, pos3;
                for(int iStation = 0;iStation < 3;iStation++) {
                    for(int iVehicle = 0;iVehicle < nb_vehicules();iVehicle++) {
                        if(inputs[iStation][iVehicle] == id1) {
                            pos1 = iVehicle;
                        }
                        if(inputs[iStation][iVehicle] == id2) {
                            pos2 = iVehicle;
                        }
                        if(inputs[iStation][iVehicle] == id3) {
                            pos3 = iVehicle;
                        }
                    }
                    swap(inputs[iStation][pos1], inputs[iStation][pos2]);
                    swap(inputs[iStation][pos2], inputs[iStation][pos3]);
                }
            }

            Solution sol; sol.inst = this;
            sol.ordres.push_back({inputs[0], inputs[0]});
            sol.ordres.push_back({inputs[1], paint_reorder(inputs[1])});
            sol.ordres.push_back({inputs[2], inputs[2]});

            double score = sol.score();
            double D = best_score - score;

            if(D > 0 || rand() / (double)RAND_MAX <= exp(D / T)) {
                mtx.lock();
                best_score = score;
                best = sol;
                mtx.unlock();
            } 
            if(rand() % 100 == 0) T *= 0.99995;
            if(id == 0 && rand() % 10000 == 0) cerr << T << " " << best_score << endl;
        }
    };

    vector<thread> threads;

    for(int i = 0;i < 16;i++) {
        threads.push_back(thread(lambda, i));
    }

    for(int i = 0;i < (int)threads.size();i++) {
        threads[i].join();
    }

    return best;
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

int Instance::partition_cost(const vector<int>& partition, const vector<int>& input) {
    int total = 0;
    for(int i = 0;i < nb_vehicules() - 1;i++) {
        if(partition[input[i]] != partition[input[i + 1]]) total++;
    }
    return total;
}

void Instance::display_partition(string path, int id, const vector<int>& partition, const vector<int>& input) {
    string p = path + "-partition" + to_string(id) + ".txt";
    ofstream fout(p);
    int current_p = partition[input[0]];
    int nb = 1;
    for (int i = 1; i < nb_vehicules(); i++) {
        if (current_p == partition[input[i]]) {
            nb += 1;
        } else {
            fout << nb << endl;
            current_p = partition[input[i]];
            nb = 1;
        }
    }
    fout << nb << endl;
}

int Instance::window_cost(const vector<bool>& is_in, int size, int maximum, const vector<int>& input) {
    int total = 0;
    int nb_in = 0;

    int fin = 0;
    for(;fin < size;fin++) {
        nb_in += is_in[input[fin]];
    }

    while(true) {
        int m = max(0, nb_in - maximum);
        total += m * m;

        if(fin >= nb_vehicules()) break;
        nb_in += is_in[input[fin]];
        nb_in -= is_in[input[fin - size]];
        fin++;
    }

    return total;
}

int Instance::batch_cost(const vector<bool>& is_in, int minimum, int maximum, const vector<int>& input) {
    int total = 0;
    int cons = 0;

    for(int vid : input) {
        if(is_in[vid]) {
            cons += 1;
        } else {
            if(cons > 0) {
                int m = 0;
                m = max(m, minimum - cons);
                m = max(m, cons - maximum);
                total += m * m;
            }
            cons = 0;
        }
    }

    if(cons > 0) {
        int m = 0;
        m = max(m, minimum - cons);
        m = max(m, cons - maximum);
        total += m * m;
    }

    return total;
}

double Solution::score() {
    double total = 0.;

    for(int iStation = 0;iStation < (int)inst->stations.size() - 1;iStation++) {
        total += inst->rcost * inst->resequencing_cost(ordres[iStation].exit, ordres[iStation + 1].entry, inst->stations[iStation].Delta);
    }

    for(const Partition& part : inst->partitions) {
        total += part.cost * inst->partition_cost(part.partition, ordres[part.iStation].entry);
    }

    for(const RollingWindow& window : inst->windows) {
        total += window.cost * inst->window_cost(window.is_in, window.window_size, window.maximum, ordres[window.iStation].entry);
    }

    for(const Batch& batch : inst->batches) {
        total += batch.cost * inst->batch_cost(batch.is_in, batch.minimum, batch.maximum, ordres[batch.iStation].entry);
    }

    return total;
}

Stat Solution::stat(string path, bool info) {
    Stat s;

    for(int iStation = 0;iStation < (int)inst->stations.size() - 1;iStation++) {
        s.sequencing += inst->rcost * inst->resequencing_cost(ordres[iStation].exit, ordres[iStation + 1].entry, inst->stations[iStation].Delta);
    }

    for(const Partition& part : inst->partitions) {
        s.partition += part.cost * inst->partition_cost(part.partition, ordres[part.iStation].entry);
        if (info) {
            inst->display_partition(path, part.id, part.partition, ordres[part.iStation].entry);
        }
        cerr << "p" << part.cost << " " << part.cost * inst->partition_cost(part.partition, ordres[part.iStation].entry) << endl;
    }

    for(const RollingWindow& window : inst->windows) {
        s.window += window.cost * inst->window_cost(window.is_in, window.window_size, window.maximum, ordres[window.iStation].entry);
        cerr << "w" << window.cost << " " << window.cost * inst->window_cost(window.is_in, window.window_size, window.maximum, ordres[window.iStation].entry) << endl;
    }

    for(const Batch& batch : inst->batches) {
        s.batch += batch.cost * inst->batch_cost(batch.is_in, batch.minimum, batch.maximum, ordres[batch.iStation].entry);
    }

    return s;
}

void Stat::display() {
    cerr << "sequencing : " << sequencing << endl;
    cerr << "partition  : " << partition << endl;
    cerr << "window     : " << window << endl;
    cerr << "batch      : " << batch << endl;
}

#endif
