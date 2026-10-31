#pragma once

#include <vector>

// ESQUELETO. A dupla implementa esta classe.
// Usa união por rank e compressão de caminho.
class UnionFind {
public:
    // TODO: parent[i] = i e rnk[i] = 0 para cada um dos n elementos.
    explicit UnionFind(int n) { (void)n; }

    // Devolve a raiz do grupo de x e aponta o caminho inteiro para ela.
    // TODO: escrever com laço, não com recursão, para não estourar a pilha.
    int find(int x) { return x; }

    // Junta os grupos de a e b. Devolve false se já estavam juntos.
    // TODO: a raiz de rank menor fica embaixo da outra.
    // TODO: com ranks iguais, a raiz de b fica embaixo da raiz de a. O teste espera essa regra.
    bool unite(int a, int b) {
        (void)a;
        (void)b;
        return false;
    }

    // Quantidade de grupos.
    // TODO: contar as posições com parent[i] == i. Não contar valores diferentes.
    int contar_grupos() const { return 0; }

    // Pronto. Usados pelos testes para ler e definir o vetor parent.
    const std::vector<int>& pais() const { return parent; }
    void definir_pais(const std::vector<int>& novos) { parent = novos; }

private:
    std::vector<int> parent;
    std::vector<int> rnk;
};
