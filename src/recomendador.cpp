#include "recomendador.hpp"

double peso_vizinha(double d, double d_ref) {
    // TODO: devolver max(0, 1 - d / d_ref).
    (void)d;
    (void)d_ref;
    return 0.0;
}

Recomendador::Recomendador(const Base& base, const KDTree& arvore, const GrafoLooks& grafo,
                           bool ajuste_popularidade)
    : base_(base), arvore_(arvore), grafo_(grafo), ajuste_popularidade_(ajuste_popularidade) {}

std::vector<Candidata> Recomendador::popularidade(const std::string& categoria, int n) const {
    // TODO: ordenar as peças da categoria por grafo_.num_looks.
    (void)categoria;
    (void)n;
    return {};
}

std::vector<Candidata> Recomendador::versao_a(const std::vector<int>& look, const std::string& categoria,
                                              int n) const {
    // TODO: calcular a média dos vetores e chamar arvore_.knn com o filtro de categoria.
    // TODO: pedir n + look.size() vizinhas, porque as peças do look são retiradas depois.
    (void)look;
    (void)categoria;
    (void)n;
    return {};
}

std::vector<Candidata> Recomendador::versao_b(const std::vector<int>& look, const std::string& categoria,
                                              int n) const {
    // TODO: chamar pontuar_partidas com as peças do look e peso 1.
    (void)look;
    (void)categoria;
    (void)n;
    return {};
}

std::vector<Candidata> Recomendador::versao_c(const std::vector<int>& look, const std::string& categoria,
                                              int n) const {
    // TODO: para cada peça do look, buscar M_VIZINHAS parecidas da mesma categoria.
    // TODO: o peso usa a distância real, ou seja, sqrt(dist2).
    (void)look;
    (void)categoria;
    (void)n;
    return {};
}

std::vector<Candidata> Recomendador::pontuar_partidas(const std::vector<Partida>& partidas,
                                                      const std::vector<int>& look,
                                                      const std::string& categoria, int n) const {
    // TODO: somar peso * contagem da BFS para cada partida.
    // TODO: manter só a categoria pedida, retirar as peças do look e ordenar.
    (void)partidas;
    (void)look;
    (void)categoria;
    (void)n;
    return {};
}

std::vector<Candidata> Recomendador::parecidas(int peca, int k) const {
    // TODO: chamar arvore_.knn com a categoria da peça, excluindo a própria peça.
    (void)peca;
    (void)k;
    return {};
}
