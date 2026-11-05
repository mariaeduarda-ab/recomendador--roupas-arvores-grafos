#pragma once

#include <utility>
#include <vector>

// Uso:
//   Tarjan t = tarjan(n, arestas);  // arestas[e] = (u, v), vertices de 0 a n - 1. O(V + E)
//   t.articulacao[v], t.pedacos[v]  // tamanhos dos pedacos que ficam se v sair (vazio se nao e articulacao)
//   t.ponte[e], t.separa[e]         // vertices do lado menor se a aresta e sair (0 se nao e ponte)
// Arestas paralelas contam como arestas diferentes, entao nenhuma delas e ponte. Laco (u == v) e ignorado.

struct Tarjan {
    std::vector<int> desc;       // tempo de descoberta, de 0 a n - 1
    std::vector<int> low;
    std::vector<int> pai;        // pai na arvore da DFS, -1 na raiz
    std::vector<int> tam;        // vertices na subarvore da DFS
    std::vector<int> comp;       // componente de cada vertice
    std::vector<int> tam_comp;   // vertices em cada componente
    std::vector<bool> articulacao;
    std::vector<std::vector<int>> pedacos;
    std::vector<bool> ponte;
    std::vector<int> separa;
};

Tarjan tarjan(int n, const std::vector<std::pair<int, int>>& arestas);
