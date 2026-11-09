#include "pecas_novas.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <sstream>

#include "grupos_formas.hpp"
#include "kdtree.hpp"

const std::array<std::string, N_FEAT> NOME_FEAT = {
    "C", "popularidade", "cor", "estampa", "formalidade", "estilo", "ocasiao", "comprimento",
    "grupos Kruskal", "grupos forma 3", "atributos / (1 + grau)", "forma 3 / (1 + grau)", "peca nova"};

namespace {

constexpr unsigned SEMENTE_METADES = 2711;
constexpr unsigned SEMENTE_NEGATIVAS = 31;
constexpr int N_NEGATIVAS = 10;
constexpr int PASSOS_GD = 3000;
constexpr double TAXA_GD = 1.0;
constexpr double L2 = 1e-3;
const double GRADE[3] = {0.0, 0.5, 1.0};
const double LISTA_A[5] = {0.5, 1.0, 2.0, 5.0, 10.0};

std::string num(double x, int casas) {
    std::ostringstream o;
    o << std::fixed << std::setprecision(casas) << x;
    std::string s = o.str();
    std::replace(s.begin(), s.end(), '.', ',');
    if (!s.empty() && s[0] == '-') s = "−" + s.substr(1);
    return s;
}

std::string pct(double x, int casas) { return num(100.0 * x, casas) + "%"; }

double segundos(std::chrono::steady_clock::time_point ini) {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - ini).count();
}

void min_max(std::vector<double>& x) { normalizar(x); }

}  // namespace

bool Novas::montar(const Base& b, const Recomendador& rec, const GrafoLooks& grafo, const std::string& pasta_dados,
                   const ConfigF& cfg_f, std::string& erro) {
    if (!ler_valores(pasta_dados + "/pecas_v5.csv", b, val, erro)) return false;
    pa = std::make_unique<PontosAtributos>(b, val);
    pa->peso = cfg_f.peso;
    f = std::make_unique<VersaoF>(b, rec, grafo, *pa);

    std::vector<Atrib> attr;
    if (!ler_attr_grupos(pasta_dados, b, attr, erro)) return false;
    gk = agrupar(montar_grafo_knn(b, attr, K_VIZINHOS_GRUPOS, cfg_f.peso_estampa), cfg_f.k_grupos > 0 ? cfg_f.k_grupos : 100,
                 (int)b.pecas.size());
    if (!grupos_escolhidos(b, pasta_dados, g3, erro) || !grupos_por_cor(b, pasta_dados, g2, erro)) return false;
    // as tabelas so contam looks com treino = true de b
    pk = std::make_unique<PontosGrupos>(b, gk);
    p3 = std::make_unique<PontosGrupos>(b, g3);
    p2 = std::make_unique<PontosGrupos>(b, g2);

    grau.resize(b.pecas.size());
    for (int i = 0; i < (int)b.pecas.size(); i++) grau[i] = grafo.num_looks(i);
    return true;
}

TermoGrupos Novas::termo_forma3(const std::vector<int>& look, const std::vector<int>& cand) const {
    TermoGrupos t;
    if (cand.empty()) return t;
    const int cat = g3.cat_do_grupo[g3.grupo[cand[0]]];
    for (int k = 0; k < cat; k++) t.ini += g3.por_categoria[k];
    t.soma.assign(g3.por_categoria[cat], 0.0);
    // grupo de cor de cada grupo da forma 3: a cor faz parte da chave, entao qualquer peca serve
    std::vector<int> cor(t.soma.size(), -1);
    for (int c : cand) {
        int h = g3.grupo[c] - t.ini;
        if (h >= 0 && h < (int)cor.size()) cor[h] = g2.grupo[c];
    }
    for (int h = 0; h < (int)t.soma.size(); h++) {
        for (int p : look) {
            const int g = g3.grupo[p];
            if (p3->contagem(g, t.ini + h) >= MIN_PARES) {
                t.soma[h] += p3->pontos_entre(g, t.ini + h);
            } else if (cor[h] >= 0 && p2->contagem(g2.grupo[p], cor[h]) >= MIN_PARES) {
                t.soma[h] += p2->pontos_entre(g2.grupo[p], cor[h]);
            }
            // so a categoria: o esperado e igual a contagem, entao o ponto e 0
        }
    }
    return t;
}

namespace {

double do_grupo(const Grupos& gr, const TermoGrupos& t, int c) {
    int h = gr.grupo[c] - t.ini;
    return h >= 0 && h < (int)t.soma.size() ? t.soma[h] : 0.0;
}

}  // namespace

void Novas::colunas(const TermosF& t, const TermoGrupos& tk, const TermoGrupos& t3, const std::vector<int>& cand,
                    std::vector<std::array<double, N_FEAT>>& x) const {
    const int n = (int)cand.size();
    x.assign(n, {});
    std::vector<double> v(n);
    const auto coluna = [&](int k) {
        min_max(v);
        for (int i = 0; i < n; i++) x[i][k] = v[i];
    };
    std::unordered_map<int, double> nc;
    for (const auto& par : t.c) nc[par.first] = par.second;
    for (int i = 0; i < n; i++) {
        auto it = nc.find(cand[i]);
        v[i] = it == nc.end() ? 0.0 : it->second;
    }
    coluna(0);
    for (int i = 0; i < n; i++) v[i] = f->popularidade()[cand[i]];
    coluna(1);
    std::array<double, N_ATRIB> um{};
    for (int a = 0; a < N_ATRIB; a++) {
        um.fill(0.0);
        um[a] = 1.0;
        for (int i = 0; i < n; i++) v[i] = f->atributos(t, cand[i], um);
        coluna(2 + a);
    }
    for (int i = 0; i < n; i++) v[i] = do_grupo(gk, tk, cand[i]);
    coluna(8);
    for (int i = 0; i < n; i++) v[i] = do_grupo(g3, t3, cand[i]);
    coluna(9);
    um.fill(1.0);
    for (int i = 0; i < n; i++) v[i] = f->atributos(t, cand[i], um);
    min_max(v);
    for (int i = 0; i < n; i++) x[i][10] = v[i] / (1.0 + grau[cand[i]]);
    for (int i = 0; i < n; i++) x[i][11] = x[i][9] / (1.0 + grau[cand[i]]);
    for (int i = 0; i < n; i++) x[i][12] = grau[cand[i]] == 0.0 ? 1.0 : 0.0;
}

void Novas::notas(const TermosF& t, const TermoGrupos& tk, const TermoGrupos& t3, const std::vector<int>& cand,
                  const ConfigN& cfg, std::vector<double>& nota) const {
    const int n = (int)cand.size();
    nota.assign(n, 0.0);
    if (cfg.logistica) {
        std::vector<std::array<double, N_FEAT>> x;
        colunas(t, tk, t3, cand, x);
        for (int i = 0; i < n; i++) {
            for (int k = 0; k < N_FEAT; k++) nota[i] += cfg.beta[k] * x[i][k];
        }
        return;
    }
    std::vector<double> v(n);
    std::vector<double> fator(n, 1.0);
    if (cfg.a > 0.0) {
        for (int i = 0; i < n; i++) fator[i] = cfg.a / (1.0 + grau[cand[i]]);
    }
    if (cfg.w[T_C] != 0.0) {
        std::unordered_map<int, double> nc;
        for (const auto& par : t.c) nc[par.first] = par.second;
        for (int i = 0; i < n; i++) {
            auto it = nc.find(cand[i]);
            v[i] = it == nc.end() ? 0.0 : it->second;
        }
        min_max(v);
        for (int i = 0; i < n; i++) nota[i] += cfg.w[T_C] * v[i];
    }
    if (cfg.w[T_ATRIB] != 0.0) {
        for (int i = 0; i < n; i++) v[i] = f->atributos(t, cand[i], cfg.peso);
        min_max(v);
        for (int i = 0; i < n; i++) nota[i] += fator[i] * cfg.w[T_ATRIB] * v[i];
    }
    if (cfg.w[T_GRUPOS] != 0.0 && cfg.grupos != 0) {
        for (int i = 0; i < n; i++) v[i] = cfg.grupos == 1 ? do_grupo(gk, tk, cand[i]) : do_grupo(g3, t3, cand[i]);
        min_max(v);
        for (int i = 0; i < n; i++) nota[i] += fator[i] * cfg.w[T_GRUPOS] * v[i];
    }
    if (cfg.w[T_POP] != 0.0) {
        for (int i = 0; i < n; i++) v[i] = f->popularidade()[cand[i]];
        min_max(v);
        for (int i = 0; i < n; i++) nota[i] += cfg.w[T_POP] * v[i];
    }
}

Notas Novas::notas(const ConfigN& cfg) const {
    return [this, cfg](int, const std::vector<int>& look, const std::vector<int>& cand, std::vector<double>& nota) {
        const TermosF t = f->termos(look, cand);
        const TermoGrupos tk = VersaoF::termo_grupos(gk, *pk, look, cand);
        const TermoGrupos t3 = termo_forma3(look, cand);
        notas(t, tk, t3, cand, cfg, nota);
    };
}

namespace {

std::string linha_config(const ConfigN& c) {
    std::ostringstream o;
    o << std::setprecision(10) << c.nome << ',' << c.grupos << ',' << c.a << ',' << (c.logistica ? 1 : 0);
    for (double w : c.w) o << ',' << w;
    for (double p : c.peso) o << ',' << p;
    for (double b : c.beta) o << ',' << b;
    return o.str();
}

}  // namespace

bool ler_config_novas(const std::string& caminho, std::vector<ConfigN>& cfgs) {
    std::ifstream arq(caminho);
    std::string linha;
    if (!std::getline(arq, linha)) return false;
    cfgs.clear();
    while (std::getline(arq, linha)) {
        if (linha.empty()) continue;
        std::stringstream s(linha);
        std::string campo;
        std::vector<std::string> col;
        while (std::getline(s, campo, ',')) col.push_back(campo);
        if ((int)col.size() != 4 + N_TERMOS + N_ATRIB + N_FEAT) return false;
        ConfigN c;
        c.nome = col[0];
        c.grupos = std::stoi(col[1]);
        c.a = std::stod(col[2]);
        c.logistica = col[3] == "1";
        int k = 4;
        for (double& w : c.w) w = std::stod(col[k++]);
        for (double& p : c.peso) p = std::stod(col[k++]);
        for (double& b : c.beta) b = std::stod(col[k++]);
        cfgs.push_back(c);
    }
    return true;
}

namespace {

struct Metade {
    std::array<int, 3> acertos{};  // @1, @5, @10 somados nas 10 ordens
    double fitb = 0.0;
};

struct Rodada {
    ConfigN cfg;
    std::string variacao;
    std::array<Metade, 2> m;
    Posicoes pos;
};

const int KS[3] = {1, 5, 10};

std::array<Metade, 2> resumir(const Posicoes& p, const std::vector<int>& metade) {
    std::array<Metade, 2> r{};
    std::array<int, 2> nf{};
    for (int q = 0; q < (int)p.pos.size(); q++) {
        Metade& m = r[metade[q]];
        for (int s = 0; s < N_ORDENS; s++) {
            for (int k = 0; k < 3; k++) m.acertos[k] += p.pos[q][s] < KS[k];
        }
        if (p.fitb[q][0] >= 0) {
            double f = 0.0;
            for (int s = 0; s < N_ORDENS; s++) f += p.fitb[q][s];
            m.fitb += f / N_ORDENS;
            nf[metade[q]]++;
        }
    }
    for (int h = 0; h < 2; h++) r[h].fitb = nf[h] > 0 ? r[h].fitb / nf[h] : 0.0;
    return r;
}

int zeros(const ConfigN& c) {
    int z = 0;
    for (double w : c.w) z += w == 0.0;
    return z;
}

// so a metade A decide. Hit@10, Hit@5, mais zeros, a menor
bool melhor_hit(const Rodada& x, const Rodada& y) {
    const Metade& a = x.m[0];
    const Metade& b = y.m[0];
    if (a.acertos[2] != b.acertos[2]) return a.acertos[2] > b.acertos[2];
    if (a.acertos[1] != b.acertos[1]) return a.acertos[1] > b.acertos[1];
    if (zeros(x.cfg) != zeros(y.cfg)) return zeros(x.cfg) > zeros(y.cfg);
    return x.cfg.a < y.cfg.a;
}

bool melhor_fitb(const Rodada& x, const Rodada& y) {
    if (x.m[0].fitb != y.m[0].fitb) return x.m[0].fitb > y.m[0].fitb;
    return melhor_hit(x, y);
}

std::vector<double> so_metade(const std::vector<double>& v, const std::vector<int>& metade, int h) {
    std::vector<double> r;
    for (int q = 0; q < (int)v.size(); q++) {
        if (metade[q] == h && v[q] >= 0) r.push_back(v[q]);
    }
    return r;
}

std::string descrever(const ConfigN& c) {
    if (c.logistica) return "regressão logística (13 colunas)";
    std::ostringstream o;
    o << "w = (" << num(c.w[0], 1) << "; " << num(c.w[1], 1) << "; " << num(c.w[2], 1) << "; " << num(c.w[3], 1) << ")";
    o << (c.grupos == 1 ? ", grupos Kruskal" : (c.grupos == 2 ? ", grupos forma 3" : ""));
    if (c.a > 0.0) o << ", a = " << num(c.a, 1);
    return o.str();
}

// regressao logistica com descida de gradiente em lote, escrita aqui
std::array<double, N_FEAT> treinar(const std::vector<std::array<double, N_FEAT>>& x, const std::vector<int>& y,
                                   double& perda_ini, double& perda_fim) {
    std::array<double, N_FEAT> beta{};
    double vies = 0.0;
    const int n = (int)x.size();
    const auto perda = [&]() {
        double s = 0.0;
        for (int i = 0; i < n; i++) {
            double z = vies;
            for (int k = 0; k < N_FEAT; k++) z += beta[k] * x[i][k];
            // log(1 + e^z) sem estourar
            double l = z > 0 ? z + std::log1p(std::exp(-z)) : std::log1p(std::exp(z));
            s += l - y[i] * z;
        }
        return s / std::max(n, 1);
    };
    perda_ini = perda();
    for (int passo = 0; passo < PASSOS_GD; passo++) {
        std::array<double, N_FEAT> g{};
        double gv = 0.0;
        for (int i = 0; i < n; i++) {
            double z = vies;
            for (int k = 0; k < N_FEAT; k++) z += beta[k] * x[i][k];
            double e = 1.0 / (1.0 + std::exp(-z)) - y[i];
            for (int k = 0; k < N_FEAT; k++) g[k] += e * x[i][k];
            gv += e;
        }
        for (int k = 0; k < N_FEAT; k++) beta[k] -= TAXA_GD * (g[k] / n + L2 * beta[k]);
        vies -= TAXA_GD * gv / n;
    }
    perda_fim = perda();
    return beta;
}

}  // namespace

bool ajustar_novas(const Base& base, const std::string& pasta_dados, const std::string& pasta_saida) {
    ConfigF cfg_f, c_pop;
    if (!ler_config_f(pasta_saida + "/config_f.csv", cfg_f, c_pop)) {
        std::cerr << "Sem " << pasta_saida << "/config_f.csv (rode ./recomendador ajustar_f).\n";
        return false;
    }
    const auto ini = std::chrono::steady_clock::now();

    // tudo com a base sem os looks de validacao. o teste nao e montado
    const Base b = base_sem_validacao(base);
    const Consultas c = consultas_validacao(base);
    const int nq = (int)c.alvo.size();
    KDTree arvore((int)b.pecas[0].v.size());
    arvore.construir(b.pecas);
    GrafoLooks grafo;
    grafo.construir(b);
    Recomendador rec(b, arvore, grafo);
    Novas nv;
    std::string erro;
    if (!nv.montar(b, rec, grafo, pasta_dados, cfg_f, erro)) {
        std::cerr << erro << '\n';
        return false;
    }

    std::vector<TermosF> t(nq);
    std::vector<TermoGrupos> tk(nq), t3(nq);
    for (int q = 0; q < nq; q++) {
        const std::vector<int> cand = candidatas(b, c, q);
        t[q] = nv.f->termos(c.look[q], cand);
        tk[q] = VersaoF::termo_grupos(nv.gk, *nv.pk, c.look[q], cand);
        t3[q] = nv.termo_forma3(c.look[q], cand);
    }
    std::cout << "Termos de " << nq << " consultas (" << segundos(ini) << " s)" << std::endl;

    // metades A (0) e B (1), sorteadas uma vez
    std::vector<int> ordem(nq);
    std::iota(ordem.begin(), ordem.end(), 0);
    std::mt19937 ger(SEMENTE_METADES);
    std::shuffle(ordem.begin(), ordem.end(), ger);
    std::vector<int> metade(nq, 1);
    for (int i = 0; i < nq / 2; i++) metade[ordem[i]] = 0;

    // peca escondida conhecida = tem look de treino em b
    int novas_val = 0;
    for (int q = 0; q < nq; q++) novas_val += nv.grau[c.alvo[q]] == 0.0;

    std::vector<Rodada> todas;
    const auto rodar = [&](const std::string& variacao, const ConfigN& cfg) -> const Rodada& {
        Rodada l;
        l.cfg = cfg;
        l.variacao = variacao;
        l.pos = avaliar_notas(
            b, c, cfg.nome,
            [&](int q, const std::vector<int>&, const std::vector<int>& cand, std::vector<double>& nota) {
                nv.notas(t[q], tk[q], t3[q], cand, cfg, nota);
            },
            &nv.f->popularidade());
        l.m = resumir(l.pos, metade);
        todas.push_back(std::move(l));
        return todas.back();
    };

    ConfigN base_f;
    base_f.nome = "F";
    base_f.w = cfg_f.w;
    base_f.peso = cfg_f.peso;
    base_f.grupos = cfg_f.w[T_GRUPOS] != 0.0 ? 1 : 0;
    const Rodada ref = rodar("F atual", base_f);
    std::cout << "F: Hit@10 A " << ref.m[0].acertos[2] << ", B " << ref.m[1].acertos[2] << " (" << segundos(ini) << " s)"
              << std::endl;

    // 1. forma 3 com recuo no lugar do Kruskal
    std::vector<std::pair<std::string, int>> melhores;  // variacao e indice em todas
    const auto grade = [&](const std::string& variacao, int grupos, const std::vector<double>& lista_a) {
        int melhor_i = -1;
        for (double a : lista_a) {
            for (double w1 : GRADE) for (double w2 : GRADE) for (double w3 : GRADE) for (double w4 : GRADE) {
                if (w1 + w2 + w3 + w4 == 0.0) continue;
                if (grupos != 0 && w3 == 0.0) continue;  // sem grupos e a mesma lista em todas as variacoes
                if (a > 0.0 && w2 + w3 == 0.0) continue;  // o fator nao muda nada
                ConfigN cfg;
                cfg.nome = variacao;
                cfg.w = {w1, w2, w3, w4};
                cfg.peso = cfg_f.peso;
                cfg.grupos = grupos;
                cfg.a = a;
                rodar(variacao, cfg);
                int i = (int)todas.size() - 1;
                if (melhor_i < 0 || melhor_hit(todas[i], todas[melhor_i])) melhor_i = i;
            }
        }
        melhores.push_back({variacao, melhor_i});
        std::cout << variacao << ": " << descrever(todas[melhor_i].cfg) << ", Hit@10 A " << todas[melhor_i].m[0].acertos[2]
                  << " (" << segundos(ini) << " s)" << std::endl;
    };
    grade("forma 3 com recuo", 2, {0.0});
    grade("peso por grau (Kruskal)", 1, {0.5, 1.0, 2.0, 5.0, 10.0});
    grade("forma 3 + peso por grau", 2, {0.5, 1.0, 2.0, 5.0, 10.0});

    // 3. regressao logistica: pares da metade A, o certo e 10 sorteados da mesma categoria
    std::vector<std::array<double, N_FEAT>> xs;
    std::vector<int> ys;
    std::mt19937 ger_neg(SEMENTE_NEGATIVAS);
    for (int q = 0; q < nq; q++) {
        if (metade[q] != 0) continue;
        const std::vector<int> cand = candidatas(b, c, q);
        if (cand.size() < 2) continue;
        std::vector<std::array<double, N_FEAT>> x;
        nv.colunas(t[q], tk[q], t3[q], cand, x);
        int certo = (int)(std::find(cand.begin(), cand.end(), c.alvo[q]) - cand.begin());
        xs.push_back(x[certo]);
        ys.push_back(1);
        std::uniform_int_distribution<int> d(0, (int)cand.size() - 1);
        for (int k = 0; k < N_NEGATIVAS; k++) {
            int i = d(ger_neg);
            if (i == certo) continue;
            xs.push_back(x[i]);
            ys.push_back(0);
        }
    }
    double perda_ini = 0.0, perda_fim = 0.0;
    ConfigN log;
    log.nome = "logistica";
    log.logistica = true;
    log.beta = treinar(xs, ys, perda_ini, perda_fim);
    rodar("regressão logística", log);
    melhores.push_back({"regressão logística", (int)todas.size() - 1});
    std::cout << "Logistica: perda " << perda_ini << " -> " << perda_fim << ", Hit@10 A " << todas.back().m[0].acertos[2]
              << " (" << segundos(ini) << " s)" << std::endl;

    // juntas: a melhor de tudo na metade A, pelo Hit@10 e pelo FITB
    int i_hit = 1, i_fitb = 1;
    for (int i = 1; i < (int)todas.size(); i++) {
        if (melhor_hit(todas[i], todas[i_hit])) i_hit = i;
        if (melhor_fitb(todas[i], todas[i_fitb])) i_fitb = i;
    }
    const auto aceita_hit = [&](const Rodada& l) {
        return l.m[0].acertos[2] > ref.m[0].acertos[2] && l.m[1].acertos[2] > ref.m[1].acertos[2];
    };
    const auto aceita_fitb = [&](const Rodada& l) { return l.m[0].fitb > ref.m[0].fitb && l.m[1].fitb > ref.m[1].fitb; };

    // grade e config
    std::filesystem::create_directories(pasta_saida);
    std::ofstream csv(pasta_saida + "/novas_grade.csv");
    csv << "variacao,grupos,a,w1,w2,w3,w4,hit1_A,hit5_A,hit10_A,fitb_A,hit1_B,hit5_B,hit10_B,fitb_B\n";
    for (const Rodada& l : todas) {
        csv << l.variacao << ',' << l.cfg.grupos << ',' << l.cfg.a;
        for (double w : l.cfg.w) csv << ',' << w;
        for (int h = 0; h < 2; h++) {
            for (int k = 0; k < 3; k++) csv << ',' << l.m[h].acertos[k];
            csv << ',' << l.m[h].fitb;
        }
        csv << '\n';
    }
    std::ofstream conf(pasta_saida + "/config_novas.csv");
    conf << "nome,grupos,a,logistica,w1,w2,w3,w4";
    for (int a = 0; a < N_ATRIB; a++) conf << ",peso_" << NOME_ATRIB[a];
    for (int k = 0; k < N_FEAT; k++) conf << ",beta" << k;
    conf << '\n';
    ConfigN ch = todas[i_hit].cfg, cf = todas[i_fitb].cfg;
    ch.nome = "F2 (Hit@10)";
    cf.nome = "F2 (FITB)";
    if (aceita_hit(todas[i_hit])) conf << linha_config(ch) << '\n';
    if (aceita_fitb(todas[i_fitb])) conf << linha_config(cf) << '\n';

    // relatorio
    const int na = (int)std::count(metade.begin(), metade.end(), 0);
    const int nb = nq - na;
    const auto hit = [&](const Metade& m, int k, int n) { return pct((double)m.acertos[k] / (N_ORDENS * n), 3); };
    const auto ic = [&](const Rodada& l, int k) {
        Intervalo i = bootstrap(so_metade(por_consulta(l.pos, k), metade, 1), so_metade(por_consulta(ref.pos, k), metade, 1));
        std::string s = num(100 * i.dif, k == 0 ? 2 : 3) + " pp (" + num(100 * i.min, k == 0 ? 2 : 3) + " a " +
                        num(100 * i.max, k == 0 ? 2 : 3) + ")";
        return s + (i.min > 0 || i.max < 0 ? " real" : (i.dif == 0 ? " empate" : " pode ser sorte"));
    };
    std::ofstream md(pasta_saida + "/novas_validacao.md");
    md << "# Variações para a peça nova (validação)\n\n"
       << "Gerado por `./recomendador ajustar_novas`. O teste não é montado.\n\n"
       << "- Consultas de validação: " << nq << ". Metade A: " << na << ". Metade B: " << nb
       << " (sorteio com semente " << SEMENTE_METADES << ").\n"
       << "- Escondidas sem nenhum look de treino (peça nova): " << novas_val << " de " << nq << " ("
       << pct((double)novas_val / nq, 1) << ").\n"
       << "- A metade A escolhe. A metade B só confirma: uma variação fica se ganhar da F atual nas duas metades.\n"
       << "- Hit@k é a média das 10 ordens de desempate. Os intervalos são de bootstrap na metade B (diferença para a F atual).\n"
       << "- Todas as notas desempatam pela popularidade, como a F.\n\n"
       << "## Melhor de cada variação (escolhida na metade A)\n\n"
       << "| Variação | Configuração | Hit@10 A | Hit@10 B | FITB A | FITB B | Hit@10 B x F | FITB B x F |\n"
       << "| --- | --- | --- | --- | --- | --- | --- | --- |\n";
    const auto linha_md = [&](const std::string& nome, const Rodada& l) {
        md << "| " << nome << " | " << descrever(l.cfg) << " | " << hit(l.m[0], 2, na) << " | " << hit(l.m[1], 2, nb) << " | "
           << pct(l.m[0].fitb, 2) << " | " << pct(l.m[1].fitb, 2) << " | " << (&l == &ref ? "" : ic(l, 10)) << " | "
           << (&l == &ref ? "" : ic(l, 0)) << " |\n";
    };
    linha_md("F atual", ref);
    for (const auto& m : melhores) linha_md(m.first, todas[m.second]);
    md << "\nTotal de configurações testadas: " << todas.size() << ". Todas estão em `novas_grade.csv`.\n\n"
       << "## Escolhas\n\n"
       << "- Melhor pelo Hit@10 na metade A: " << todas[i_hit].variacao << ", " << descrever(todas[i_hit].cfg) << ". "
       << (aceita_hit(todas[i_hit]) ? "Ganha da F nas duas metades e vai ao teste." : "Não ganha da F nas duas metades e não vai ao teste.")
       << "\n- Melhor pelo FITB na metade A (critério acrescentado depois): " << todas[i_fitb].variacao << ", "
       << descrever(todas[i_fitb].cfg) << ". "
       << (aceita_fitb(todas[i_fitb]) ? "Ganha da F no FITB nas duas metades e vai ao teste." : "Não ganha da F no FITB nas duas metades e não vai ao teste.")
       << "\n\n## Regressão logística\n\n"
       << "Pares da metade A: " << xs.size() << " (" << std::count(ys.begin(), ys.end(), 1) << " certos). "
       << PASSOS_GD << " passos de descida de gradiente em lote, taxa " << num(TAXA_GD, 1) << ", L2 " << L2
       << ". Perda média de " << num(perda_ini, 4) << " para " << num(perda_fim, 4) << ".\n\n"
       << "| Coluna | Peso aprendido |\n| --- | --- |\n";
    for (int k = 0; k < N_FEAT; k++) md << "| " << NOME_FEAT[k] << " | " << num(log.beta[k], 3) << " |\n";
    std::cout << "Pronto em " << segundos(ini) << " s. Veja " << pasta_saida << "/novas_validacao.md" << std::endl;
    return true;
}
