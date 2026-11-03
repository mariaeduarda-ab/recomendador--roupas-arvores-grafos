#pragma once

#include <string>
#include <vector>

#include "avaliacao.hpp"
#include "leitura.hpp"
#include "recomendador.hpp"

constexpr int N_ERRADAS = 3;

// a peca escondida e 3 pecas sorteadas da mesma categoria, que tambem aparecem (ou nao) no treino como ela.
// sem pecas suficientes, a pergunta fica de fora
struct Pergunta {
    Escondida escondida;
    int certa = -1;
    std::vector<int> opcoes;  // inclui a certa
};

std::vector<Pergunta> montar_perguntas(const Base& base, const std::vector<Escondida>& testes, unsigned semente);

// peca fora da lista perde de quem esta nela. no empate a nota e 1/empatadas, que e a media de sortear
double credito(const std::vector<Candidata>& lista, const Pergunta& p);

// escolhe os pesos na validacao, roda as perguntas de teste e grava os csv em pasta_saida
bool avaliar_fitb(const Base& base, const std::string& pasta_dados, const std::string& tabela_cores,
                  const std::string& pasta_saida);
