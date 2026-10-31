#pragma once

#include <vector>

#include "kdtree.hpp"
#include "peca.hpp"

struct Aresta {
    int destino = -1;
    double peso = 0.0;  // distância entre as duas peças
};

// Pronto. Números que o programa imprime para cada k testado.
struct ResumoGrupos {
    int quantidade = 0;
    int maior = 0;
};

// ESQUELETO. A dupla implementa esta classe.
class GrafoSimilaridade {
public:
    // Monta o grafo kNN mútuo em lista de adjacência.
    // A aresta A-B só existe se B está entre as k de A e A está entre as k de B.
    // Cada busca usa a categoria da própria peça e exclui a própria peça.
    void construir(const std::vector<Peca>& pecas, const KDTree& arvore, int k);

    // Roda o Union-Find com um unite por aresta. grupos[i] é a raiz do grupo da peça i.
    std::vector<int> grupos() const;

    const std::vector<std::vector<Aresta>>& adjacencia() const { return adj_; }

private:
    std::vector<std::vector<Aresta>> adj_;
};

// Pronto. Conta os grupos e o tamanho do maior a partir do vetor grupos.
ResumoGrupos resumir_grupos(const std::vector<int>& grupos);
