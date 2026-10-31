#pragma once

#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "peca.hpp"

struct Vizinho {
    int indice = -1;  // posicao no vetor de pecas
    double dist2 = 0.0;
};

// (dist2, indice). o topo e a pior das k melhores
using HeapMax = std::priority_queue<std::pair<double, int>>;

class KDTree {
public:
    explicit KDTree(int dim);

    // guarda so um ponteiro, entao pecas precisa continuar vivo
    void construir(const std::vector<Peca>& pecas);

    std::vector<int> preordem() const;
    Vizinho mais_proximo(const std::vector<double>& q, int& visitados) const;

    // categoria != nullptr filtra na hora de entrar no heap.
    // excluir e pra peca nao achar ela mesma
    std::vector<Vizinho> knn(const std::vector<double>& q, int k,
                             const std::string* categoria = nullptr, int excluir = -1) const;

    // gabarito da knn nos testes
    std::vector<Vizinho> forca_bruta(const std::vector<double>& q, int k,
                                     const std::string* categoria = nullptr, int excluir = -1) const;

private:
    struct No {
        int indice = -1;
        int eixo = 0;
        int esquerda = -1;
        int direita = -1;
    };

    struct Busca {
        const std::vector<double>& q;
        int k;
        const std::string* categoria;
        int excluir;
        HeapMax heap;
        int visitados;
    };

    int montar(std::vector<int>& ids, int ini, int fim, int prof);
    void pre(int no, std::vector<int>& res) const;
    void buscar(int no, Busca& b) const;
    bool passa_filtro(int i, const std::string* categoria, int excluir) const;

    int dim_;
    const std::vector<Peca>* pecas_ = nullptr;
    std::vector<No> nos_;
    int raiz_ = -1;
};
