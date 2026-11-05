#pragma once

#include <array>
#include <string>
#include <vector>

#include "leitura.hpp"

// Uso na versao F:
//   ValoresAtrib val;
//   ler_valores("dados/pecas_v5.csv", base, val, erro);  // uma vez so, nao olha os looks
//   PontosAtributos pa(base_sem_validacao(base), val);  // na validacao. no teste final, PontosAtributos pa(base, val)
//   pa.peso[ESTAMPA] = 0.5;                             // pesos padrao 1, mudam sem recalcular as tabelas
//   pa.pontos(p, c);                                    // p e c sao indices em base.pecas. mesma categoria da 0
//   pa.explicar(p, c);                                  // "estampa lisa com listrada: +0,40; ..."
// As tabelas contam so os looks com treino = true.

enum Atributo { COR, ESTAMPA, FORMALIDADE, ESTILO, OCASIAO, COMPRIMENTO, N_ATRIB };

extern const std::array<std::string, N_ATRIB> NOME_ATRIB;

constexpr double MIN_PARES_CATEGORIA = 200;  // abaixo disso usa os grupos de categoria

struct ValoresAtrib {
    std::array<std::vector<std::string>, N_ATRIB> nomes;  // nomes[a][x] = valor x do atributo a
    // val[a][i] = valores marcados da peca i. vazio = desconhecido. estilo e ocasiao podem ter varios
    std::array<std::vector<std::vector<int>>, N_ATRIB> val;
    // id do valor, criando se ainda nao existe
    int valor(int a, const std::string& nome);
};

// parte de cima, parte de baixo, peca unica, calcado, bolsa ou acessorio
std::string grupo_categoria(const std::string& categoria);

// baixa, media, alta ou desconhecido (0,5 e o valor de quem nao tem palavra formal nem casual)
std::string faixa_formalidade(double f);

// pecas_v5.csv: pecas_v4.csv + grupo_cor. peca fora da base e ignorada, peca da base sem linha da erro
bool ler_valores(const std::string& caminho, const Base& base, ValoresAtrib& val, std::string& erro);

// contagens de um par de categorias (ou de grupos) num atributo
struct Tabela {
    double total = 0;          // pares com os dois valores conhecidos
    std::vector<double> n;     // n[x * nv + y]
    std::vector<double> soma_a;  // aparicoes de cada valor do lado A nesses pares
    std::vector<double> soma_b;
};

class PontosAtributos {
public:
    PontosAtributos(const Base& base, const ValoresAtrib& val);

    std::array<double, N_ATRIB> peso;

    double pontos(int p, int c) const;            // soma de peso x pontos de cada atributo
    double pontos_de(int a, int p, int c) const;  // um atributo, sem peso
    std::string explicar(int p, int c) const;

    // tabela usada para as categorias de p e c (a de categoria ou a de grupo), nullptr se mesma categoria
    const Tabela* tabela(int a, int p, int c) const;
    bool usa_grupo(const std::string& cat_a, const std::string& cat_b) const;
    double pares(const std::string& cat_a, const std::string& cat_b) const;

    // le direto na tabela de categorias (grupo = false) ou de grupos
    const Tabela* tabela_de(int a, bool grupo, const std::string& x, const std::string& y) const;
    const std::vector<std::string>& categorias() const { return cats_; }
    const std::vector<std::string>& grupos() const { return grupos_; }
    const ValoresAtrib& valores() const { return val_; }

private:
    const ValoresAtrib& val_;
    std::vector<std::string> cats_;
    std::vector<std::string> grupos_;
    std::vector<int> cat_;      // categoria de cada peca
    std::vector<int> grupo_da_cat_;
    std::vector<double> pares_;  // pares de pecas por par de categorias, n_cat x n_cat
    std::array<std::vector<Tabela>, N_ATRIB> por_cat_;    // [a][A * n_cat + B]
    std::array<std::vector<Tabela>, N_ATRIB> por_grupo_;  // [a][G * n_grupo + H]
};

// esperado(x, y) = total x fracao de x no lado A x fracao de y no lado B
double esperado(const Tabela& t, int nv, int x, int y);
double pontos_tabela(const Tabela& t, int nv, int x, int y);

// comando pontos: tabelas com todos os looks de treino, csv, resumo e o diagnostico na validacao
bool rodar_pontos(const Base& base, const std::string& pasta_dados, const std::string& pasta_saida);
