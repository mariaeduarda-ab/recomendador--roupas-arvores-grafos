#pragma once

#include <string>
#include <vector>

#include "leitura.hpp"

// so o grupo 1 entra na busca. o grupo 2 so muda a nota
struct Atrib {
    std::vector<double> g1;  // 5 de estilo, 5 de ocasiao e formalidade
    std::vector<double> lab;
    bool neutra = false;
    std::string estampa = "desconhecido";
    std::string comprimento = "desconhecido";
    std::string modelagem = "desconhecido";
};

// attr[i] e da peca base.pecas[i]. ignora id fora da base pq os subconjuntos tem so parte
bool ler_atributos(const std::string& caminho, const Base& base, std::vector<Atrib>& attr, std::string& erro);

// pecas_v2.csv ou pecas_v2_exemplo.csv, igual ao ler_base
bool ler_atributos_da_pasta(const std::string& pasta, const Base& base, std::vector<Atrib>& attr, std::string& erro);
