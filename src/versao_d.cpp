#include "versao_d.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <queue>
#include <unordered_set>

namespace {

constexpr int N_ESTILOS = 5;

bool tem_estampa(const Atrib& x) {
    return x.estampa != "lisa" && x.estampa != "desconhecido";
}

int grupo_da_peca(const Atrib& x, int cat) {
    int estilo = N_ESTILOS;
    for (int i = N_ESTILOS - 1; i >= 0; i--) {
        if (x.g1[i] > 0.5) estilo = i;
    }
    int estampa = tem_estampa(x) ? 2 : (x.estampa == "lisa" ? 1 : 0);
    return cat * 100 + estilo * 10 + estampa;
}

long long chave_par(int a, int b) {
    return ((long long)a << 32) | (unsigned)b;
}

bool no_look(const std::vector<int>& look, int p) {
    return std::find(look.begin(), look.end(), p) != look.end();
}

}  // namespace

double regra_cor(const Atrib& p, const Atrib& q) {
    return p.neutra || q.neutra ? BONUS_COR_NEUTRA : 0.0;
}

double regra_estampa(const Atrib& p, const Atrib& q) {
    // desconhecido e neutro: a descricao quase nunca diz que a peca e lisa
    if (p.estampa == "desconhecido" || q.estampa == "desconhecido") return 0.0;
    if (tem_estampa(p) && tem_estampa(q)) return -1.0;
    if (tem_estampa(p) != tem_estampa(q)) return 1.0;
    return 0.0;
}

void TabelaGrupos::montar(const Base& base, const std::vector<Atrib>& attr) {
    std::unordered_map<std::string, int> id_cat;
    grupo_.resize(base.pecas.size());
    for (int i = 0; i < (int)base.pecas.size(); i++) {
        int c = id_cat.emplace(base.pecas[i].categoria, (int)id_cat.size()).first->second;
        grupo_[i] = grupo_da_peca(attr[i], c);
    }
    for (const Look& look : base.looks) {
        if (!look.treino) continue;
        for (int p : look.pecas) {
            aparicoes_[grupo_[p]] += 1.0;
            for (int q : look.pecas) {
                if (base.pecas[p].categoria != base.pecas[q].categoria) juntos_[chave_par(grupo_[p], grupo_[q])] += 1.0;
            }
        }
    }
}

// divide pelos dois grupos pra ficar perto de 0 a 1. um grupo tem milhares de looks,
// dividir so por um deixaria esse bonus muito maior que as outras partes da nota
double TabelaGrupos::bonus(int p, int q) const {
    auto it = juntos_.find(chave_par(grupo_[p], grupo_[q]));
    if (it == juntos_.end()) return 0.0;
    return it->second / std::sqrt(aparicoes_.at(grupo_[p]) * aparicoes_.at(grupo_[q]));
}

VersaoD::VersaoD(const Base& base, const std::vector<Atrib>& attr, const GrafoLooks& grafo, const ConfigD& cfg)
    : base_(base), attr_(attr), grafo_(grafo), cfg_(cfg) {
    auto t0 = std::chrono::steady_clock::now();
    montar_arvores();
    auto t1 = std::chrono::steady_clock::now();
    montar_arestas();
    auto t2 = std::chrono::steady_clock::now();
    ms_arvores_ = std::chrono::duration<double, std::milli>(t1 - t0).count();
    ms_arestas_ = std::chrono::duration<double, std::milli>(t2 - t1).count();
    grupos_.montar(base_, attr_);
}

void VersaoD::trocar_pesos(double a, double b, double c, double d) {
    cfg_.a = a;
    cfg_.b = b;
    cfg_.c = c;
    cfg_.d = d;
}

void VersaoD::montar_arvores() {
    int n = (int)base_.pecas.size();
    busca_.resize(n);
    local_.resize(n);
    for (int i = 0; i < n; i++) {
        busca_[i] = attr_[i].g1;
        if (cfg_.misturar) {
            busca_[i].insert(busca_[i].end(), attr_[i].lab.begin(), attr_[i].lab.end());
            busca_[i].push_back(tem_estampa(attr_[i]) ? 1.0 : 0.0);
        }
        auto novo = id_cat_.emplace(base_.pecas[i].categoria, (int)id_cat_.size());
        if (novo.second) pecas_cat_.emplace_back();
        int c = novo.first->second;
        Peca p;
        p.id = i;
        p.v = busca_[i];
        local_[i] = (int)pecas_cat_[c].size();
        pecas_cat_[c].push_back(p);
    }
    // so depois de encher pecas_cat_: a arvore guarda ponteiro pro vetor
    for (const std::vector<Peca>& lista : pecas_cat_) {
        arvores_.emplace_back((int)lista[0].v.size());
        arvores_.back().construir(lista);
    }
}

void VersaoD::montar_arestas() {
    int n = (int)base_.pecas.size();
    std::vector<std::vector<int>> viz(n);
    for (int p = 0; p < n; p++) {
        int cat_p = id_cat_.at(base_.pecas[p].categoria);
        for (int c = 0; c < (int)arvores_.size(); c++) {
            if (c == cat_p) continue;
            for (const Vizinho& v : arvores_[c].knn(busca_[p], cfg_.k)) viz[p].push_back(pecas_cat_[c][v.indice].id);
        }
        std::sort(viz[p].begin(), viz[p].end());
    }

    adj_.assign(n, {});
    for (int p = 0; p < n; p++) {
        for (int q : viz[p]) {
            if (!cfg_.mutuo) {
                adj_[p].push_back(q);
                adj_[q].push_back(p);
            } else if (std::binary_search(viz[q].begin(), viz[q].end(), p)) {
                adj_[p].push_back(q);
            }
        }
    }
    for (std::vector<int>& lista : adj_) {
        std::sort(lista.begin(), lista.end());
        lista.erase(std::unique(lista.begin(), lista.end()), lista.end());
    }
}

double VersaoD::proximidade(int p, int q) const {
    const std::vector<double>& x = attr_[p].g1;
    return 1.0 - std::sqrt(distancia2(x, attr_[q].g1) / x.size());
}

double VersaoD::bonus_grupo(int p, int q) const {
    return grupos_.bonus(p, q);
}

double VersaoD::nota_com(int p, int q, const Contagem& bfs) const {
    double bonus = 0.0;
    if (cfg_.d != 0.0) {
        auto it = bfs.find(q);
        bonus = it != bfs.end() ? it->second : bonus_grupo(p, q);
    }
    return cfg_.a * proximidade(p, q) + cfg_.b * regra_cor(attr_[p], attr_[q]) +
           cfg_.c * regra_estampa(attr_[p], attr_[q]) + cfg_.d * bonus;
}

double VersaoD::nota(int p, int q) const {
    return nota_com(p, q, grafo_.bfs_2_passos(p, true));
}

std::vector<Candidata> VersaoD::completar(const std::vector<int>& look, const std::string& categoria, int n) const {
    std::unordered_set<int> cand;
    for (int p : look) {
        for (int q : adj_[p]) {
            if (base_.pecas[q].categoria == categoria && !no_look(look, q)) cand.insert(q);
        }
    }

    // peca da mesma categoria nao tem aresta com a candidata
    std::vector<int> partida;
    std::vector<Contagem> bfs;
    for (int p : look) {
        if (base_.pecas[p].categoria != categoria) {
            partida.push_back(p);
            bfs.push_back(grafo_.bfs_2_passos(p, true));
        }
    }

    std::vector<Candidata> res;
    for (int c : cand) {
        double soma = 0.0;
        for (int i = 0; i < (int)partida.size(); i++) soma += nota_com(partida[i], c, bfs[i]);
        res.push_back({c, soma});
    }
    ordenar_e_cortar(res, n);
    return res;
}

std::vector<double> VersaoD::notas(const std::vector<int>& look, const std::vector<int>& cand) const {
    std::vector<double> res(cand.size(), 0.0);
    if (cand.empty()) return res;
    const std::string& categoria = base_.pecas[cand[0]].categoria;
    for (int p : look) {
        if (base_.pecas[p].categoria == categoria) continue;
        const Contagem bfs = grafo_.bfs_2_passos(p, true);
        for (std::size_t i = 0; i < cand.size(); i++) res[i] += nota_com(p, cand[i], bfs);
    }
    return res;
}

std::vector<LookMontado> VersaoD::montar(int peca, const std::vector<std::string>& categorias) const {
    const Contagem bfs_peca = grafo_.bfs_2_passos(peca, true);
    std::vector<std::vector<int>> combos = {{peca}};
    for (const std::string& cat : categorias) {
        std::vector<Candidata> melhores;
        for (int q : adj_[peca]) {
            if (base_.pecas[q].categoria == cat) melhores.push_back({q, nota_com(peca, q, bfs_peca)});
        }
        ordenar_e_cortar(melhores, M_MONTAR);
        if (melhores.empty()) continue;

        std::vector<std::vector<int>> novos;
        for (const std::vector<int>& combo : combos) {
            for (const Candidata& c : melhores) {
                novos.push_back(combo);
                novos.back().push_back(c.indice);
            }
        }
        combos = novos;
    }

    std::unordered_map<int, Contagem> bfs;
    for (const std::vector<int>& combo : combos) {
        for (int p : combo) {
            if (!bfs.count(p)) bfs[p] = grafo_.bfs_2_passos(p, true);
        }
    }

    // heap de minimo: o topo e o pior dos 3 guardados
    using Item = std::pair<double, int>;
    std::priority_queue<Item, std::vector<Item>, std::greater<Item>> heap;
    for (int i = 0; i < (int)combos.size(); i++) {
        double soma = 0.0;
        for (int x = 0; x < (int)combos[i].size(); x++) {
            for (int y = x + 1; y < (int)combos[i].size(); y++) {
                int p = combos[i][x];
                int q = combos[i][y];
                if (base_.pecas[p].categoria != base_.pecas[q].categoria) soma += nota_com(p, q, bfs[p]);
            }
        }
        heap.push({soma, i});
        if ((int)heap.size() > N_MELHORES_LOOKS) heap.pop();
    }

    std::vector<LookMontado> res(heap.size());
    for (int i = (int)res.size() - 1; i >= 0; i--) {
        res[i].soma = heap.top().first;
        res[i].pecas = combos[heap.top().second];
        heap.pop();
    }
    return res;
}

std::vector<Vizinho> VersaoD::vizinhos(int p, const std::string& categoria, int k) const {
    auto it = id_cat_.find(categoria);
    if (it == id_cat_.end()) return {};
    int c = it->second;
    int excluir = base_.pecas[p].categoria == categoria ? local_[p] : -1;
    std::vector<Vizinho> res = arvores_[c].knn(busca_[p], k, nullptr, excluir);
    for (Vizinho& v : res) v.indice = pecas_cat_[c][v.indice].id;
    return res;
}

std::vector<Vizinho> VersaoD::vizinhos_bruta(int p, const std::string& categoria, int k) const {
    auto it = id_cat_.find(categoria);
    if (it == id_cat_.end()) return {};
    int c = it->second;
    int excluir = base_.pecas[p].categoria == categoria ? local_[p] : -1;
    std::vector<Vizinho> res = arvores_[c].forca_bruta(busca_[p], k, nullptr, excluir);
    for (Vizinho& v : res) v.indice = pecas_cat_[c][v.indice].id;
    return res;
}
