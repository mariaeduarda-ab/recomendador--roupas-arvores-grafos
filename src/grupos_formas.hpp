#pragma once

#include <string>

#include "leitura.hpp"

// Criterio escrito antes de rodar: entre as formas 1, 2 e 3, fica a de menor media (entre as categorias)
// da fracao do maior grupo, desde que no maximo 20% dos grupos tenham 1 peca so.
// Se nenhuma passar, fica a com menos grupos de 1 peca. O Kruskal puro so entra como comparacao.
constexpr double MAX_UNITARIOS = 0.20;
constexpr int K_FORMAS = 50;              // parada do Kruskal puro e do limitado
constexpr double FRACAO_LIMITE = 0.05;    // maior grupo do Kruskal limitado

// compara as formas sem olhar looks, escolhe pelo criterio acima e, se a forma usa o vetor,
// escolhe o peso da estampa (0; 0,1; 0,2) pelo Hit@10 na validacao de C + pontos_grupos
bool comparar_formas(const Base& base, const std::string& pasta_dados, const std::string& pasta_saida);
