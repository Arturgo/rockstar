#include "solver.h"

Result solve(const Instance& instance) {
    Result result;

    result.nom = instance.nom;
    std::vector<int> taille_membres;
    for (const std::string& nom : instance.membres)
        taille_membres.push_back(nom.size());
    result.taille_membres = taille_membres;
    result.nombre_membres = instance.membres.size();

    return result;
}
