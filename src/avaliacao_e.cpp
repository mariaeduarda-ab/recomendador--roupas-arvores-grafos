#include "avaliacao_e.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "avaliacao_d.hpp"

namespace {

constexpr int REPETICOES = 3;
constexpr int CONSULTAS = 100;
constexpr int N_LISTA = 10;

const std::vector<int> GRADE_K = {20, 50, 100};
const std::vector<double> GRADE_A = {0.5, 1.0};
const std::vector<double> GRADE_B = {0.0, 1.0};
const std::vector<double> GRADE_C = {0.0, 1.0};
const std::vector<double> GRADE_D = {1.0, 5.0};

struct Pesos {
    double a, b, c, d;
};

// posicao da escondida: conta quem tem nota maior. no empate vale o menor indice, igual ao ordenar_e_cortar
int posicao(const std::vector<Partes>& lista, int alvo, const Pesos& p) {
    auto it = std::find_if(lista.begin(), lista.end(), [&](const Partes& x) { return x.indice == alvo; });
    if (it == lista.end()) return -1;
    double nota_alvo = it->nota(p.a, p.b, p.c, p.d);
    int pos = 0;
    for (const Partes& x : lista) {
        double n = x.nota(p.a, p.b, p.c, p.d);
        if (n > nota_alvo || (n == nota_alvo && x.indice < alvo)) pos++;
    }
    return pos;
}

// escolhe os pesos de uma forma de buscar. grava cada tentativa no csv
ConfigE melhor_config(const VersaoE& versao, const Base& base, const std::vector<Escondida>& testes,
                      const std::vector<Pesos>& grade, ConfigE cfg, const std::string& nome, std::ofstream& csv,
                      int& melhor10, int& melhor5, ConfigE melhor) {
    std::vector<std::vector<int>> pos(grade.size());
    std::vector<int> cobertos(grade.size(), 0);
    for (const Escondida& e : testes) {
        int alvo = base.looks[e.look].pecas[e.pos];
        std::vector<Partes> lista = versao.partes(sem_escondida(e, base), base.pecas[alvo].categoria, cfg);
        for (int g = 0; g < (int)grade.size(); g++) pos[g].push_back(posicao(lista, alvo, grade[g]));
    }
    for (int g = 0; g < (int)grade.size(); g++) {
        int h10 = conta_hit(pos[g], 10);
        int h5 = conta_hit(pos[g], 5);
        csv << nome << ',' << cfg.k << ',' << grade[g].a << ',' << grade[g].b << ',' << grade[g].c << ',' << grade[g].d
            << ',' << testes.size() << ',' << conta_hit(pos[g], 1) << ',' << h5 << ',' << h10 << ','
            << conta_cobertura(pos[g]) << std::endl;
        if (h10 > melhor10 || (h10 == melhor10 && h5 > melhor5)) {
            melhor10 = h10;
            melhor5 = h5;
            melhor = cfg;
            melhor.a = grade[g].a;
            melhor.b = grade[g].b;
            melhor.c = grade[g].c;
            melhor.d = grade[g].d;
        }
    }
    return melhor;
}

void gravar_config(std::ofstream& csv, const std::string& nome, const ConfigE& c) {
    csv << nome << ',' << c.k << ',' << c.a << ',' << c.b << ',' << c.c << ',' << c.d << std::endl;
    std::cout << nome << ": k = " << c.k << ", a = " << c.a << ", b = " << c.b << ", c = " << c.c << ", d = " << c.d
              << std::endl;
}

void medir_tempos_e(const std::string& pasta_dados, TabelaCores cores, const ConfigE& cfg,
                    const std::string& pasta_saida) {
    std::ofstream csv(pasta_saida + "/tempos_e.csv");
    csv << "pecas,arvores_ms,recomendacao_ms\n";
    for (const std::string tamanho : {"1000", "10000", "100000"}) {
        Base base;
        std::vector<Atrib> attr;
        std::vector<AtribE> attr_e;
        std::string erro;
        if (!ler_pecas(pasta_dados + "/pecas_" + tamanho + ".csv", base, erro) ||
            !ler_looks(pasta_dados + "/looks_" + tamanho + ".csv", base, erro) ||
            !ler_atributos(pasta_dados + "/pecas_v2_" + tamanho + ".csv", base, attr, erro) ||
            !ler_atributos_e(pasta_dados + "/pecas_v3_" + tamanho + ".csv", base, cores, attr_e, erro)) {
            std::cerr << erro << '\n';
            continue;
        }
        GrafoLooks grafo;
        grafo.construir(base);

        std::vector<int> todos;
        for (int l = 0; l < (int)base.looks.size(); l++) {
            if (!base.looks[l].treino && base.looks[l].pecas.size() >= 2) todos.push_back(l);
        }
        std::vector<Escondida> testes = sortear_escondidas(base, todos);
        if ((int)testes.size() > CONSULTAS) testes.resize(CONSULTAS);

        double arvores = 0.0;
        double rec = 0.0;
        for (int r = 0; r < REPETICOES; r++) {
            VersaoE versao(base, attr, attr_e, cores, grafo);
            arvores += versao.ms_arvores();
            auto ini = std::chrono::steady_clock::now();
            for (const Escondida& e : testes) {
                int alvo = base.looks[e.look].pecas[e.pos];
                versao.completar(sem_escondida(e, base), base.pecas[alvo].categoria, cfg, N_LISTA);
            }
            rec += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - ini).count();
        }
        csv << tamanho << ',' << arvores / REPETICOES << ',' << rec / REPETICOES / std::max(1, (int)testes.size())
            << std::endl;
    }
}

}  // namespace

bool avaliar_e(const Base& base, const std::string& pasta_dados, const std::string& tabela_cores,
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

    std::vector<Pesos> grade;
    for (double a : GRADE_A) for (double b : GRADE_B) for (double c : GRADE_C) for (double d : GRADE_D) grade.push_back({a, b, c, d});

    // os looks de validacao saem do grafo e do bonus. o teste ja estava fora
    Base sem_val = base;
    std::vector<int> val = sortear_validacao(base);
    for (int l : val) sem_val.looks[l].treino = false;
    GrafoLooks grafo_val;
    grafo_val.construir(sem_val);
    std::vector<Escondida> testes_val = sortear_escondidas(sem_val, val);
    VersaoE versao_val(sem_val, attr, attr_e, cores, grafo_val);

    std::ofstream val_csv(pasta_saida + "/validacao_e.csv");
    val_csv << "forma,k,a,b,c,d,looks,hit1,hit5,hit10,cobertura\n";

    ConfigE cfg;
    int m10 = -1;
    int m5 = -1;
    for (int k : GRADE_K) {
        ConfigE tentativa;
        tentativa.k = k;
        cfg = melhor_config(versao_val, sem_val, testes_val, grade, tentativa, "arvores", val_csv, m10, m5, cfg);
        std::cout << "Validacao k = " << k << ". Melhor ate agora: " << m10 << " de " << testes_val.size() << std::endl;
    }

    ConfigE inteira;
    inteira.categoria_inteira = true;
    int i10 = -1;
    int i5 = -1;
    inteira = melhor_config(versao_val, sem_val, testes_val, grade, inteira, "categoria inteira", val_csv, i10, i5, inteira);
    std::cout << "Validacao categoria inteira: " << i10 << " de " << testes_val.size() << std::endl;

    std::ofstream escolhida(pasta_saida + "/config_e.csv");
    escolhida << "forma,k,a,b,c,d\n";
    gravar_config(escolhida, "arvores", cfg);
    gravar_config(escolhida, "categoria_inteira", inteira);

    std::vector<int> looks_teste;
    for (int l = 0; l < (int)base.looks.size(); l++) {
        if (!base.looks[l].treino) looks_teste.push_back(l);
    }
    const std::vector<Escondida> testes = sortear_escondidas(base, looks_teste);

    KDTree arvore((int)base.pecas[0].v.size());
    arvore.construir(base.pecas);
    GrafoLooks grafo;
    grafo.construir(base);
    Recomendador rec(base, arvore, grafo);
    VersaoE versao(base, attr, attr_e, cores, grafo);
    const int tudo = (int)base.pecas.size();

    const auto com = [&](const ConfigE& c) {
        return [&versao, c, tudo](const std::vector<int>& l, const std::string& cat) { return versao.completar(l, cat, c, tudo); };
    };

    std::ofstream csv(pasta_saida + "/hit_e.csv");
    csv << "versao,looks,acertos1,acertos5,acertos10,hit1,hit5,hit10,cobertos,cobertura,candidatas_media\n";
    Linha pop = rodar("Popularidade", base, testes, [&](const std::vector<int>& l, const std::string& c) { return rec.popularidade(l, c, tudo); });
    gravar_linha(csv, pop);
    Linha va = rodar("A", base, testes, [&](const std::vector<int>& l, const std::string& c) { return rec.versao_a(l, c, tudo); });
    gravar_linha(csv, va);
    Linha vc = rodar("C", base, testes, [&](const std::vector<int>& l, const std::string& c) { return rec.versao_c(l, c, tudo); });
    gravar_linha(csv, vc);

    Linha ve = rodar("E", base, testes, com(cfg));
    gravar_linha(csv, ve);
    Linha vi = rodar("E na categoria inteira", base, testes, com(inteira));
    gravar_linha(csv, vi);

    std::ofstream dif(pasta_saida + "/diferencas_e.csv");
    dif << "x,y,k,so_x_acerta,so_y_acerta,diferenca,ic95_min,ic95_max\n";
    gravar_diferenca(dif, ve, vc);
    gravar_diferenca(dif, ve, pop);
    gravar_diferenca(dif, vi, vc);
    gravar_diferenca(dif, vi, pop);
    gravar_diferenca(dif, vi, va);

    ConfigE v = cfg;
    v.b = 1.0;
    v.c = 1.0;
    gravar_linha(csv, rodar("E com regras ligadas (b = c = 1)", base, testes, com(v)));
    v = cfg;
    v.b = 0.0;
    gravar_linha(csv, rodar("E sem regra de cor", base, testes, com(v)));
    v = cfg;
    v.c = 0.0;
    gravar_linha(csv, rodar("E sem regra de atencao", base, testes, com(v)));
    v = cfg;
    v.d = 0.0;
    gravar_linha(csv, rodar("E sem bonus de looks", base, testes, com(v)));
    v = cfg;
    v.filtrar_cor = false;
    gravar_linha(csv, rodar("E sem filtro de cor na busca", base, testes, com(v)));

    medir_tempos_e(pasta_dados, cores, cfg, pasta_saida);
    return true;
}

bool ler_config_e(const std::string& caminho, ConfigE& cfg) {
    std::ifstream arquivo(caminho);
    std::string linha;
    std::getline(arquivo, linha);
    if (!std::getline(arquivo, linha)) return false;
    std::vector<std::string> col = separar_colunas(linha);
    if (col.size() != 6) return false;
    cfg.k = std::stoi(col[1]);
    cfg.a = std::stod(col[2]);
    cfg.b = std::stod(col[3]);
    cfg.c = std::stod(col[4]);
    cfg.d = std::stod(col[5]);
    return true;
}
