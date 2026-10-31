#pragma once

#include <string>
#include <vector>

#include "leitura.hpp"
#include "recomendador.hpp"

int hit_em_k(const std::vector<Candidata>& lista, int alvo, int k);

// esconde uma peca de cada look de teste e grava hit_k.csv
void avaliar_hit(const Base& base, const Recomendador& rec, const GrafoLooks& grafo, const std::string& pasta_saida);

// usa pecas_N.csv e looks_N.csv. sem subconjuntos, mede a base inteira
void medir_tempos(const std::string& pasta_dados, const std::string& pasta_saida);
