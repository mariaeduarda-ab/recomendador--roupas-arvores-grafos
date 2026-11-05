#pragma once

#include <array>
#include <string>
#include <utility>
#include <vector>

#include "avaliacao_justa.hpp"
#include "grafo_looks.hpp"
#include "grupos_kruskal.hpp"
#include "pontos_atributos.hpp"
#include "recomendador.hpp"

// nota_F(c) = w1 C(c) + w2 pontos_atributos(c) + w3 pontos_grupos(c) + w4 popularidade(c),
// cada termo entre 0 e 1 dentro da consulta. no empate ganha a mais popular.
// Uso:
//   VersaoF f(base, rec, grafo, pa);             // base sem validacao no ajuste, base inteira no teste
//   TermosF t = f.termos(look, cand);            // uma vez por consulta
//   f.notas(t, nullptr, cand, cfg, nota);        // combina os pesos. com grupos, passe o termo_grupos
//   avaliar_notas(..., &f.popularidade());       // desempate pela popularidade

enum Termo { T_C, T_ATRIB, T_GRUPOS, T_POP, N_TERMOS };

extern const std::array<std::string, N_TERMOS> NOME_TERMO;

struct ConfigF {
    std::array<double, N_TERMOS> w{};    // w1 a w4
    std::array<double, N_ATRIB> peso{};  // peso de cada atributo dentro do termo 2
    int k_grupos = 0;                    // so vale com w3 > 0
    double peso_estampa = PESO_ESTAMPA;
};

// linhas "F" e "C + popularidade" do config_f.csv
bool ler_config_f(const std::string& caminho, ConfigF& f, ConfigF& c_pop);
bool gravar_config_f(const std::string& caminho, const ConfigF& f, const ConfigF& c_pop);

// min-max entre as candidatas. todas iguais, ou uma so, vira tudo 0
void normalizar(std::vector<double>& x);

// termos de uma consulta, sem peso. o atributo depende so do valor da candidata,
// entao guarda a soma no look por valor e nao por peca
struct TermosF {
    std::vector<std::pair<int, double>> c;               // pecas que a BFS da C alcanca. as outras tem 0
    std::array<std::vector<double>, N_ATRIB> por_valor;  // [a][y] = soma, nas pecas do look, dos pontos com o valor y
};

// soma no look dos pontos de cada grupo da categoria pedida. ini = primeiro grupo da categoria
struct TermoGrupos {
    int ini = 0;
    std::vector<double> soma;
};

class VersaoF {
public:
    VersaoF(const Base& base, const Recomendador& rec, const GrafoLooks& grafo, const PontosAtributos& pa);

    TermosF termos(const std::vector<int>& look, const std::vector<int>& cand) const;
    // pontos_atributos(c) de uma candidata, com os pesos de cfg
    double atributos(const TermosF& t, int c, const std::array<double, N_ATRIB>& peso) const;

    // os grupos de uma categoria tem ids seguidos (agrupar numera categoria por categoria)
    static TermoGrupos termo_grupos(const Grupos& gr, const PontosGrupos& pg, const std::vector<int>& look,
                                    const std::vector<int>& cand);

    // tg pode ser nullptr quando w3 = 0. gr e o mesmo agrupamento usado em tg
    void notas(const TermosF& t, const TermoGrupos* tg, const Grupos* gr, const std::vector<int>& cand,
               const ConfigF& cfg, std::vector<double>& nota) const;

    // log(1 + grau no grafo peca-look), por peca
    const std::vector<double>& popularidade() const { return pop_; }

private:
    const Base& base_;
    const Recomendador& rec_;
    const PontosAtributos& pa_;
    std::vector<double> pop_;
    mutable std::vector<double> nota_c_;  // por peca, zerada depois de cada uso
};

// vetor dos grupos com o pecas_v4.csv, como no comando grupos
bool ler_attr_grupos(const std::string& pasta_dados, const Base& base, std::vector<Atrib>& attr, std::string& erro);

// comando ajustar_f: grade na validacao, variacoes, escolha_final.md, grade_f.csv e config_f.csv.
// nunca monta o teste
bool ajustar_f(const Base& base, const std::string& pasta_dados, const std::string& pasta_saida);
