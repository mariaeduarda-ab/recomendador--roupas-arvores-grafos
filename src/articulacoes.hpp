#pragma once

#include <string>

#include "leitura.hpp"

// Analise depois do congelamento: nao muda nenhuma recomendacao.
// Grafo entre os grupos da forma escolhida (forma 3). Aresta = par de grupos de categorias diferentes com
// pelo menos MIN_N_ARESTA pares nos looks de treino e pontos acima do limiar. Os 3 limiares sao os quartis
// dos pontos das arestas positivas que passam no MIN_N_ARESTA.
constexpr int MIN_N_ARESTA = 3;      // 1 ou 2 pares podem ser um look so
constexpr int MIN_SEPARADOS = 3;     // articulacao que solta 1 ou 2 grupos fica de fora
constexpr int N_TABELA = 10;
constexpr int REPETICOES_TEMPO = 1000;

// comando articulacoes: Tarjan nos 3 limiares, arquivos do Gephi e resultados/articulacoes.md
bool rodar_articulacoes(const Base& base, const std::string& pasta_dados, const std::string& pasta_saida);
