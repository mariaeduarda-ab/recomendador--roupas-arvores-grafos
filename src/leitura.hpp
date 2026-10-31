#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "peca.hpp"

// Um look guarda as peças pelo índice delas em Base::pecas.
struct Look {
    int id = 0;  // look_id que veio do CSV
    bool treino = true;
    std::vector<int> pecas;
};

// Toda estrutura usa o índice (posição no vetor), e não o id do CSV.
// Assim os ids do CSV não precisam ser sequenciais.
struct Base {
    std::vector<Peca> pecas;
    std::vector<Look> looks;
    std::unordered_map<int, int> indice_da_peca;
    std::unordered_map<int, int> indice_do_look;
};

// Lê id, categoria e nome (opcional). Toda outra coluna vira um número do vetor.
bool ler_pecas(const std::string& caminho, Base& base, std::string& erro);

// Lê look_id, peca_id e conjunto. Linhas com peça fora da base são ignoradas,
// porque os subconjuntos de teste de tempo têm só parte das peças.
bool ler_looks(const std::string& caminho, Base& base, std::string& erro);

// Procura pecas.csv e looks.csv na pasta. Se não achar, tenta pecas_exemplo.csv e looks_exemplo.csv.
bool ler_base(const std::string& pasta, Base& base, std::string& erro);
