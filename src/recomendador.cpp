#include "recomendador.hpp"

#include <algorithm>
#include <cmath>

namespace {

// desempata pelo indice. a ordem do unordered_map nao e garantida
void ordenar_e_cortar(std::vector<Candidata>& lista, int n) {
    std::sort(lista.begin(), lista.end(), [](const Candidata& a, const Candidata& b) {
        if (a.pontuacao != b.pontuacao) return a.pontuacao > b.pontuacao;
        return a.indice < b.indice;
    });
    if ((int)lista.size() > n) lista.resize(n);
}

bool esta_no_look(const std::vector<int>& look, int p) {
    return std::find(look.begin(), look.end(), p) != look.end();
}

}  // namespace

double peso_vizinha(double d, double d_ref) {
    return std::max(0.0, 1.0 - d / d_ref);
}

Recomendador::Recomendador(const Base& base, const KDTree& arvore, const GrafoLooks& grafo,
                           bool ajuste_popularidade)
    : base_(base), arvore_(arvore), grafo_(grafo), ajuste_popularidade_(ajuste_popularidade) {
    // calcula uma vez so. refazer a cada look de teste percorreria a base inteira toda vez
    for (int i = 0; i < (int)base.pecas.size(); i++) {
        ranking_[base.pecas[i].categoria].push_back({i, (double)grafo.num_looks(i)});
    }
    for (auto& par : ranking_) ordenar_e_cortar(par.second, (int)par.second.size());
}

std::vector<Candidata> Recomendador::popularidade(const std::string& categoria, int n) const {
    auto it = ranking_.find(categoria);
    if (it == ranking_.end()) return {};
    int total = std::min(n, (int)it->second.size());
    return std::vector<Candidata>(it->second.begin(), it->second.begin() + total);
}

std::vector<Candidata> Recomendador::versao_a(const std::vector<int>& look, const std::string& categoria,
                                              int n) const {
    if (look.empty()) return {};
    std::vector<double> media(base_.pecas[look[0]].v.size(), 0.0);
    for (int p : look) {
        for (int i = 0; i < (int)media.size(); i++) media[i] += base_.pecas[p].v[i];
    }
    for (double& x : media) x /= look.size();

    // pede a mais pq as pecas do look saem depois
    std::vector<Vizinho> viz = arvore_.knn(media, n + (int)look.size(), &categoria);
    std::vector<Candidata> res;
    for (const Vizinho& v : viz) {
        if (!esta_no_look(look, v.indice)) res.push_back({v.indice, 1.0 / (1.0 + std::sqrt(v.dist2))});
    }
    ordenar_e_cortar(res, n);
    return res;
}

std::vector<Candidata> Recomendador::versao_b(const std::vector<int>& look, const std::string& categoria,
                                              int n) const {
    std::vector<Partida> partidas;
    for (int p : look) partidas.push_back({p, 1.0});
    return pontuar_partidas(partidas, look, categoria, n);
}

std::vector<Candidata> Recomendador::versao_c(const std::vector<int>& look, const std::string& categoria,
                                              int n) const {
    std::vector<Partida> partidas;
    for (int p : look) {
        partidas.push_back({p, 1.0});
        const Peca& peca = base_.pecas[p];
        for (const Vizinho& v : arvore_.knn(peca.v, M_VIZINHAS, &peca.categoria, p)) {
            double peso = peso_vizinha(std::sqrt(v.dist2));
            if (peso > 0 && !esta_no_look(look, v.indice)) partidas.push_back({v.indice, peso});
        }
    }
    return pontuar_partidas(partidas, look, categoria, n);
}

std::vector<Candidata> Recomendador::pontuar_partidas(const std::vector<Partida>& partidas,
                                                      const std::vector<int>& look,
                                                      const std::string& categoria, int n) const {
    std::unordered_map<int, double> soma;
    for (const Partida& partida : partidas) {
        for (const auto& par : grafo_.bfs_2_passos(partida.indice, ajuste_popularidade_)) {
            soma[par.first] += partida.peso * par.second;
        }
    }

    std::vector<Candidata> res;
    for (const auto& par : soma) {
        if (base_.pecas[par.first].categoria == categoria && !esta_no_look(look, par.first)) {
            res.push_back({par.first, par.second});
        }
    }
    ordenar_e_cortar(res, n);
    return res;
}

std::vector<Candidata> Recomendador::parecidas(int peca, int k) const {
    const Peca& p = base_.pecas[peca];
    std::vector<Candidata> res;
    for (const Vizinho& v : arvore_.knn(p.v, k, &p.categoria, peca)) {
        res.push_back({v.indice, 1.0 / (1.0 + std::sqrt(v.dist2))});
    }
    return res;
}
