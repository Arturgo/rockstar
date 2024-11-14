#include "instance.h"
#include "solution.h"


int Instance::resequencing_cost(const std::vector<int>& input, const std::vector<int>& output, int Delta) const {
    std::vector<int> pos_in(n), pos_out(n);

    for(int i = 0;i < n;i++) {
        pos_in[input[i]] = i;
        pos_out[output[i]] = i;
    }

    int total = 0;
    for(int i = 0;i < n;i++) {
        total += std::max(0, pos_in[i] - Delta - pos_out[i]);
    }
    return total;
}

int Instance::partition_cost(const std::vector<int>& partition, const std::vector<int>& input) const {
    int total = 0;
    for(int i = 0;i < n - 1;i++) {
        if(partition[input[i]] != partition[input[i + 1]]) total++;
    }
    return total;
}

int Instance::window_cost(const std::vector<bool>& is_in, int size, int maximum, const std::vector<int>& input) const {
    int total = 0;
    int nb_in = 0;

    int fin = 0;
    for(;fin < size;fin++) {
        nb_in += is_in[input[fin]];
    }

    while(true) {
        int m = std::max(0, nb_in - maximum);
        total += m * m;

        if(fin >= n) break;
        nb_in += is_in[input[fin]];
        nb_in -= is_in[input[fin - size]];
        fin++;
    }

    return total;
}

int Instance::batch_cost(const std::vector<bool>& is_in, int minimum, int maximum, const std::vector<int>& input) const {
    int total = 0;
    int cons = 0;

    for(int vid : input) {
        if(is_in[vid]) {
            cons += 1;
        } else {
            if(cons > 0) {
                int m = 0;
                m = std::max(m, minimum - cons);
                m = std::max(m, cons - maximum);
                total += m * m;
            }
            cons = 0;
        }
    }

    if(cons > 0) {
        int m = 0;
        m = std::max(m, minimum - cons);
        m = std::max(m, cons - maximum);
        total += m * m;
    }

    return total;
}




double Solution::score(const Instance& instance) const {
    double total = 0.;

    for (int i = 0; i < (int)instance.shops.size() - 1; i++)
    {
        const Shop& previous_shop = instance.shops[i];
        const Shop& next_shop = instance.shops[i + 1];

        total += instance.rc * instance.resequencing_cost(
            orders.at(previous_shop.name).exit,
            orders.at(next_shop.name).entry,
            previous_shop.resequencing_lag
        );
    }

    for (const LotChangeConstraint& part : instance.lot_change_constraints) {
        total += part.cost * instance.partition_cost(part.partition, orders.at(part.shop->name).entry);
    }

    for (const RollingWindowConstraint& window : instance.rolling_window_constraints) {
        total += window.cost * instance.window_cost(window.vehicles, window.window_size, window.max_vehicles, orders.at(window.shop->name).entry);
    }

    for (const BatchSizeConstraint& batch : instance.batch_size_constraints) {
        total += batch.cost * instance.batch_cost(batch.vehicles, batch.min_vehicles, batch.max_vehicles, orders.at(batch.shop->name).entry);
    }

    return total;
}
