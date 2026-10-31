#pragma once

#include <string>
#include <vector>

#include "grafo_looks.hpp"
#include "kdtree.hpp"
#include "leitura.hpp"

// Fáceis de mudar nos testes.
constexpr double D_REF = 1.0;    // distância de referência do peso das vizinhas
constexpr int M_VIZINHAS = 5;    // quantas peças parecidas entram para cada peça do look

struct Candidata {
    int indice = -1;
    double pontuacao = 0.0;
};

// Peça de partida da BFS e o peso dela.
struct Partida {
    int indice = -1;
    double peso = 0.0;
};

// Peso de uma vizinha a uma distância d: max(0, 1 - d / d_ref).
double peso_vizinha(double d, double d_ref = D_REF);

// ESQUELETO. A dupla implementa esta classe.
// Toda lista volta ordenada da maior para a menor pontuação, com no máximo n candidatas.
// Peças que já estão no look nunca são candidatas.
class Recomendador {
public:
    Recomendador(const Base& base, const KDTree& arvore, const GrafoLooks& grafo,
                 bool ajuste_popularidade = false);

    // Peças da categoria que mais aparecem nos looks de treino.
    std::vector<Candidata> popularidade(const std::string& categoria, int n) const;

    // A. Proximidade até a média dos vetores das peças do look.
    std::vector<Candidata> versao_a(const std::vector<int>& look, const std::string& categoria, int n) const;

    // B. Contagem da BFS de 2 passos a partir das peças do look.
    std::vector<Candidata> versao_b(const std::vector<int>& look, const std::string& categoria, int n) const;

    // C. Monta as partidas (peças do look com peso 1 e até M_VIZINHAS parecidas de cada uma,
    // da mesma categoria, com peso_vizinha) e chama pontuar_partidas.
    std::vector<Candidata> versao_c(const std::vector<int>& look, const std::string& categoria, int n) const;

    // Pontuação de c: soma de peso(q) * (looks em que q e c aparecem juntas).
    std::vector<Candidata> pontuar_partidas(const std::vector<Partida>& partidas, const std::vector<int>& look,
                                            const std::string& categoria, int n) const;

    // Busca kNN direta, filtrada pela categoria da peça consultada.
    std::vector<Candidata> parecidas(int peca, int k) const;

private:
    const Base& base_;
    const KDTree& arvore_;
    const GrafoLooks& grafo_;
    bool ajuste_popularidade_;
};
