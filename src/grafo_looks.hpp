#pragma once

#include <unordered_map>
#include <vector>

#include "leitura.hpp"

// ESQUELETO. A dupla implementa esta classe.
// Grafo bipartido. Peças de um lado e looks do outro.
class GrafoLooks {
public:
    // Monta as duas listas. Só looks com treino == true entram.
    void construir(const Base& base);

    // BFS de 2 passos: peça, looks da peça, outras peças desses looks.
    // Devolve quantas vezes cada peça foi alcançada. A peça inicial não entra.
    // ajuste_popularidade: divide a contagem pela raiz do número de looks da candidata.
    std::unordered_map<int, double> bfs_2_passos(int peca, bool ajuste_popularidade = false) const;

    // Em quantos looks de treino a peça aparece.
    int num_looks(int peca) const;

private:
    std::vector<std::vector<int>> looks_da_peca;  // looks em que a peça p aparece
    std::vector<std::vector<int>> pecas_do_look;  // peças do look l
};
