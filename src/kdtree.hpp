#pragma once

#include <string>
#include <vector>

#include "peca.hpp"

// Resultado de uma busca. indice é a posição da peça no vetor de peças.
struct Vizinho {
    int indice = -1;
    double dist2 = 0.0;  // distância ao quadrado
};

// ESQUELETO. A dupla implementa esta classe.
// A dimensão é definida em tempo de execução. Os testes usam 2 e o projeto real usa 5.
class KDTree {
public:
    explicit KDTree(int dim);

    // Monta a árvore. O eixo de cada nível é profundidade % dim.
    // O nó é a peça na posição n/2 depois de std::nth_element por esse eixo.
    // Guarda um ponteiro para pecas, então o vetor precisa continuar vivo.
    void construir(const std::vector<Peca>& pecas);

    // Índices das peças em pré-ordem (nó, esquerda, direita).
    std::vector<int> preordem() const;

    // Peça mais próxima de q. visitados recebe quantos nós a busca comparou.
    Vizinho mais_proximo(const std::vector<double>& q, int& visitados) const;

    // As k peças mais próximas de q, da mais próxima para a mais distante.
    // Usa heap de máximo (std::priority_queue) e distâncias ao quadrado.
    // categoria != nullptr: só peças dessa categoria entram no heap.
    // excluir >= 0: essa peça nunca entra no heap. Serve para a peça não achar a si mesma.
    std::vector<Vizinho> knn(const std::vector<double>& q, int k,
                             const std::string* categoria = nullptr, int excluir = -1) const;

    // Mesmo contrato de knn, mas compara com todas as peças. É o gabarito dos testes.
    std::vector<Vizinho> forca_bruta(const std::vector<double>& q, int k,
                                     const std::string* categoria = nullptr, int excluir = -1) const;

private:
    struct No {
        int indice = -1;
        int eixo = 0;
        int esquerda = -1;
        int direita = -1;
    };

    int dim_;
    const std::vector<Peca>* pecas_ = nullptr;
    std::vector<No> nos_;
    int raiz_ = -1;
};
