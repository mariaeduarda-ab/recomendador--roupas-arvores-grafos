#include "grafo_looks.hpp"

#include <cmath>

void GrafoLooks::construir(const Base& base) {
    looks_da_peca.assign(base.pecas.size(), {});
    pecas_do_look.assign(base.looks.size(), {});
    for (int l = 0; l < (int)base.looks.size(); l++) {
        // look de teste aqui seria vazamento: a BFS acharia a peca escondida
        if (!base.looks[l].treino) continue;
        pecas_do_look[l] = base.looks[l].pecas;
        for (int p : pecas_do_look[l]) looks_da_peca[p].push_back(l);
    }
}

std::unordered_map<int, double> GrafoLooks::bfs_2_passos(int peca, bool ajuste_popularidade) const {
    std::unordered_map<int, double> cont;
    for (int l : looks_da_peca[peca]) {
        for (int p : pecas_do_look[l]) {
            if (p != peca) cont[p] += 1.0;
        }
    }
    if (ajuste_popularidade) {
        for (auto& par : cont) par.second /= std::sqrt(num_looks(par.first));
    }
    return cont;
}

int GrafoLooks::num_looks(int peca) const {
    return (int)looks_da_peca[peca].size();
}
