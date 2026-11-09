#pragma once

#include <array>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "avaliacao_d.hpp"
#include "leitura.hpp"

// Avaliacao justa: toda versao da nota a todas as pecas da categoria da escondida.
// Uso para avaliar uma nota nova (ex.: versao F) na validacao:
//   Base sem_val = base_sem_validacao(base);       // monte grafo, tabelas e popularidade com sem_val
//   Consultas c = consultas_validacao(base);
//   Posicoes p = avaliar_notas(sem_val, c, "F", [&](int q, const std::vector<int>& look,
//                                                  const std::vector<int>& cand, std::vector<double>& nota) { ... });
//   taxa(p, 10, s) da o Hit@10 na ordem s; por_consulta(p, 10) e bootstrap(x, y) comparam duas versoes.
// No teste e igual, com consultas_teste(base).

constexpr int N_ORDENS = 10;  // sementes 1 a 10 do desempate
constexpr int N_BOOTSTRAP = 1000;

struct Consultas {
    std::vector<int> indice_look;         // indice em base.looks
    std::vector<int> alvo;                // indice da peca escondida
    std::vector<std::vector<int>> look;   // look sem a escondida
    std::vector<std::vector<int>> erradas;  // 3 erradas do FITB, vazio se a pergunta ficou de fora
    std::unordered_map<std::string, std::vector<int>> por_cat;
    std::vector<std::vector<int>> ordem;  // ordem[s][peca] = lugar da peca na ordem sorteada s
};

// looks de teste, sortear_escondidas e perguntas do FITB com a semente 13, como no avaliar_fitb
Consultas consultas_teste(const Base& base);

// copia da base com os looks de sortear_validacao (semente 7) fora do treino
Base base_sem_validacao(const Base& base);

// looks de sortear_validacao, perguntas com a semente 11. recebe a base inteira, mas nao olha o teste
Consultas consultas_validacao(const Base& base);

// todas as pecas da categoria da consulta q, fora as do look. a escondida sempre esta aqui
std::vector<int> candidatas(const Base& base, const Consultas& c, int q);

// preenche nota[i] da peca cand[i]. nota chega zerada. q serve pra guardar contas por consulta
using Notas = std::function<void(int q, const std::vector<int>& look, const std::vector<int>& cand,
                                 std::vector<double>& nota)>;

// para versao que so devolve lista curta: peca fora da lista fica com 0
Notas de_lista(const Base& base, const Lista& gerar);

struct Posicoes {
    std::string nome;
    std::vector<int> tam;                          // candidatas da consulta
    std::vector<int> empatadas;                    // outras pecas com a mesma nota da escondida
    std::vector<std::array<int, N_ORDENS>> pos;    // 0 = primeira da lista
    std::vector<std::array<signed char, N_ORDENS>> fitb;  // 1 acerta, 0 erra, -1 sem pergunta
};

// nota maior vem antes. no empate vem antes quem esta antes na ordem s.
// com desempate (um valor por peca, ex.: popularidade), ele decide antes da ordem s
Posicoes avaliar_notas(const Base& base, const Consultas& c, const std::string& nome, const Notas& f,
                       const std::vector<double>* desempate = nullptr);

// k = 1, 5 ou 10 da o Hit@k. k = 0 da o FITB
double taxa(const Posicoes& p, int k, int s);

// media nas 10 ordens de cada consulta. -1 quando a consulta nao tem pergunta de FITB
std::vector<double> por_consulta(const Posicoes& p, int k);

struct Intervalo {
    double dif = 0.0;
    double min = 0.0;
    double max = 0.0;
};

// x - y nas mesmas consultas. sorteia consultas com reposicao e pega os percentis 2,5 e 97,5
Intervalo bootstrap(const std::vector<double>& x, const std::vector<double>& y);

// roda aleatorio, popularidade, A, B, C, D e D nova. validacao = true nunca monta o teste
// novas = true roda tambem as configs de config_novas.csv, separa peca conhecida e peca nova
// e grava tabela_final_2 (ou validacao_final_2). as tabelas de antes nao mudam
bool avaliar_justo(const Base& base, const std::string& pasta_dados, bool validacao, const std::string& pasta_saida,
                   bool novas = false);
