#pragma once

#include <cstddef>
#include <string>
#include <vector>

// Uma peça de roupa. O vetor v guarda os atributos numéricos já normalizados e com pesos.
// No projeto real v tem 5 números. Nos testes com o exemplo do guia v tem 2.
struct Peca {
    int id = 0;             // id que veio do CSV
    std::string categoria;  // filtro, não entra no vetor
    std::string nome;       // opcional, usado só nos exemplos e no Gephi
    std::vector<double> v;
};

// Distância euclidiana ao quadrado. Evita a raiz quadrada e mantém a ordem das distâncias.
inline double distancia2(const std::vector<double>& a, const std::vector<double>& b) {
    double soma = 0.0;
    for (std::size_t i = 0; i < a.size() && i < b.size(); i++) {
        const double diferenca = a[i] - b[i];
        soma += diferenca * diferenca;
    }
    return soma;
}
