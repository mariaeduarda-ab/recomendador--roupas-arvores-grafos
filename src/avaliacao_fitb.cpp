#include "avaliacao_fitb.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <unordered_map>

#include "atributos.hpp"
#include "avaliacao_d.hpp"
#include "grafo_looks.hpp"
#include "kdtree.hpp"
#include "versao_d.hpp"
#include "versao_e.hpp"

namespace {

constexpr unsigned SEMENTE_PERGUNTAS_VAL = 11;
constexpr unsigned SEMENTE_PERGUNTAS_TESTE = 13;
constexpr double Z_95 = 1.96;

const std::vector<int> GRADE_K_D = {5, 10, 20};
const std::vector<int> GRADE_K_E = {20, 50, 100};
const std::vector<double> GRADE_A = {0.5, 1.0};
const std::vector<double> GRADE_B = {0.0, 1.0};
const std::vector<double> GRADE_C = {0.0, 1.0};
const std::vector<double> GRADE_D = {1.0, 5.0};

double media(const std::vector<double>& x) {
    double soma = 0.0;
    for (double v : x) soma += v;
    return x.empty() ? 0.0 : soma / x.size();
}

double margem(const std::vector<double>& x) {
    if (x.size() < 2) return 0.0;
    double m = media(x);
    double soma = 0.0;
    for (double v : x) soma += (v - m) * (v - m);
    return Z_95 * std::sqrt(soma / (x.size() - 1) / x.size());
}

std::vector<double> rodar_perguntas(const Base& base, const std::vector<Pergunta>& perguntas, const Lista& gerar) {
    std::vector<double> res;
    for (const Pergunta& p : perguntas) {
        res.push_back(credito(gerar(sem_escondida(p.escondida, base), base.pecas[p.certa].categoria), p));
    }
    return res;
}

void gravar(std::ofstream& csv, const std::string& nome, const std::vector<double>& x) {
    double m = media(x);
    double e = margem(x);
    csv << nome << ',' << x.size() << ',' << m << ',' << m - e << ',' << m + e << std::endl;
    std::cout << nome << ": " << 100.0 * m << "%" << std::endl;
}

void gravar_dif(std::ofstream& csv, const std::string& nx, const std::vector<double>& x, const std::string& ny,
                const std::vector<double>& y) {
    std::vector<double> d(x.size());
    for (std::size_t i = 0; i < x.size(); i++) d[i] = x[i] - y[i];
    double m = media(d);
    double e = margem(d);
    csv << nx << ',' << ny << ',' << m << ',' << m - e << ',' << m + e << std::endl;
}

struct Pesos {
    double a, b, c, d;
};

std::vector<Pesos> montar_grade() {
    std::vector<Pesos> grade;
    for (double a : GRADE_A) for (double b : GRADE_B) for (double c : GRADE_C) for (double d : GRADE_D) grade.push_back({a, b, c, d});
    return grade;
}

// a versao E guarda as partes da nota, entao cada peso sai sem refazer a busca
ConfigE escolher_e(const VersaoE& versao, const Base& base, const std::vector<Pergunta>& perguntas, ConfigE forma,
                   const std::string& nome, std::ofstream& csv, double& melhor_nota, ConfigE melhor) {
    const std::vector<Pesos> grade = montar_grade();
    std::vector<double> soma(grade.size(), 0.0);
    for (const Pergunta& p : perguntas) {
        std::vector<Partes> partes = versao.partes(sem_escondida(p.escondida, base), base.pecas[p.certa].categoria, forma);
        std::vector<Partes> so_opcoes;
        for (const Partes& x : partes) {
            if (std::find(p.opcoes.begin(), p.opcoes.end(), x.indice) != p.opcoes.end()) so_opcoes.push_back(x);
        }
        for (int g = 0; g < (int)grade.size(); g++) {
            std::vector<Candidata> lista;
            for (const Partes& x : so_opcoes) lista.push_back({x.indice, x.nota(grade[g].a, grade[g].b, grade[g].c, grade[g].d)});
            soma[g] += credito(lista, p);
        }
    }
    for (int g = 0; g < (int)grade.size(); g++) {
        double nota = perguntas.empty() ? 0.0 : soma[g] / perguntas.size();
        csv << nome << ',' << forma.k << ",," << grade[g].a << ',' << grade[g].b << ',' << grade[g].c << ',' << grade[g].d
            << ',' << nota << std::endl;
        if (nota > melhor_nota) {
            melhor_nota = nota;
            melhor = forma;
            melhor.a = grade[g].a;
            melhor.b = grade[g].b;
            melhor.c = grade[g].c;
            melhor.d = grade[g].d;
        }
    }
    return melhor;
}

}  // namespace

std::vector<Pergunta> montar_perguntas(const Base& base, const std::vector<Escondida>& testes, unsigned semente) {
    std::unordered_map<std::string, std::vector<int>> por_cat;
    for (int i = 0; i < (int)base.pecas.size(); i++) por_cat[base.pecas[i].categoria].push_back(i);

    std::vector<char> no_treino(base.pecas.size(), 0);
    for (const Look& look : base.looks) {
        if (!look.treino) continue;
        for (int p : look.pecas) no_treino[p] = 1;
    }

    std::mt19937 gerador(semente);
    std::vector<Pergunta> res;
    for (const Escondida& e : testes) {
        const std::vector<int>& look = base.looks[e.look].pecas;
        int certa = look[e.pos];
        std::vector<int> cand;
        for (int q : por_cat[base.pecas[certa].categoria]) {
            // 27% das escondidas aparecem no treino, contra 83% do catalogo. sem isso, basta
            // preferir a peca menos vista pra acertar, e a prova mede a divisao, nao o look
            if (no_treino[q] == no_treino[certa] && std::find(look.begin(), look.end(), q) == look.end()) cand.push_back(q);
        }
        if ((int)cand.size() < N_ERRADAS) continue;

        Pergunta p;
        p.escondida = e;
        p.certa = certa;
        p.opcoes.push_back(certa);
        for (int i = 0; i < N_ERRADAS; i++) {
            std::uniform_int_distribution<int> sorteio(i, (int)cand.size() - 1);
            std::swap(cand[i], cand[sorteio(gerador)]);
            p.opcoes.push_back(cand[i]);
        }
        res.push_back(p);
    }
    return res;
}

double credito(const std::vector<Candidata>& lista, const Pergunta& p) {
    const double fora = -std::numeric_limits<double>::infinity();
    double melhor = fora;
    double nota_certa = fora;
    std::unordered_map<int, double> nota;
    for (const Candidata& c : lista) nota[c.indice] = c.pontuacao;
    for (int o : p.opcoes) {
        auto it = nota.find(o);
        double n = it != nota.end() ? it->second : fora;
        melhor = std::max(melhor, n);
        if (o == p.certa) nota_certa = n;
    }
    if (nota_certa < melhor) return 0.0;
    int empatadas = 0;
    for (int o : p.opcoes) {
        auto it = nota.find(o);
        if ((it != nota.end() ? it->second : fora) == melhor) empatadas++;
    }
    return 1.0 / empatadas;
}

bool avaliar_fitb(const Base& base, const std::string& pasta_dados, const std::string& tabela_cores,
                  const std::string& pasta_saida) {
    std::vector<Atrib> attr;
    std::vector<AtribE> attr_e;
    TabelaCores cores;
    std::string erro;
    if (!ler_atributos_da_pasta(pasta_dados, base, attr, erro) || !cores.ler(tabela_cores, erro) ||
        !ler_atributos_e(pasta_dados + "/pecas_v3.csv", base, cores, attr_e, erro)) {
        std::cerr << erro << '\n';
        return false;
    }
    std::filesystem::create_directories(pasta_saida);

    // os looks de validacao saem do grafo e do bonus. o teste ja estava fora
    Base sem_val = base;
    std::vector<int> val = sortear_validacao(base);
    for (int l : val) sem_val.looks[l].treino = false;
    GrafoLooks grafo_val;
    grafo_val.construir(sem_val);
    const std::vector<Pergunta> perg_val = montar_perguntas(sem_val, sortear_escondidas(sem_val, val), SEMENTE_PERGUNTAS_VAL);

    std::ofstream val_csv(pasta_saida + "/validacao_fitb.csv");
    val_csv << "forma,k,mutuo,a,b,c,d,nota\n";

    VersaoE e_val(sem_val, attr, attr_e, cores, grafo_val);
    ConfigE cfg_e;
    double nota_e = -1.0;
    for (int k : GRADE_K_E) {
        ConfigE forma;
        forma.k = k;
        cfg_e = escolher_e(e_val, sem_val, perg_val, forma, "E arvores", val_csv, nota_e, cfg_e);
    }
    ConfigE cfg_i;
    cfg_i.categoria_inteira = true;
    double nota_i = -1.0;
    cfg_i = escolher_e(e_val, sem_val, perg_val, cfg_i, "E categoria inteira", val_csv, nota_i, cfg_i);
    std::cout << "Validacao E: " << nota_e << ", E inteira: " << nota_i << std::endl;

    ConfigD cfg_d;
    double nota_d = -1.0;
    for (int k : GRADE_K_D) {
        for (bool mutuo : {true, false}) {
            ConfigD forma;
            forma.k = k;
            forma.mutuo = mutuo;
            VersaoD versao(sem_val, attr, grafo_val, forma);
            for (const Pesos& g : montar_grade()) {
                versao.trocar_pesos(g.a, g.b, g.c, g.d);
                double nota = media(rodar_perguntas(sem_val, perg_val, [&](const std::vector<int>& l, const std::string& c) {
                    return versao.completar(l, c, (int)sem_val.pecas.size());
                }));
                val_csv << "D," << k << ',' << mutuo << ',' << g.a << ',' << g.b << ',' << g.c << ',' << g.d << ',' << nota << std::endl;
                if (nota > nota_d) {
                    nota_d = nota;
                    cfg_d = forma;
                    cfg_d.a = g.a;
                    cfg_d.b = g.b;
                    cfg_d.c = g.c;
                    cfg_d.d = g.d;
                }
            }
            std::cout << "Validacao D k = " << k << " mutuo = " << mutuo << ": " << nota_d << std::endl;
        }
    }

    std::ofstream cfg_csv(pasta_saida + "/config_fitb.csv");
    cfg_csv << "versao,k,mutuo,a,b,c,d\n"
            << "E arvores," << cfg_e.k << ",," << cfg_e.a << ',' << cfg_e.b << ',' << cfg_e.c << ',' << cfg_e.d << '\n'
            << "E categoria inteira,,," << cfg_i.a << ',' << cfg_i.b << ',' << cfg_i.c << ',' << cfg_i.d << '\n'
            << "D," << cfg_d.k << ',' << cfg_d.mutuo << ',' << cfg_d.a << ',' << cfg_d.b << ',' << cfg_d.c << ',' << cfg_d.d << std::endl;

    std::vector<int> looks_teste;
    for (int l = 0; l < (int)base.looks.size(); l++) {
        if (!base.looks[l].treino) looks_teste.push_back(l);
    }
    const std::vector<Pergunta> perguntas = montar_perguntas(base, sortear_escondidas(base, looks_teste), SEMENTE_PERGUNTAS_TESTE);

    KDTree arvore((int)base.pecas[0].v.size());
    arvore.construir(base.pecas);
    GrafoLooks grafo;
    grafo.construir(base);
    Recomendador rec(base, arvore, grafo);
    VersaoE ve(base, attr, attr_e, cores, grafo);
    VersaoD vd(base, attr, grafo, cfg_d);
    const int tudo = (int)base.pecas.size();

    const auto com_e = [&](const ConfigE& c) {
        return [&ve, c, tudo](const std::vector<int>& l, const std::string& cat) { return ve.completar(l, cat, c, tudo); };
    };

    std::ofstream csv(pasta_saida + "/fitb.csv");
    csv << "versao,perguntas,nota,ic95_min,ic95_max\n";
    csv << "Aleatoria (esperado)," << perguntas.size() << ",0.25,0.25,0.25" << std::endl;

    std::vector<double> pop = rodar_perguntas(base, perguntas, [&](const std::vector<int>& l, const std::string& c) { return rec.popularidade(l, c, tudo); });
    gravar(csv, "Popularidade", pop);
    std::vector<double> va = rodar_perguntas(base, perguntas, [&](const std::vector<int>& l, const std::string& c) { return rec.versao_a(l, c, tudo); });
    gravar(csv, "A", va);
    std::vector<double> vb = rodar_perguntas(base, perguntas, [&](const std::vector<int>& l, const std::string& c) { return rec.versao_b(l, c, tudo); });
    gravar(csv, "B", vb);
    std::vector<double> vc = rodar_perguntas(base, perguntas, [&](const std::vector<int>& l, const std::string& c) { return rec.versao_c(l, c, tudo); });
    gravar(csv, "C", vc);
    std::vector<double> nd = rodar_perguntas(base, perguntas, [&](const std::vector<int>& l, const std::string& c) { return vd.completar(l, c, tudo); });
    gravar(csv, "D", nd);
    std::vector<double> ne = rodar_perguntas(base, perguntas, com_e(cfg_e));
    gravar(csv, "E com KD-tree", ne);
    std::vector<double> ni = rodar_perguntas(base, perguntas, com_e(cfg_i));
    gravar(csv, "E na categoria inteira", ni);

    std::ofstream dif(pasta_saida + "/diferencas_fitb.csv");
    dif << "x,y,diferenca,ic95_min,ic95_max\n";
    gravar_dif(dif, "E inteira", ni, "Popularidade", pop);
    gravar_dif(dif, "E inteira", ni, "A", va);
    gravar_dif(dif, "E inteira", ni, "B", vb);
    gravar_dif(dif, "E inteira", ni, "C", vc);
    gravar_dif(dif, "E inteira", ni, "D", nd);
    gravar_dif(dif, "E inteira", ni, "E com KD-tree", ne);

    const auto variacao = [&](const std::string& nome, ConfigE c) {
        std::vector<double> x = rodar_perguntas(base, perguntas, com_e(c));
        gravar(csv, nome, x);
        gravar_dif(dif, nome, x, "E inteira", ni);
    };
    ConfigE v = cfg_i;
    v.b = 0.0;
    variacao("E inteira sem regra de cor", v);
    v = cfg_i;
    v.c = 0.0;
    variacao("E inteira sem regra de atencao", v);
    v = cfg_i;
    v.c = 1.0;
    variacao("E inteira com regra de atencao", v);
    v = cfg_i;
    v.d = 0.0;
    variacao("E inteira sem bonus de looks", v);
    v = cfg_i;
    v.filtrar_cor = false;
    variacao("E inteira sem filtro de cor", v);
    return true;
}
