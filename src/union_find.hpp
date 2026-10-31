#pragma once

#include <utility>
#include <vector>

class UnionFind {
public:
    explicit UnionFind(int n) : parent(n), rnk(n, 0) {
        for (int i = 0; i < n; i++) parent[i] = i;
    }

    // com laco pq uma corrente longa estouraria a pilha na recursao
    int find(int x) {
        int raiz = x;
        while (parent[raiz] != raiz) raiz = parent[raiz];
        while (parent[x] != raiz) {
            int prox = parent[x];
            parent[x] = raiz;
            x = prox;
        }
        return raiz;
    }

    bool unite(int a, int b) {
        int ra = find(a);
        int rb = find(b);
        if (ra == rb) return false;
        if (rnk[ra] < rnk[rb]) std::swap(ra, rb);
        parent[rb] = ra;
        if (rnk[ra] == rnk[rb]) rnk[ra]++;
        return true;
    }

    int contar_grupos() const {
        int total = 0;
        for (int i = 0; i < (int)parent.size(); i++) {
            if (parent[i] == i) total++;
        }
        return total;
    }

    const std::vector<int>& pais() const { return parent; }
    void definir_pais(const std::vector<int>& novos) { parent = novos; }

private:
    std::vector<int> parent;
    std::vector<int> rnk;
};
