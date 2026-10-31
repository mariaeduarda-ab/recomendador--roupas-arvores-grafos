#include "kdtree.hpp"

KDTree::KDTree(int dim) : dim_(dim) {}

void KDTree::construir(const std::vector<Peca>& pecas) {
    // TODO: guardar o ponteiro para pecas e montar a árvore recursivamente.
    // TODO: em cada nível, usar std::nth_element em um vetor de índices.
    (void)pecas;
}

std::vector<int> KDTree::preordem() const {
    // TODO: percorrer nó, esquerda, direita a partir de raiz_.
    return {};
}

Vizinho KDTree::mais_proximo(const std::vector<double>& q, int& visitados) const {
    // TODO: descer primeiro pelo lado de q.
    // TODO: só visitar o outro lado se (q[eixo] - no[eixo])^2 < melhor distância ao quadrado.
    (void)q;
    visitados = 0;
    return Vizinho{};
}

std::vector<Vizinho> KDTree::knn(const std::vector<double>& q, int k, const std::string* categoria,
                                 int excluir) const {
    // TODO: mesma busca de mais_proximo, com um heap de máximo de tamanho k.
    // TODO: enquanto o heap tiver vaga, o outro lado sempre precisa ser visitado.
    // TODO: com o heap cheio, a poda usa o topo do heap.
    // TODO: devolver da mais próxima para a mais distante.
    (void)q;
    (void)k;
    (void)categoria;
    (void)excluir;
    return {};
}

std::vector<Vizinho> KDTree::forca_bruta(const std::vector<double>& q, int k,
                                         const std::string* categoria, int excluir) const {
    // TODO: comparar com todas as peças e guardar as k melhores.
    (void)q;
    (void)k;
    (void)categoria;
    (void)excluir;
    return {};
}
