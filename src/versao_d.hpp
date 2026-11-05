#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "atributos.hpp"
#include "grafo_looks.hpp"
#include "kdtree.hpp"
#include "recomendador.hpp"

// peso 0 desliga a parte: b = 0 tira a regra de cor, c = 0 a de estampa, d = 0 o bonus
struct ConfigD {
    double a = 1.0;
    double b = 1.0;
    double c = 1.0;
    double d = 1.0;
    int k = 5;
    bool mutuo = true;      // false: basta um dos lados
    bool misturar = false;  // busca com grupo 1 + Lab + estampa
};

constexpr double BONUS_COR_NEUTRA = 0.5;
constexpr int M_MONTAR = 5;
constexpr int N_MELHORES_LOOKS = 3;

double regra_cor(const Atrib& p, const Atrib& q);
double regra_estampa(const Atrib& p, const Atrib& q);

struct LookMontado {
    std::vector<int> pecas;
    double soma = 0.0;
};

// conta quantas vezes cada grupo de atributos aparece junto de outro nos looks de treino
class TabelaGrupos {
public:
    void montar(const Base& base, const std::vector<Atrib>& attr);
    double bonus(int p, int q) const;

private:
    std::vector<int> grupo_;
    std::unordered_map<long long, double> juntos_;
    std::unordered_map<int, double> aparicoes_;
};

// o bonus so ve os looks com treino == true na Base e no grafo.
// pra separar validacao, passe uma copia da Base com esses looks marcados como nao treino
class VersaoD {
public:
    VersaoD(const Base& base, const std::vector<Atrib>& attr, const GrafoLooks& grafo, const ConfigD& cfg);

    // so pesos. evita refazer as arestas a cada teste na validacao
    void trocar_pesos(double a, double b, double c, double d);

    double proximidade(int p, int q) const;
    double bonus_grupo(int p, int q) const;
    double nota(int p, int q) const;

    std::vector<Candidata> completar(const std::vector<int>& look, const std::string& categoria, int n) const;

    // mesma nota do completar, mas em todas as pecas de cand, sem a lista curta das arestas
    std::vector<double> notas(const std::vector<int>& look, const std::vector<int>& cand) const;

    // melhores looks a partir de uma peca, do maior pro menor. categoria sem candidata fica de fora
    std::vector<LookMontado> montar(int peca, const std::vector<std::string>& categorias) const;

    double ms_arvores() const { return ms_arvores_; }
    double ms_arestas() const { return ms_arestas_; }

    const std::vector<std::vector<int>>& arestas() const { return adj_; }
    std::vector<Vizinho> vizinhos(int p, const std::string& categoria, int k) const;
    std::vector<Vizinho> vizinhos_bruta(int p, const std::string& categoria, int k) const;

private:
    using Contagem = std::unordered_map<int, double>;

    void montar_arvores();
    void montar_arestas();
    double nota_com(int p, int q, const Contagem& bfs) const;

    const Base& base_;
    const std::vector<Atrib>& attr_;
    const GrafoLooks& grafo_;
    ConfigD cfg_;

    std::unordered_map<std::string, int> id_cat_;
    std::vector<std::vector<Peca>> pecas_cat_;  // Peca::id guarda o indice global
    std::vector<KDTree> arvores_;
    std::vector<std::vector<double>> busca_;
    std::vector<int> local_;
    std::vector<std::vector<int>> adj_;

    double ms_arvores_ = 0.0;
    double ms_arestas_ = 0.0;

    TabelaGrupos grupos_;
};
