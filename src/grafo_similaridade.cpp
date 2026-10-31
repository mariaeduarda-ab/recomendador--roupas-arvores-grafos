#include "grafo_similaridade.hpp"

#include <algorithm>
#include <cmath>
#include <unordered_map>

#include "union_find.hpp"

void GrafoSimilaridade::construir(const std::vector<Peca>& pecas, const KDTree& arvore, int k) {
    int n = (int)pecas.size();
    std::vector<std::vector<Vizinho>> viz(n);
    for (int i = 0; i < n; i++) viz[i] = arvore.knn(pecas[i].v, k, &pecas[i].categoria, i);

    adj_.assign(n, {});
    for (int i = 0; i < n; i++) {
        for (const Vizinho& v : viz[i]) {
            int j = v.indice;
            if (j < i) continue;  // o par ja foi visto quando era a vez de j
            bool mutuo = std::any_of(viz[j].begin(), viz[j].end(),
                                     [i](const Vizinho& w) { return w.indice == i; });
            if (mutuo) {
                double d = std::sqrt(v.dist2);
                adj_[i].push_back({j, d});
                adj_[j].push_back({i, d});
            }
        }
    }
}

std::vector<int> GrafoSimilaridade::grupos() const {
    int n = (int)adj_.size();
    UnionFind uf(n);
    for (int i = 0; i < n; i++) {
        for (const Aresta& a : adj_[i]) {
            if (i < a.destino) uf.unite(i, a.destino);
        }
    }
    std::vector<int> res(n);
    for (int i = 0; i < n; i++) res[i] = uf.find(i);
    return res;
}

ResumoGrupos resumir_grupos(const std::vector<int>& grupos) {
    std::unordered_map<int, int> tamanhos;
    for (int g : grupos) tamanhos[g]++;
    ResumoGrupos resumo;
    resumo.quantidade = (int)tamanhos.size();
    for (const auto& par : tamanhos) resumo.maior = std::max(resumo.maior, par.second);
    return resumo;
}
