#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "avaliacao_justa.hpp"
#include "grafo_looks.hpp"
#include "grupos_kruskal.hpp"
#include "pontos_atributos.hpp"
#include "recomendador.hpp"
#include "versao_f.hpp"

// Variacoes para a peca nova (sem look de treino), todas escolhidas so na validacao:
// a metade A escolhe e a metade B confirma. Uma variacao so fica se ganhar da F nas duas.
//   grupos 1: grupos do Kruskal da F. grupos 2: forma 3 (cor, estampa, formalidade) com recuo
//   para categoria + cor quando o par de grupos tem menos de MIN_PARES pares no treino, e depois 0.
//   a > 0: atributos e grupos ganham peso a / (1 + grau) e C e popularidade ficam iguais.
//   logistica: pesos aprendidos por descida de gradiente na metade A (pares certo x sorteados).
// nota = w1 C + w4 pop + fator (w2 atrib + w3 grupos), cada termo de 0 a 1 dentro da consulta,
// com fator = 1 sem grau e a / (1 + grau) com grau.

constexpr int MIN_PARES = 20;
constexpr int N_FEAT = 13;

struct ConfigN {
    std::string nome;
    std::array<double, N_TERMOS> w{};
    std::array<double, N_ATRIB> peso{};
    int grupos = 0;  // 0 sem grupos, 1 Kruskal da F, 2 forma 3 com recuo
    double a = 0.0;  // 0 = sem peso por grau
    bool logistica = false;
    std::array<double, N_FEAT> beta{};
};

bool ler_config_novas(const std::string& caminho, std::vector<ConfigN>& cfgs);

// tudo que as notas novas precisam. b, rec e grafo precisam viver enquanto as notas forem usadas
struct Novas {
    ValoresAtrib val;
    std::unique_ptr<PontosAtributos> pa;
    std::unique_ptr<VersaoF> f;
    Grupos gk, g3, g2;
    std::unique_ptr<PontosGrupos> pk, p3, p2;
    std::vector<double> grau;

    bool montar(const Base& b, const Recomendador& rec, const GrafoLooks& grafo, const std::string& pasta_dados,
                const ConfigF& cfg_f, std::string& erro);
    // soma no look dos pontos de cada grupo da forma 3, com o recuo
    TermoGrupos termo_forma3(const std::vector<int>& look, const std::vector<int>& cand) const;
    // as 13 colunas de cada candidata, ja de 0 a 1 dentro da consulta
    void colunas(const TermosF& t, const TermoGrupos& tk, const TermoGrupos& t3, const std::vector<int>& cand,
                 std::vector<std::array<double, N_FEAT>>& x) const;
    void notas(const TermosF& t, const TermoGrupos& tk, const TermoGrupos& t3, const std::vector<int>& cand,
               const ConfigN& cfg, std::vector<double>& nota) const;
    Notas notas(const ConfigN& cfg) const;
};

extern const std::array<std::string, N_FEAT> NOME_FEAT;

// comando ajustar_novas: grava novas_validacao.md, novas_grade.csv e config_novas.csv. nunca monta o teste
bool ajustar_novas(const Base& base, const std::string& pasta_dados, const std::string& pasta_saida);
