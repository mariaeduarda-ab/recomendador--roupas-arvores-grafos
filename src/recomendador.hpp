#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "grafo_looks.hpp"
#include "kdtree.hpp"
#include "leitura.hpp"

constexpr double D_REF = 1.0;  // distancia em que o peso da vizinha chega a 0
constexpr int M_VIZINHAS = 5;  // parecidas por peca do look na versao C

struct Candidata {
    int indice = -1;
    double pontuacao = 0.0;
};

struct Partida {
    int indice = -1;
    double peso = 0.0;
};

double peso_vizinha(double d, double d_ref = D_REF);
void ordenar_e_cortar(std::vector<Candidata>& lista, int n);

// as listas voltam da maior pontuacao pra menor, e peca do look nunca e candidata
class Recomendador {
public:
    // o grafo ja precisa estar construido, o ranking de popularidade sai dele
    Recomendador(const Base& base, const KDTree& arvore, const GrafoLooks& grafo,
                 bool ajuste_popularidade = false);

    std::vector<Candidata> popularidade(const std::vector<int>& look, const std::string& categoria, int n) const;

    // A: proximidade ate a media dos vetores do look
    std::vector<Candidata> versao_a(const std::vector<int>& look, const std::string& categoria, int n) const;
    // B: BFS so a partir das pecas do look
    std::vector<Candidata> versao_b(const std::vector<int>& look, const std::string& categoria, int n) const;
    // C: BFS a partir das pecas do look e das parecidas com elas
    std::vector<Candidata> versao_c(const std::vector<int>& look, const std::string& categoria, int n) const;

    std::vector<Candidata> pontuar_partidas(const std::vector<Partida>& partidas, const std::vector<int>& look,
                                            const std::string& categoria, int n) const;

    std::vector<Candidata> parecidas(int peca, int k) const;

private:
    const Base& base_;
    const KDTree& arvore_;
    const GrafoLooks& grafo_;
    bool ajuste_popularidade_;
    std::unordered_map<std::string, std::vector<Candidata>> ranking_;
};
