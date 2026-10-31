#pragma once

#include <unordered_map>
#include <vector>

#include "leitura.hpp"

// bipartido: pecas de um lado, looks do outro. so looks de treino entram
class GrafoLooks {
public:
    void construir(const Base& base);

    // peca -> looks dela -> outras pecas desses looks. devolve a contagem de cada uma
    // ajuste_popularidade divide por raiz(looks da candidata), senao peca popular domina tudo
    std::unordered_map<int, double> bfs_2_passos(int peca, bool ajuste_popularidade = false) const;

    int num_looks(int peca) const;

private:
    std::vector<std::vector<int>> looks_da_peca;
    std::vector<std::vector<int>> pecas_do_look;
};
