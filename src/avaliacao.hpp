#pragma once

#include <string>
#include <vector>

#include "leitura.hpp"
#include "recomendador.hpp"

// 1 se o alvo está entre as k primeiras da lista. 0 se não está.
int hit_em_k(const std::vector<Candidata>& lista, int alvo, int k);

// Esconde uma peça sorteada de cada look de teste e grava pasta_saida/hit_k.csv.
// Devolve false se nenhuma versão devolveu candidatas.
bool avaliar_hit(const Base& base, const Recomendador& recomendador, const std::string& pasta_saida);

// Mede os tempos nos subconjuntos pecas_N.csv e looks_N.csv e grava pasta_saida/tempos.csv.
// Se não houver subconjuntos, mede a base inteira da pasta.
void medir_tempos(const std::string& pasta_dados, const std::string& pasta_saida);
