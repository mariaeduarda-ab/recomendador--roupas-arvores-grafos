#include "versao_f.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "atributos.hpp"
#include "kdtree.hpp"

const std::array<std::string, N_TERMOS> NOME_TERMO = {"C", "pontos por atributo", "grupos", "popularidade"};

namespace {

const std::string CABECALHO_CONFIG =
    "versao,w_c,w_atributos,w_grupos,w_popularidade,cor,estampa,formalidade,estilo,ocasiao,comprimento,"
    "k_grupos,peso_estampa";

std::string linha_config(const std::string& nome, const ConfigF& f) {
    std::ostringstream o;
    o << nome;
    for (double w : f.w) o << ',' << w;
    for (double p : f.peso) o << ',' << p;
    o << ',' << f.k_grupos << ',' << f.peso_estampa;
    return o.str();
}

bool ler_linha_config(const std::string& linha, std::string& nome, ConfigF& f) {
    std::vector<std::string> campos;
    std::stringstream s(linha);
    std::string campo;
    while (std::getline(s, campo, ',')) campos.push_back(campo);
    if (campos.size() != 1 + N_TERMOS + N_ATRIB + 2) return false;
    try {
        nome = campos[0];
        for (int i = 0; i < N_TERMOS; i++) f.w[i] = std::stod(campos[1 + i]);
        for (int a = 0; a < N_ATRIB; a++) f.peso[a] = std::stod(campos[1 + N_TERMOS + a]);
        f.k_grupos = std::stoi(campos[1 + N_TERMOS + N_ATRIB]);
        f.peso_estampa = std::stod(campos[2 + N_TERMOS + N_ATRIB]);
    } catch (const std::exception&) {
        return false;
    }
    return true;
}

}  // namespace

bool ler_config_f(const std::string& caminho, ConfigF& f, ConfigF& c_pop) {
    std::ifstream arq(caminho);
    std::string linha;
    if (!arq || !std::getline(arq, linha)) return false;
    bool tem_f = false, tem_c = false;
    while (std::getline(arq, linha)) {
        if (!linha.empty() && linha.back() == '\r') linha.pop_back();
        if (linha.empty()) continue;
        std::string nome;
        ConfigF x;
        if (!ler_linha_config(linha, nome, x)) return false;
        if (nome == "F") {
            f = x;
            tem_f = true;
        } else if (nome == "C + popularidade") {
            c_pop = x;
            tem_c = true;
        }
    }
    return tem_f && tem_c;
}

bool gravar_config_f(const std::string& caminho, const ConfigF& f, const ConfigF& c_pop) {
    std::ofstream arq(caminho);
    arq << CABECALHO_CONFIG << '\n' << linha_config("F", f) << '\n' << linha_config("C + popularidade", c_pop) << '\n';
    return (bool)arq;
}

void normalizar(std::vector<double>& x) {
    if (x.empty()) return;
    auto mm = std::minmax_element(x.begin(), x.end());
    double menor = *mm.first;
    double faixa = *mm.second - menor;
    // todas iguais: o termo nao separa ninguem, vale 0 pra todas
    if (faixa <= 0) {
        std::fill(x.begin(), x.end(), 0.0);
        return;
    }
    for (double& v : x) v = (v - menor) / faixa;
}

VersaoF::VersaoF(const Base& base, const Recomendador& rec, const GrafoLooks& grafo, const PontosAtributos& pa)
    : base_(base), rec_(rec), pa_(pa) {
    pop_.resize(base.pecas.size());
    for (int i = 0; i < (int)base.pecas.size(); i++) pop_[i] = std::log(1.0 + grafo.num_looks(i));
    nota_c_.assign(base.pecas.size(), 0.0);
}

TermosF VersaoF::termos(const std::vector<int>& look, const std::vector<int>& cand) const {
    TermosF t;
    if (cand.empty()) return t;
    const int c0 = cand[0];
    for (const Candidata& x : rec_.versao_c(look, base_.pecas[c0].categoria, (int)base_.pecas.size())) {
        t.c.push_back({x.indice, x.pontuacao});
    }
    // a tabela depende so das categorias, e todas as candidatas tem a categoria de c0
    const ValoresAtrib& val = pa_.valores();
    for (int a = 0; a < N_ATRIB; a++) {
        const int nv = (int)val.nomes[a].size();
        t.por_valor[a].assign(nv, 0.0);
        for (int p : look) {
            const Tabela* tab = pa_.tabela(a, p, c0);
            const std::vector<int>& vp = val.val[a][p];
            if (tab == nullptr || vp.empty()) continue;
            for (int y = 0; y < nv; y++) {
                double soma = 0.0;
                for (int x : vp) soma += pontos_tabela(*tab, nv, x, y);
                t.por_valor[a][y] += soma / vp.size();
            }
        }
    }
    return t;
}

double VersaoF::atributos(const TermosF& t, int c, const std::array<double, N_ATRIB>& peso) const {
    const ValoresAtrib& val = pa_.valores();
    double soma = 0.0;
    for (int a = 0; a < N_ATRIB; a++) {
        const std::vector<int>& vc = val.val[a][c];
        if (peso[a] == 0.0 || vc.empty() || t.por_valor[a].empty()) continue;
        // media nos valores marcados, igual ao pontos_de
        double m = 0.0;
        for (int y : vc) m += t.por_valor[a][y];
        soma += peso[a] * m / vc.size();
    }
    return soma;
}

TermoGrupos VersaoF::termo_grupos(const Grupos& gr, const PontosGrupos& pg, const std::vector<int>& look,
                                  const std::vector<int>& cand) {
    TermoGrupos t;
    if (cand.empty()) return t;
    const int cat = gr.cat_do_grupo[gr.grupo[cand[0]]];
    for (int k = 0; k < cat; k++) t.ini += gr.por_categoria[k];
    t.soma.assign(gr.por_categoria[cat], 0.0);
    for (int h = 0; h < (int)t.soma.size(); h++) {
        for (int p : look) t.soma[h] += pg.pontos_entre(gr.grupo[p], t.ini + h);
    }
    return t;
}

void VersaoF::notas(const TermosF& t, const TermoGrupos* tg, const Grupos* gr, const std::vector<int>& cand,
                    const ConfigF& cfg, std::vector<double>& nota) const {
    const int n = (int)cand.size();
    nota.assign(n, 0.0);
    std::vector<double> x(n);
    const auto somar = [&](double w) {
        normalizar(x);
        for (int i = 0; i < n; i++) nota[i] += w * x[i];
    };
    // peso 0 nem calcula o termo
    if (cfg.w[T_C] != 0.0) {
        for (const auto& par : t.c) nota_c_[par.first] = par.second;
        for (int i = 0; i < n; i++) x[i] = nota_c_[cand[i]];
        for (const auto& par : t.c) nota_c_[par.first] = 0.0;
        somar(cfg.w[T_C]);
    }
    if (cfg.w[T_ATRIB] != 0.0) {
        for (int i = 0; i < n; i++) x[i] = atributos(t, cand[i], cfg.peso);
        somar(cfg.w[T_ATRIB]);
    }
    if (cfg.w[T_GRUPOS] != 0.0 && tg != nullptr && gr != nullptr) {
        for (int i = 0; i < n; i++) {
            int h = gr->grupo[cand[i]] - tg->ini;
            x[i] = h >= 0 && h < (int)tg->soma.size() ? tg->soma[h] : 0.0;
        }
        somar(cfg.w[T_GRUPOS]);
    }
    if (cfg.w[T_POP] != 0.0) {
        for (int i = 0; i < n; i++) x[i] = pop_[cand[i]];
        somar(cfg.w[T_POP]);
    }
}

bool ler_attr_grupos(const std::string& pasta_dados, const Base& base, std::vector<Atrib>& attr, std::string& erro) {
    const std::string v4 = pasta_dados + "/pecas_v4.csv";
    return std::filesystem::exists(v4) ? ler_atributos(v4, base, attr, erro)
                                       : ler_atributos_da_pasta(pasta_dados, base, attr, erro);
}

bool VersoesF::montar(const Base& b, const Recomendador& rec, const GrafoLooks& grafo, const std::string& pasta_dados,
                      const std::string& arquivo_config, std::string& erro) {
    if (!ler_config_f(arquivo_config, f, c_pop)) {
        erro = "sem " + arquivo_config + " (rode ./recomendador ajustar_f)";
        return false;
    }
    if (!ler_valores(pasta_dados + "/pecas_v5.csv", b, val, erro)) return false;
    pa = std::make_unique<PontosAtributos>(b, val);
    pa->peso = f.peso;
    versao = std::make_unique<VersaoF>(b, rec, grafo, *pa);
    if (f.w[T_GRUPOS] != 0.0) {
        std::vector<Atrib> attr;
        if (!ler_attr_grupos(pasta_dados, b, attr, erro)) return false;
        gr = agrupar(montar_grafo_knn(b, attr, K_VIZINHOS_GRUPOS, f.peso_estampa), f.k_grupos, (int)b.pecas.size());
        pg = std::make_unique<PontosGrupos>(b, gr);
    }
    return true;
}

Notas VersoesF::notas(const ConfigF& cfg) const {
    return [this, cfg](int, const std::vector<int>& look, const std::vector<int>& cand, std::vector<double>& nota) {
        const TermosF t = versao->termos(look, cand);
        if (cfg.w[T_GRUPOS] != 0.0 && pg) {
            const TermoGrupos tg = VersaoF::termo_grupos(gr, *pg, look, cand);
            versao->notas(t, &tg, &gr, cand, cfg, nota);
        } else {
            versao->notas(t, nullptr, nullptr, cand, cfg, nota);
        }
    };
}

namespace {

const double GRADE[3] = {0.0, 0.5, 1.0};
const int LISTA_K[3] = {20, 50, 100};
const double LISTA_PE[3] = {0.0, 0.1, 0.2};
const int METRICAS[4] = {1, 5, 10, 0};

struct Resultado {
    std::string etapa;
    ConfigF cfg;
    std::array<int, 3> acertos{};   // Hit@1, @5, @10 somados nas 10 ordens. inteiro pra comparar sem erro de conta
    std::array<double, 4> media{};  // Hit@1, @5, @10 e FITB, media das 10 ordens
};

Resultado resumir(const std::string& etapa, const ConfigF& cfg, const Posicoes& p) {
    Resultado r;
    r.etapa = etapa;
    r.cfg = cfg;
    for (int m = 0; m < 4; m++) {
        for (int s = 0; s < N_ORDENS; s++) r.media[m] += taxa(p, METRICAS[m], s) / N_ORDENS;
    }
    for (int m = 0; m < 3; m++) {
        for (const auto& pos : p.pos) {
            for (int s = 0; s < N_ORDENS; s++) r.acertos[m] += pos[s] < METRICAS[m];
        }
    }
    return r;
}

// peso de atributo so conta quando o termo 2 esta ligado
int contar(const ConfigF& f, double valor) {
    int n = 0;
    for (double w : f.w) n += w == valor;
    if (f.w[T_ATRIB] != 0.0) {
        for (double p : f.peso) n += p == valor;
    }
    return n;
}

// Hit@10, depois Hit@5, depois mais zeros. (1, 1) e (0,5; 0,5) dao a mesma lista, entao prefere o 1
bool melhor(const Resultado& a, const Resultado& b) {
    if (a.acertos[2] != b.acertos[2]) return a.acertos[2] > b.acertos[2];
    if (a.acertos[1] != b.acertos[1]) return a.acertos[1] > b.acertos[1];
    if (contar(a.cfg, 0.0) != contar(b.cfg, 0.0)) return contar(a.cfg, 0.0) > contar(b.cfg, 0.0);
    return contar(a.cfg, 1.0) > contar(b.cfg, 1.0);
}

std::string num(double x, int casas) {
    std::ostringstream o;
    o << std::fixed << std::setprecision(casas) << x;
    std::string s = o.str();
    std::replace(s.begin(), s.end(), '.', ',');
    if (!s.empty() && s[0] == '-') s = "−" + s.substr(1);
    return s;
}

std::string peso_txt(double w) {
    if (w == 0.0) return "0";
    if (w == 1.0) return "1";
    return num(w, 1);
}

std::string pct(double x, int m) { return num(100.0 * x, METRICAS[m] == 0 ? 2 : 3) + "%"; }

std::string descrever(const ConfigF& f) {
    std::string s = "w = (" + peso_txt(f.w[0]) + "; " + peso_txt(f.w[1]) + "; " + peso_txt(f.w[2]) + "; " +
                    peso_txt(f.w[3]) + ")";
    if (f.w[T_ATRIB] != 0.0) {
        s += ", atributos (";
        for (int a = 0; a < N_ATRIB; a++) s += (a ? "; " : "") + peso_txt(f.peso[a]);
        s += ")";
    }
    if (f.w[T_GRUPOS] != 0.0) s += ", K = " + std::to_string(f.k_grupos) + ", estampa " + peso_txt(f.peso_estampa);
    return s;
}

std::string conclusao(const Intervalo& i) {
    if (i.min > 0) return "ganha (diferença real)";
    if (i.max < 0) return "perde (diferença real)";
    return "pode ser sorte";
}

double segundos(std::chrono::steady_clock::time_point ini) {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - ini).count();
}

}  // namespace

bool ajustar_f(const Base& base, const std::string& pasta_dados, const std::string& pasta_saida) {
    ValoresAtrib val;
    std::vector<Atrib> attr;
    std::string erro;
    if (!ler_valores(pasta_dados + "/pecas_v5.csv", base, val, erro) || !ler_attr_grupos(pasta_dados, base, attr, erro)) {
        std::cerr << erro << '\n';
        return false;
    }
    std::filesystem::create_directories(pasta_saida);
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
    PontosAtributos pa(b, val);
    VersaoF f(b, rec, grafo, pa);

    std::vector<TermosF> termos(nq);
    for (int q = 0; q < nq; q++) termos[q] = f.termos(c.look[q], candidatas(b, c, q));
    std::cout << "Termos de " << nq << " consultas (" << segundos(ini) << " s)" << std::endl;

    struct OpcaoGrupos {
        int k = 0;
        double pe = 0.0;
        Grupos gr;
        std::vector<TermoGrupos> t;
    };
    std::vector<OpcaoGrupos> opcoes;
    for (double pe : LISTA_PE) {
        GrafoKnn g = montar_grafo_knn(b, attr, K_VIZINHOS_GRUPOS, pe);
        for (int k : LISTA_K) {
            OpcaoGrupos o;
            o.k = k;
            o.pe = pe;
            o.gr = agrupar(g, k, (int)b.pecas.size());
            PontosGrupos pg(b, o.gr);
            for (int q = 0; q < nq; q++) o.t.push_back(VersaoF::termo_grupos(o.gr, pg, c.look[q], candidatas(b, c, q)));
            opcoes.push_back(std::move(o));
        }
    }
    std::cout << "Grupos de " << opcoes.size() << " opções (" << segundos(ini) << " s)" << std::endl;

    const auto opcao = [&](const ConfigF& cfg) -> const OpcaoGrupos* {
        if (cfg.w[T_GRUPOS] == 0.0) return nullptr;
        for (const OpcaoGrupos& o : opcoes) {
            if (o.k == cfg.k_grupos && o.pe == cfg.peso_estampa) return &o;
        }
        return nullptr;
    };
    const auto rodar = [&](const ConfigF& cfg, const std::string& nome, bool desempata) {
        const OpcaoGrupos* o = opcao(cfg);
        return avaliar_notas(
            b, c, nome,
            [&](int q, const std::vector<int>&, const std::vector<int>& cand, std::vector<double>& nota) {
                f.notas(termos[q], o ? &o->t[q] : nullptr, o ? &o->gr : nullptr, cand, cfg, nota);
            },
            desempata ? &f.popularidade() : nullptr);
    };
    std::vector<Resultado> grade;
    const auto registrar = [&](const std::string& etapa, const ConfigF& cfg) {
        grade.push_back(resumir(etapa, cfg, rodar(cfg, etapa, true)));
        return grade.back();
    };
    // grade de w1 a w4. so_sem_grupos fixa w3 = 0
    const auto grade_w = [&](const std::string& etapa, ConfigF cfg, bool sem_grupos, bool so_com_grupos,
                             Resultado* atual) {
        Resultado melhor_r;
        bool tem = false;
        if (atual) {
            melhor_r = *atual;
            tem = true;
        }
        for (double w1 : GRADE) {
            for (double w2 : GRADE) {
                for (double w3 : GRADE) {
                    for (double w4 : GRADE) {
                        if (w1 + w2 + w3 + w4 == 0.0) continue;
                        if ((sem_grupos && w3 != 0.0) || (so_com_grupos && w3 == 0.0)) continue;
                        cfg.w = {w1, w2, w3, w4};
                        if (atual && cfg.w == atual->cfg.w) continue;
                        Resultado r = registrar(etapa, cfg);
                        if (!tem || melhor(r, melhor_r)) melhor_r = r;
                        tem = true;
                    }
                }
            }
        }
        return melhor_r;
    };
    // um atributo de cada vez, os outros fixos
    const auto passar_atributos = [&](const std::string& etapa, Resultado atual) {
        if (atual.cfg.w[T_ATRIB] == 0.0) return atual;
        for (int a = 0; a < N_ATRIB; a++) {
            const double antes = atual.cfg.peso[a];
            for (double v : GRADE) {
                if (v == antes) continue;
                ConfigF cfg = atual.cfg;
                cfg.peso[a] = v;
                if (std::all_of(cfg.peso.begin(), cfg.peso.end(), [](double p) { return p == 0.0; })) continue;
                Resultado r = registrar(etapa, cfg);
                if (melhor(r, atual)) atual = r;
            }
        }
        return atual;
    };

    // 1. grade dos w com os pesos de atributo em 1. primeiro sem grupos, depois cada opcao de grupos
    ConfigF inicial;
    inicial.peso.fill(1.0);
    inicial.k_grupos = 0;
    const Resultado sem_grupos = grade_w("1. grade sem grupos", inicial, true, false, nullptr);
    std::cout << "Sem grupos: " << descrever(sem_grupos.cfg) << " (" << segundos(ini) << " s)" << std::endl;
    std::vector<Resultado> melhor_opcao;
    Resultado com_grupos;
    for (std::size_t i = 0; i < opcoes.size(); i++) {
        ConfigF cfg = inicial;
        cfg.k_grupos = opcoes[i].k;
        cfg.peso_estampa = opcoes[i].pe;
        melhor_opcao.push_back(grade_w("1. grade com grupos", cfg, false, true, nullptr));
        if (i == 0 || melhor(melhor_opcao.back(), com_grupos)) com_grupos = melhor_opcao.back();
        std::cout << "K = " << opcoes[i].k << ", estampa " << opcoes[i].pe << ": " << descrever(melhor_opcao.back().cfg)
                  << " (" << segundos(ini) << " s)" << std::endl;
    }
    const bool usa_grupos = melhor(com_grupos, sem_grupos);
    Resultado atual = usa_grupos ? com_grupos : sem_grupos;

    // 2. pesos de atributo, 3. grade dos w de novo, 4. pesos de atributo de novo
    atual = passar_atributos("2. pesos de atributo", atual);
    atual = grade_w("3. grade de novo", atual.cfg, !usa_grupos, false, &atual);
    atual = passar_atributos("4. pesos de atributo de novo", atual);
    ConfigF escolhida = atual.cfg;
    if (escolhida.w[T_GRUPOS] == 0.0) {
        escolhida.k_grupos = 0;
        escolhida.peso_estampa = 0.0;
    }
    std::cout << "F: " << descrever(escolhida) << " (" << segundos(ini) << " s)" << std::endl;

    // C + popularidade: a mesma formula com w2 = w3 = 0
    Resultado c_pop;
    bool tem_c_pop = false;
    for (double w1 : GRADE) {
        for (double w4 : GRADE) {
            if (w1 + w4 == 0.0) continue;
            ConfigF cfg = inicial;
            cfg.w = {w1, 0.0, 0.0, w4};
            Resultado r = registrar("C + popularidade", cfg);
            if (!tem_c_pop || melhor(r, c_pop)) c_pop = r;
            tem_c_pop = true;
        }
    }
    ConfigF cfg_c_pop = c_pop.cfg;
    cfg_c_pop.k_grupos = 0;
    cfg_c_pop.peso_estampa = 0.0;
    gravar_config_f(pasta_saida + "/config_f.csv", escolhida, cfg_c_pop);

    // variacoes na validacao
    struct Variacao {
        std::string nome;
        ConfigF cfg;
        bool desempata = true;
        std::string obs;
    };
    std::vector<Variacao> var;
    var.push_back({"F (escolhida)", escolhida, true, ""});
    var.push_back({"C + popularidade", cfg_c_pop, true, ""});
    var.push_back({"C (sem desempate, como na tabela corrigida)", ConfigF{}, false, ""});
    var.back().cfg.w[T_C] = 1.0;
    for (int t = 0; t < N_TERMOS; t++) {
        ConfigF cfg = t == T_GRUPOS ? com_grupos.cfg : escolhida;
        cfg.w = {0.0, 0.0, 0.0, 0.0};
        cfg.w[t] = 1.0;
        std::string obs;
        if (t == T_GRUPOS) obs = "K = " + std::to_string(cfg.k_grupos) + ", estampa " + peso_txt(cfg.peso_estampa);
        if (t == T_ATRIB) obs = "pesos de atributo da F";
        var.push_back({"Só " + NOME_TERMO[t], cfg, true, obs});
    }
    for (int t = 0; t < N_TERMOS; t++) {
        if (escolhida.w[t] == 0.0) continue;
        ConfigF cfg = escolhida;
        cfg.w[t] = 0.0;
        var.push_back({"F sem " + NOME_TERMO[t], cfg, true, ""});
    }
    if (!usa_grupos) {
        ConfigF cfg = escolhida;
        cfg.w[T_GRUPOS] = com_grupos.cfg.w[T_GRUPOS];
        cfg.k_grupos = com_grupos.cfg.k_grupos;
        cfg.peso_estampa = com_grupos.cfg.peso_estampa;
        var.push_back({"F com grupos", cfg, true,
                       "w3 = " + peso_txt(cfg.w[T_GRUPOS]) + ", K = " + std::to_string(cfg.k_grupos) + ", estampa " +
                           peso_txt(cfg.peso_estampa)});
    }
    if (escolhida.w[T_ATRIB] != 0.0) {
        for (int a = 0; a < N_ATRIB; a++) {
            if (escolhida.peso[a] == 0.0) continue;
            ConfigF cfg = escolhida;
            cfg.peso[a] = 0.0;
            bool vazio = std::all_of(cfg.peso.begin(), cfg.peso.end(), [](double p) { return p == 0.0; });
            var.push_back({"F sem " + NOME_ATRIB[a], cfg, true, vazio ? "nenhum atributo sobra" : ""});
        }
    }
    std::vector<Posicoes> pos_var;
    std::vector<Resultado> res_var;
    for (const Variacao& v : var) {
        pos_var.push_back(rodar(v.cfg, v.nome, v.desempata));
        res_var.push_back(resumir("variação", v.cfg, pos_var.back()));
    }
    std::cout << "Variações prontas (" << segundos(ini) << " s)" << std::endl;

    std::ofstream csv(pasta_saida + "/grade_f.csv");
    csv << "etapa,w_c,w_atributos,w_grupos,w_popularidade,cor,estampa,formalidade,estilo,ocasiao,comprimento,"
           "k_grupos,peso_estampa,hit1,hit5,hit10,fitb\n";
    for (const Resultado& r : grade) {
        csv << r.etapa;
        for (double w : r.cfg.w) csv << ',' << w;
        for (double p : r.cfg.peso) csv << ',' << p;
        csv << ',' << (r.cfg.w[T_GRUPOS] != 0.0 ? r.cfg.k_grupos : 0) << ','
            << (r.cfg.w[T_GRUPOS] != 0.0 ? r.cfg.peso_estampa : 0.0);
        for (double m : r.media) csv << ',' << m;
        csv << '\n';
    }

    // bootstrap nas mesmas consultas de validacao. indices: 0 = F, 1 = C + pop, 2 = C, 6 = so popularidade
    const int i_f = 0, i_cpop = 1, i_c = 2, i_pop = 3 + T_POP;
    std::vector<std::pair<int, int>> pares = {{i_f, i_pop}, {i_f, i_c}, {i_f, i_cpop}, {i_cpop, i_pop}, {i_cpop, i_c}};
    for (std::size_t v = 0; v < var.size(); v++) {
        if (var[v].nome == "F com grupos" || var[v].nome == "F sem grupos") pares.push_back({(int)v, i_f});
    }

    std::ofstream md(pasta_saida + "/escolha_final.md");
    md << "# Escolha final da versão F (validação)\n\n"
       << "Gerado por `./recomendador ajustar_f`. Tudo aqui usa só os looks de validação (semente 7), com a "
          "avaliação justa. As tabelas de pontos, os grupos, o grafo peça–look e a popularidade são montados sem "
          "esses looks. O teste não é montado.\n\n"
       << "- Consultas: " << nq << ". Cada número é a média das 10 ordens de desempate.\n"
       << "- `nota_F(c) = w1 × C(c) + w2 × pontos_atributos(c) + w3 × pontos_grupos(c) + w4 × popularidade(c)`.\n"
       << "- Cada termo vai de 0 a 1 dentro da consulta (mínimo e máximo entre as candidatas). Se todas as "
          "candidatas têm o mesmo valor, o termo vale 0.\n"
       << "- Popularidade = log(1 + grau da peça no grafo peça–look). No empate ganha a mais popular. "
          "Depois vale a ordem sorteada.\n"
       << "- Pesos de atributo na ordem cor; estampa; formalidade; estilo; ocasião; comprimento.\n\n"
       << "## Como a grade foi feita\n\n"
       << "A grade cheia teria 3^4 × 3^6 combinações para cada opção de grupos. Por isso:\n\n"
       << "1. Grade de w1 a w4 em (0; 0,5; 1) com os pesos de atributo em 1. Primeiro sem grupos (w3 = 0). "
          "Depois, para cada opção de grupos (K = 20, 50, 100 e peso da estampa 0; 0,1; 0,2), as combinações "
          "com w3 > 0. Os grupos só ficam se a melhor com grupos ganhar da melhor sem grupos.\n"
       << "2. Com os w fixos, cada peso de atributo passa por 0; 0,5 e 1, um de cada vez, com os outros fixos. "
          "Fica o valor que ganha.\n"
       << "3. Grade dos w de novo, com os pesos de atributo do passo 2.\n"
       << "4. Pesos de atributo de novo, um de cada vez.\n\n"
       << "Critério: maior Hit@10 (média das 10 ordens). No empate, maior Hit@5. Depois, a combinação com mais "
          "zeros. Por último, a com mais pesos iguais a 1, porque (1; 1) e (0,5; 0,5) dão a mesma lista. "
          "Foram " << grade.size() << " combinações. Todas estão em `grade_f.csv`.\n\n";

    md << "## Grupos com Kruskal\n\n"
       << "Melhor combinação de cada opção de grupos (só com w3 > 0) contra a melhor sem grupos, no passo 1.\n\n"
       << "| Opção | Melhor combinação | Hit@5 | Hit@10 | FITB |\n| --- | --- | --- | --- | --- |\n"
       << "| Sem grupos | " << descrever(sem_grupos.cfg) << " | " << pct(sem_grupos.media[1], 1) << " | "
       << pct(sem_grupos.media[2], 2) << " | " << pct(sem_grupos.media[3], 3) << " |\n";
    for (std::size_t i = 0; i < opcoes.size(); i++) {
        const Resultado& r = melhor_opcao[i];
        md << "| K = " << opcoes[i].k << ", estampa " << peso_txt(opcoes[i].pe) << " | " << descrever(r.cfg) << " | "
           << pct(r.media[1], 1) << " | " << pct(r.media[2], 2) << " | " << pct(r.media[3], 3) << " |\n";
    }
    md << "\n"
       << (usa_grupos ? "A melhor com grupos ganhou da melhor sem grupos. Os grupos ficam na F."
                      : "Nenhuma opção de grupos ganhou da melhor combinação sem grupos. Os grupos saem da F, que "
                        "fica com C, pontos por atributo e popularidade.")
       << "\n\n";

    md << "## Escolha\n\n"
       << "- F: " << descrever(escolhida) << ". Hit@10 = " << pct(atual.media[2], 2) << ", Hit@5 = "
       << pct(atual.media[1], 1) << ".\n"
       << "- C + popularidade: " << descrever(cfg_c_pop) << ". Hit@10 = " << pct(c_pop.media[2], 2) << ".\n"
       << "- As duas estão em `config_f.csv`, que o `avaliar_justo` lê.\n\n";
    md << "Caminho da escolha:\n\n| Passo | Melhor até aqui | Hit@5 | Hit@10 |\n| --- | --- | --- | --- |\n";
    {
        // melhor acumulado ao fim de cada passo
        Resultado acum;
        bool tem = false;
        std::string etapa_atual;
        const auto fechar = [&]() {
            if (tem) md << "| " << etapa_atual.substr(0, 1) << " | " << descrever(acum.cfg) << " | " << pct(acum.media[1], 1)
                        << " | " << pct(acum.media[2], 2) << " |\n";
        };
        for (const Resultado& r : grade) {
            if (r.etapa == "C + popularidade") continue;
            std::string e = r.etapa.substr(0, 2);
            if (e != etapa_atual.substr(0, 2) && tem) fechar();
            if (e != etapa_atual.substr(0, 2)) etapa_atual = r.etapa;
            if (!tem || melhor(r, acum)) acum = r;
            tem = true;
        }
        fechar();
    }
    md << "\n## Variações na validação\n\n"
       << "| Versão | Configuração | Hit@1 | Hit@5 | Hit@10 | FITB | Obs. |\n| --- | --- | --- | --- | --- | --- | --- |\n";
    for (std::size_t v = 0; v < var.size(); v++) {
        const Resultado& r = res_var[v];
        md << "| " << var[v].nome << " | " << descrever(var[v].cfg) << " | " << pct(r.media[0], 0) << " | "
           << pct(r.media[1], 1) << " | " << pct(r.media[2], 2) << " | " << pct(r.media[3], 3) << " | " << var[v].obs
           << " |\n";
    }
    md << "\n\"Só popularidade\" é a versão Popularidade. \"C (sem desempate)\" é a C da tabela corrigida. "
          "\"Só C\" é a mesma C com o desempate pela popularidade.\n\n"
       << "## Diferenças com intervalo de 95% (bootstrap, validação)\n\n"
       << "Mesmo bootstrap da avaliação justa: 1.000 sorteios dos looks com reposição. Se o intervalo não passa "
          "pelo 0, a diferença é real. Se passa, pode ser sorte. pp = pontos percentuais.\n\n"
       << "| Comparação | Métrica | Diferença | Intervalo de 95% | Conclusão |\n| --- | --- | --- | --- | --- |\n";
    for (const auto& par : pares) {
        for (int m = 0; m < 4; m++) {
            Intervalo it = bootstrap(por_consulta(pos_var[par.first], METRICAS[m]),
                                     por_consulta(pos_var[par.second], METRICAS[m]));
            int casas = METRICAS[m] == 0 ? 2 : 3;
            md << "| " << var[par.first].nome << " x " << var[par.second].nome << " | "
               << (METRICAS[m] == 0 ? std::string("FITB") : "Hit@" + std::to_string(METRICAS[m])) << " | "
               << num(100 * it.dif, casas) << " pp | " << num(100 * it.min, casas) << " a " << num(100 * it.max, casas)
               << " pp | " << conclusao(it) << " |\n";
        }
    }
    std::cout << "Escolha em " << pasta_saida << "/escolha_final.md (" << segundos(ini) << " s)" << std::endl;
    return true;
}
