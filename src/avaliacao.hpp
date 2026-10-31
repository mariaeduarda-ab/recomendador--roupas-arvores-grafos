#pragma once

#include <string>
#include <vector>

#include "leitura.hpp"
#include "recomendador.hpp"

struct Escondida {
    int look = -1;
    int pos = -1;  // posicao da peca em look.pecas
};

// sorteia uma peca de cada look da lista com a semente fixa. looks com menos de 2 pecas ficam de fora
std::vector<Escondida> sortear_escondidas(const Base& base, const std::vector<int>& looks);

int hit_em_k(const std::vector<Candidata>& lista, int alvo, int k);

// esconde uma peca de cada look de teste e grava hit_k.csv
void avaliar_hit(const Base& base, const Recomendador& rec, const GrafoLooks& grafo, const std::string& pasta_saida);

// usa pecas_N.csv e looks_N.csv. sem subconjuntos, mede a base inteira
void medir_tempos(const std::string& pasta_dados, const std::string& pasta_saida);
