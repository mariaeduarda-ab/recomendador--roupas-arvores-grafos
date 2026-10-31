#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "peca.hpp"

struct Look {
    int id = 0;
    bool treino = true;
    std::vector<int> pecas;  // indices em Base::pecas
};

// as estruturas usam o indice no vetor. nos subconjuntos o id do csv tem buracos
struct Base {
    std::vector<Peca> pecas;
    std::vector<Look> looks;
    std::unordered_map<int, int> indice_da_peca;
    std::unordered_map<int, int> indice_do_look;
};

// toda coluna fora id, categoria e nome vira numero do vetor
std::vector<std::string> separar_colunas(std::string linha);

bool ler_pecas(const std::string& caminho, Base& base, std::string& erro);

// ignora peca fora da base pq os subconjuntos tem so parte das pecas
bool ler_looks(const std::string& caminho, Base& base, std::string& erro);

bool ler_base(const std::string& pasta, Base& base, std::string& erro);
