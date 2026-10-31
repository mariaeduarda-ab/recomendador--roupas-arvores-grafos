#include "grafo_similaridade.hpp"

#include <algorithm>
#include <unordered_map>

#include "union_find.hpp"

void GrafoSimilaridade::construir(const std::vector<Peca>& pecas, const KDTree& arvore, int k) {
    // TODO: guardar as k vizinhas de cada peça com arvore.knn.
    // TODO: criar a aresta só quando a escolha for dos dois lados.
    // TODO: o peso é a raiz quadrada de dist2. Criar a aresta nas duas listas.
    (void)pecas;
    (void)arvore;
    (void)k;
}

std::vector<int> GrafoSimilaridade::grupos() const {
    // TODO: criar UnionFind(adj_.size()) e chamar unite uma vez por aresta.
    // TODO: devolver find(i) para cada peça.
    return {};
}

ResumoGrupos resumir_grupos(const std::vector<int>& grupos) {
    std::unordered_map<int, int> tamanhos;
    for (const int grupo : grupos) tamanhos[grupo]++;
    ResumoGrupos resumo;
    resumo.quantidade = static_cast<int>(tamanhos.size());
    for (const auto& par : tamanhos) resumo.maior = std::max(resumo.maior, par.second);
    return resumo;
}
