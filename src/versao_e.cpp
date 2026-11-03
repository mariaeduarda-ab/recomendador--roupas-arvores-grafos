#include "versao_e.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <sstream>
#include <unordered_set>

namespace {

constexpr double PONTO_MESMA_COR = 1.0;
constexpr double PONTO_COMBINA = 0.5;
constexpr double PONTO_NAO_COMBINA = -0.5;

bool e_roupa(Tipo t) {
    return t != Tipo::COMPLEMENTO;
}

bool cima_e_baixo(Tipo a, Tipo b) {
    return (a == Tipo::CIMA && b == Tipo::BAIXO) || (a == Tipo::BAIXO && b == Tipo::CIMA);
}

}  // namespace

void TabelaCores::crescer() {
    int n = (int)nomes_.size();
    cromatica_.resize(n, 0);
    combina_.resize(n);
    for (std::vector<char>& linha : combina_) linha.resize(n, 0);
}

int TabelaCores::id(const std::string& cor) {
    auto novo = nomes_.emplace(cor, (int)nomes_.size());
    if (novo.second) crescer();
    return novo.first->second;
}

void TabelaCores::adicionar(const std::string& cor, const std::vector<std::string>& combina) {
    int a = id(cor);
    cromatica_[a] = 1;
    for (const std::string& outra : combina) {
        int b = id(outra);
        cromatica_[b] = 1;
        combina_[a][b] = 1;
        combina_[b][a] = 1;
    }
}

bool TabelaCores::cromatica(int cor) const {
    return cromatica_[cor] != 0;
}

bool TabelaCores::lista_combina(int a, int b) const {
    return combina_[a][b] != 0;
}

bool TabelaCores::ler(const std::string& caminho, std::string& erro) {
    std::ifstream arquivo(caminho);
    if (!arquivo) {
        erro = "Não foi possível abrir " + caminho;
        return false;
    }
    std::string linha;
    std::getline(arquivo, linha);
    while (std::getline(arquivo, linha)) {
        if (!linha.empty() && linha.back() == '\r') linha.pop_back();
        if (linha.empty()) continue;
        std::size_t virgula = linha.find(',');
        if (virgula == std::string::npos) {
            erro = "Linha sem vírgula em " + caminho;
            return false;
        }
        std::vector<std::string> combina;
        std::stringstream resto(linha.substr(virgula + 1));
        std::string cor;
        while (std::getline(resto, cor, ';')) combina.push_back(cor);
        adicionar(linha.substr(0, virgula), combina);
    }
    return true;
}

bool ler_atributos_e(const std::string& caminho, const Base& base, TabelaCores& cores, std::vector<AtribE>& attr,
                     std::string& erro) {
    std::ifstream arquivo(caminho);
    if (!arquivo) {
        erro = "Não foi possível abrir " + caminho;
        return false;
    }
    std::string linha;
    std::getline(arquivo, linha);
    attr.assign(base.pecas.size(), AtribE{});
    std::vector<char> lida(base.pecas.size(), 0);
    while (std::getline(arquivo, linha)) {
        std::vector<std::string> col = separar_colunas(linha);
        if (col.size() != 3) {
            erro = "Linha com formato errado em " + caminho + ": " + linha;
            return false;
        }
        auto it = base.indice_da_peca.find(std::stoi(col[0]));
        if (it == base.indice_da_peca.end()) continue;
        attr[it->second].cor = cores.id(col[1]);
        attr[it->second].atencao = std::stod(col[2]);
        lida[it->second] = 1;
    }
    if (std::find(lida.begin(), lida.end(), 0) != lida.end()) {
        erro = "Peça sem atributos em " + caminho;
        return false;
    }
    return true;
}

Tipo tipo_da_categoria(const std::string& categoria) {
    if (categoria == "top" || categoria == "outerwear") return Tipo::CIMA;
    if (categoria == "pants" || categoria == "skirt") return Tipo::BAIXO;
    if (categoria == "dress" || categoria == "jumpsuit") return Tipo::INTEIRA;
    return Tipo::COMPLEMENTO;
}

double regra_cor_e(const TabelaCores& cores, int cor_p, Tipo tipo_p, int cor_q, Tipo tipo_q) {
    if (!cores.cromatica(cor_p) || !cores.cromatica(cor_q)) return 0.0;
    if (cor_p == cor_q) {
        if (cima_e_baixo(tipo_p, tipo_q)) return -PONTO_MESMA_COR;
        return e_roupa(tipo_p) != e_roupa(tipo_q) ? PONTO_MESMA_COR : 0.0;
    }
    return cores.lista_combina(cor_p, cor_q) ? PONTO_COMBINA : PONTO_NAO_COMBINA;
}

double regra_atencao(double p, double q) {
    return p * (1.0 - q) + q * (1.0 - p) - p * q;
}

VersaoE::VersaoE(const Base& base, const std::vector<Atrib>& attr, const std::vector<AtribE>& attr_e,
                 const TabelaCores& cores, const GrafoLooks& grafo)
    : base_(base), attr_(attr), attr_e_(attr_e), cores_(cores), grafo_(grafo) {
    auto ini = std::chrono::steady_clock::now();
    for (int i = 0; i < (int)base_.pecas.size(); i++) {
        const std::string& cat = base_.pecas[i].categoria;
        std::vector<std::vector<Peca>>& por_cor = pecas_[cat];
        por_cor.resize(cores_.total());
        Peca p;
        p.id = i;
        p.v = attr_[i].g1;
        por_cor[attr_e_[i].cor].push_back(p);
        da_categoria_[cat].push_back(i);
    }
    for (const auto& par : pecas_) {
        std::vector<KDTree>& lista = arvores_[par.first];
        for (const std::vector<Peca>& pecas : par.second) {
            lista.emplace_back(pecas.empty() ? 1 : (int)pecas[0].v.size());
            if (!pecas.empty()) lista.back().construir(pecas);
        }
    }
    ms_arvores_ = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - ini).count();
    grupos_.montar(base_, attr_);
}

std::vector<int> VersaoE::candidatas(const std::vector<int>& look, const std::string& categoria,
                                     const ConfigE& cfg) const {
    auto it = pecas_.find(categoria);
    if (it == pecas_.end()) return {};

    Tipo tipo_cat = tipo_da_categoria(categoria);
    auto liberada = [&](int p, int cor) {
        if (!cfg.filtrar_cor) return true;
        int cor_p = attr_e_[p].cor;
        if (!cores_.cromatica(cor_p) || !cores_.cromatica(cor)) return true;
        if (cor_p == cor) return !cima_e_baixo(tipo_da_categoria(base_.pecas[p].categoria), tipo_cat);
        return cores_.lista_combina(cor_p, cor);
    };

    std::unordered_set<int> cand;
    if (cfg.categoria_inteira) {
        for (int q : da_categoria_.at(categoria)) {
            for (int p : look) {
                if (base_.pecas[p].categoria != categoria && liberada(p, attr_e_[q].cor)) {
                    cand.insert(q);
                    break;
                }
            }
        }
    } else {
        const std::vector<KDTree>& arvores = arvores_.at(categoria);
        for (int p : look) {
            if (base_.pecas[p].categoria == categoria) continue;
            for (int cor = 0; cor < (int)arvores.size(); cor++) {
                const std::vector<Peca>& pecas = it->second[cor];
                if (pecas.empty() || !liberada(p, cor)) continue;
                for (const Vizinho& v : arvores[cor].knn(attr_[p].g1, cfg.k)) cand.insert(pecas[v.indice].id);
            }
        }
    }
    for (int p : look) cand.erase(p);
    std::vector<int> res(cand.begin(), cand.end());
    std::sort(res.begin(), res.end());
    return res;
}

std::vector<Partes> VersaoE::partes(const std::vector<int>& look, const std::string& categoria,
                                    const ConfigE& cfg) const {
    std::vector<int> cand = candidatas(look, categoria, cfg);

    std::vector<int> partida;
    std::vector<std::unordered_map<int, double>> bfs;
    for (int p : look) {
        if (base_.pecas[p].categoria != categoria) {
            partida.push_back(p);
            bfs.push_back(grafo_.bfs_2_passos(p, true));
        }
    }

    Tipo tipo_q = tipo_da_categoria(categoria);
    std::vector<Partes> res(cand.size());
    for (int i = 0; i < (int)cand.size(); i++) {
        int q = cand[i];
        res[i].indice = q;
        for (int j = 0; j < (int)partida.size(); j++) {
            int p = partida[j];
            const std::vector<double>& g1 = attr_[p].g1;
            res[i].soma[0] += 1.0 - std::sqrt(distancia2(g1, attr_[q].g1) / g1.size());
            res[i].soma[1] += regra_cor_e(cores_, attr_e_[p].cor, tipo_da_categoria(base_.pecas[p].categoria),
                                          attr_e_[q].cor, tipo_q);
            res[i].soma[2] += regra_atencao(attr_e_[p].atencao, attr_e_[q].atencao);
            auto achou = bfs[j].find(q);
            res[i].soma[3] += achou != bfs[j].end() ? achou->second : grupos_.bonus(p, q);
        }
    }
    return res;
}

std::vector<Candidata> VersaoE::completar(const std::vector<int>& look, const std::string& categoria,
                                          const ConfigE& cfg, int n) const {
    std::vector<Candidata> res;
    for (const Partes& x : partes(look, categoria, cfg)) res.push_back({x.indice, x.nota(cfg.a, cfg.b, cfg.c, cfg.d)});
    ordenar_e_cortar(res, n);
    return res;
}
