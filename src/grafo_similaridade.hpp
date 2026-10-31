#pragma once

#include <vector>

#include "kdtree.hpp"
#include "peca.hpp"

struct Aresta {
    int destino = -1;
    double peso = 0.0;  // distancia entre as pecas
};

struct ResumoGrupos {
    int quantidade = 0;
    int maior = 0;
};

class GrafoSimilaridade {
public:
    // knn mutuo: A-B so existe se um esta entre as k do outro, nos dois sentidos
    void construir(const std::vector<Peca>& pecas, const KDTree& arvore, int k);

    // grupos[i] e a raiz do grupo da peca i
    std::vector<int> grupos() const;

    const std::vector<std::vector<Aresta>>& adjacencia() const { return adj_; }

private:
    std::vector<std::vector<Aresta>> adj_;
};

ResumoGrupos resumir_grupos(const std::vector<int>& grupos);
