#pragma once

#include <cstddef>
#include <string>
#include <vector>

struct Peca {
    int id = 0;
    std::string categoria;
    std::string nome;  // so existe no exemplo
    std::vector<double> v;  // 5 numeros na base real, 2 no exemplo
};

// sem raiz, a ordem das distancias nao muda
inline double distancia2(const std::vector<double>& a, const std::vector<double>& b) {
    double soma = 0.0;
    for (std::size_t i = 0; i < a.size() && i < b.size(); i++) {
        double dif = a[i] - b[i];
        soma += dif * dif;
    }
    return soma;
}
