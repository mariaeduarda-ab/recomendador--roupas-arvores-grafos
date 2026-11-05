#include "tarjan.hpp"

#include <algorithm>
#include <functional>

Tarjan tarjan(int n, const std::vector<std::pair<int, int>>& arestas) {
    Tarjan t;
    if (n < 0) n = 0;
    const int m = (int)arestas.size();
    t.desc.assign(n, -1);
    t.low.assign(n, 0);
    t.pai.assign(n, -1);
    t.tam.assign(n, 1);
    t.comp.assign(n, -1);
    t.articulacao.assign(n, false);
    t.pedacos.assign(n, {});
    t.ponte.assign(m, false);
    t.separa.assign(m, 0);

    // lista de adjacencia compacta: viz[inicio[v] .. inicio[v + 1]) com o numero da aresta
    std::vector<int> inicio(n + 1, 0);
    auto valida = [&](const std::pair<int, int>& a) {
        return a.first != a.second && a.first >= 0 && a.second >= 0 && a.first < n && a.second < n;
    };
    for (const auto& a : arestas) {
        if (!valida(a)) continue;
        inicio[a.first + 1]++;
        inicio[a.second + 1]++;
    }
    for (int v = 0; v < n; v++) inicio[v + 1] += inicio[v];
    std::vector<int> viz(inicio[n]);
    std::vector<int> num(inicio[n]);
    std::vector<int> livre(inicio.begin(), inicio.end() - 1);
    for (int e = 0; e < m; e++) {
        if (!valida(arestas[e])) continue;
        int u = arestas[e].first;
        int v = arestas[e].second;
        viz[livre[u]] = v;
        num[livre[u]++] = e;
        viz[livre[v]] = u;
        num[livre[v]++] = e;
    }

    std::vector<int> aresta_pai(n, -1);
    std::vector<int> prox(n, 0);
    std::vector<int> filho_da_ponte;  // ponte e o vertice de baixo dela
    std::vector<int> pilha;
    int tempo = 0;
    for (int r = 0; r < n; r++) {
        if (t.desc[r] != -1) continue;
        const int c = (int)t.tam_comp.size();
        t.desc[r] = t.low[r] = tempo++;
        t.comp[r] = c;
        prox[r] = inicio[r];
        pilha.push_back(r);
        while (!pilha.empty()) {
            int v = pilha.back();
            if (prox[v] < inicio[v + 1]) {
                int k = prox[v]++;
                int w = viz[k];
                // pula so a aresta que trouxe ate v. uma paralela a ela conta como aresta de volta
                if (num[k] == aresta_pai[v]) continue;
                if (t.desc[w] == -1) {
                    t.desc[w] = t.low[w] = tempo++;
                    t.comp[w] = c;
                    t.pai[w] = v;
                    aresta_pai[w] = num[k];
                    prox[w] = inicio[w];
                    pilha.push_back(w);
                } else {
                    t.low[v] = std::min(t.low[v], t.desc[w]);
                }
                continue;
            }
            // v terminou: passa o low e o tamanho para o pai
            pilha.pop_back();
            int p = t.pai[v];
            if (p < 0) continue;
            t.tam[p] += t.tam[v];
            t.low[p] = std::min(t.low[p], t.low[v]);
            // nenhuma aresta da subarvore de v sobe acima de p: sem p, ela se solta
            if (t.low[v] >= t.desc[p]) t.pedacos[p].push_back(t.tam[v]);
            if (t.low[v] > t.desc[p]) {
                t.ponte[aresta_pai[v]] = true;
                filho_da_ponte.push_back(v);
            }
        }
        t.tam_comp.push_back(t.tam[r]);
    }

    for (int v = 0; v < n; v++) {
        std::vector<int>& p = t.pedacos[v];
        if (t.pai[v] < 0) {
            // a raiz so e articulacao com 2 ou mais filhos na arvore
            if (p.size() < 2) p.clear();
        } else if (!p.empty()) {
            int soltos = 0;
            for (int x : p) soltos += x;
            p.push_back(t.tam_comp[t.comp[v]] - 1 - soltos);  // o pedaco de cima, com a raiz
        }
        t.articulacao[v] = !p.empty();
        std::sort(p.begin(), p.end(), std::greater<int>());
    }
    for (int v : filho_da_ponte) {
        t.separa[aresta_pai[v]] = std::min(t.tam[v], t.tam_comp[t.comp[v]] - t.tam[v]);
    }
    return t;
}
