#include "avaliacao_d.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <random>
#include <unordered_map>
#include <vector>

#include "atributos.hpp"
#include "avaliacao.hpp"
#include "grafo_looks.hpp"
#include "kdtree.hpp"
#include "recomendador.hpp"
#include "versao_d.hpp"

namespace {

constexpr unsigned SEMENTE_VALIDACAO = 7;
constexpr unsigned SEMENTE_ALEATORIA = 123;
constexpr double FRACAO_VALIDACAO = 0.1;
constexpr int REPETICOES = 3;
constexpr int CONSULTAS = 100;
constexpr int N_LISTA = 10;
constexpr double Z_95 = 1.96;
const int VALORES_K[3] = {1, 5, 10};

const std::vector<int> GRADE_K = {5, 10, 20};
const std::vector<double> GRADE_A = {0.5, 1.0};
const std::vector<double> GRADE_B = {0.0, 1.0};
const std::vector<double> GRADE_C = {0.0, 1.0};
const std::vector<double> GRADE_D = {1.0, 5.0};

}  // namespace

int conta_hit(const std::vector<int>& pos, int k) {
    int total = 0;
    for (int p : pos) {
        if (p >= 0 && p < k) total++;
    }
    return total;
}

int conta_cobertura(const std::vector<int>& pos) {
    int total = 0;
    for (int p : pos) {
        if (p >= 0) total++;
    }
    return total;
}

std::vector<int> sem_escondida(const Escondida& e, const Base& base) {
    std::vector<int> resto = base.looks[e.look].pecas;
    resto.erase(resto.begin() + e.pos);
    return resto;
}

Linha rodar(const std::string& nome, const Base& base, const std::vector<Escondida>& testes, const Lista& gerar) {
    Linha linha;
    linha.nome = nome;
    double soma = 0.0;
    for (const Escondida& e : testes) {
        int alvo = base.looks[e.look].pecas[e.pos];
        std::vector<Candidata> lista = gerar(sem_escondida(e, base), base.pecas[alvo].categoria);
        soma += (double)lista.size();
        int achou = -1;
        for (int i = 0; i < (int)lista.size(); i++) {
            if (lista[i].indice == alvo) {
                achou = i;
                break;
            }
        }
        linha.pos.push_back(achou);
    }
    linha.candidatas = testes.empty() ? 0.0 : soma / testes.size();
    return linha;
}

void gravar_linha(std::ofstream& csv, const Linha& l) {
    int n = (int)l.pos.size();
    csv << l.nome << ',' << n;
    for (int k : VALORES_K) csv << ',' << conta_hit(l.pos, k);
    for (int k : VALORES_K) csv << ',' << (n > 0 ? (double)conta_hit(l.pos, k) / n : 0.0);
    csv << ',' << conta_cobertura(l.pos) << ',' << (n > 0 ? (double)conta_cobertura(l.pos) / n : 0.0)
        << ',' << l.candidatas << std::endl;
    std::cout << l.nome << ": " << conta_hit(l.pos, 10) << " acertos no top 10" << std::endl;
}

namespace {

// sorteia N_LISTA pecas da categoria. fora delas a escondida ainda e candidata, entao pos fica >= N_LISTA
Linha rodar_aleatoria(const Base& base, const std::vector<Escondida>& testes) {
    std::unordered_map<std::string, std::vector<int>> por_cat;
    for (int i = 0; i < (int)base.pecas.size(); i++) por_cat[base.pecas[i].categoria].push_back(i);

    std::mt19937 gerador(SEMENTE_ALEATORIA);
    Linha linha;
    linha.nome = "Aleatoria (sorteio)";
    double soma = 0.0;
    for (const Escondida& e : testes) {
        int alvo = base.looks[e.look].pecas[e.pos];
        std::vector<int> resto = sem_escondida(e, base);
        std::vector<int> cand = por_cat[base.pecas[alvo].categoria];
        cand.erase(std::remove_if(cand.begin(), cand.end(),
                                  [&](int p) { return std::find(resto.begin(), resto.end(), p) != resto.end(); }),
                   cand.end());
        soma += (double)cand.size();
        int lim = std::min(N_LISTA, (int)cand.size());
        int achou = N_LISTA;
        for (int i = 0; i < lim; i++) {
            std::uniform_int_distribution<int> sorteio(i, (int)cand.size() - 1);
            std::swap(cand[i], cand[sorteio(gerador)]);
            if (cand[i] == alvo) achou = i;
        }
        linha.pos.push_back(achou);
    }
    linha.candidatas = testes.empty() ? 0.0 : soma / testes.size();
    return linha;
}

// valor esperado de escolher ao acaso: k / candidatas, em cada look
void gravar_esperado(std::ofstream& csv, const Base& base, const std::vector<Escondida>& testes) {
    std::unordered_map<std::string, int> tam;
    for (const Peca& p : base.pecas) tam[p.categoria]++;
    double soma[3] = {};
    double cand_total = 0.0;
    for (const Escondida& e : testes) {
        int alvo = base.looks[e.look].pecas[e.pos];
        const std::string& cat = base.pecas[alvo].categoria;
        int m = tam[cat];
        for (int p : sem_escondida(e, base)) {
            if (base.pecas[p].categoria == cat) m--;
        }
        cand_total += m;
        for (int i = 0; i < 3; i++) soma[i] += (double)std::min(VALORES_K[i], m) / m;
    }
    int n = (int)testes.size();
    csv << "Aleatoria (esperado)," << n;
    for (int i = 0; i < 3; i++) csv << ',' << soma[i];
    for (int i = 0; i < 3; i++) csv << ',' << (n > 0 ? soma[i] / n : 0.0);
    csv << ',' << n << ",1," << (n > 0 ? cand_total / n : 0.0) << std::endl;
}

}  // namespace

// diferenca pareada: mesmos looks nas duas versoes. se o intervalo passa pelo 0, nao da pra afirmar
void gravar_diferenca(std::ofstream& csv, const Linha& x, const Linha& y) {
    int n = (int)x.pos.size();
    for (int k : VALORES_K) {
        double soma = 0.0;
        double soma2 = 0.0;
        int so_x = 0;
        int so_y = 0;
        for (int i = 0; i < n; i++) {
            int hx = x.pos[i] >= 0 && x.pos[i] < k;
            int hy = y.pos[i] >= 0 && y.pos[i] < k;
            soma += hx - hy;
            soma2 += (hx - hy) * (hx - hy);
            if (hx > hy) so_x++;
            if (hy > hx) so_y++;
        }
        double media = n > 0 ? soma / n : 0.0;
        double var = n > 1 ? (soma2 - n * media * media) / (n - 1) : 0.0;
        double margem = Z_95 * std::sqrt(var / std::max(n, 1));
        csv << x.nome << ',' << y.nome << ',' << k << ',' << so_x << ',' << so_y << ',' << media << ','
            << media - margem << ',' << media + margem << std::endl;
    }
}

std::vector<int> sortear_validacao(const Base& base) {
    std::vector<int> treino;
    for (int l = 0; l < (int)base.looks.size(); l++) {
        if (base.looks[l].treino) treino.push_back(l);
    }
    std::mt19937 gerador(SEMENTE_VALIDACAO);
    std::shuffle(treino.begin(), treino.end(), gerador);
    treino.resize((std::size_t)(treino.size() * FRACAO_VALIDACAO));
    return treino;
}

namespace {

ConfigD escolher_config(const Base& base, const std::vector<Atrib>& attr, const std::string& pasta_saida) {
    std::vector<int> val = sortear_validacao(base);

    // os looks de validacao saem do grafo e do bonus. o teste ja estava fora
    Base sem_val = base;
    for (int l : val) sem_val.looks[l].treino = false;
    GrafoLooks grafo;
    grafo.construir(sem_val);
    std::vector<Escondida> testes = sortear_escondidas(sem_val, val);

    std::ofstream csv(pasta_saida + "/validacao_d.csv");
    csv << "k,mutuo,a,b,c,d,looks,hit1,hit5,hit10,cobertura\n";
    ConfigD melhor;
    int melhor10 = -1;
    int melhor5 = -1;
    for (int k : GRADE_K) {
        for (bool mutuo : {true, false}) {
            ConfigD cfg;
            cfg.k = k;
            cfg.mutuo = mutuo;
            VersaoD versao(sem_val, attr, grafo, cfg);
            for (double a : GRADE_A) for (double b : GRADE_B) for (double c : GRADE_C) for (double d : GRADE_D) {
                versao.trocar_pesos(a, b, c, d);
                Linha l = rodar("", sem_val, testes, [&](const std::vector<int>& look, const std::string& cat) {
                    return versao.completar(look, cat, (int)sem_val.pecas.size());
                });
                int h10 = conta_hit(l.pos, 10);
                int h5 = conta_hit(l.pos, 5);
                csv << k << ',' << mutuo << ',' << a << ',' << b << ',' << c << ',' << d << ',' << testes.size() << ','
                    << conta_hit(l.pos, 1) << ',' << h5 << ',' << h10 << ',' << conta_cobertura(l.pos) << std::endl;
                // na grade, o primeiro que empata fica: k menor e mutuo vem antes
                if (h10 > melhor10 || (h10 == melhor10 && h5 > melhor5)) {
                    melhor10 = h10;
                    melhor5 = h5;
                    melhor = cfg;
                    melhor.a = a;
                    melhor.b = b;
                    melhor.c = c;
                    melhor.d = d;
                }
            }
            std::cout << "Validacao k = " << k << " mutuo = " << mutuo << " pronta. Melhor ate agora: " << melhor10
                      << " de " << testes.size() << std::endl;
        }
    }
    return melhor;
}

double medir_ms(const std::function<void()>& tarefa) {
    auto ini = std::chrono::steady_clock::now();
    tarefa();
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - ini).count();
}

void medir_tempos_d(const std::string& pasta_dados, const std::string& pasta_saida, const ConfigD& cfg) {
    std::ofstream csv(pasta_saida + "/tempos_d.csv");
    csv << "pecas,arvores_ms,arestas_ms,recomendacao_ms\n";
    for (const std::string tamanho : {"1000", "10000", "100000"}) {
        Base base;
        std::vector<Atrib> attr;
        std::string erro;
        if (!ler_pecas(pasta_dados + "/pecas_" + tamanho + ".csv", base, erro) ||
            !ler_looks(pasta_dados + "/looks_" + tamanho + ".csv", base, erro) ||
            !ler_atributos(pasta_dados + "/pecas_v2_" + tamanho + ".csv", base, attr, erro)) {
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
        double arestas = 0.0;
        double rec = 0.0;
        for (int r = 0; r < REPETICOES; r++) {
            VersaoD versao(base, attr, grafo, cfg);
            arvores += versao.ms_arvores();
            arestas += versao.ms_arestas();
            rec += medir_ms([&] {
                for (const Escondida& e : testes) {
                    int alvo = base.looks[e.look].pecas[e.pos];
                    versao.completar(sem_escondida(e, base), base.pecas[alvo].categoria, N_LISTA);
                }
            });
        }
        int q = std::max(1, (int)testes.size());
        csv << tamanho << ',' << arvores / REPETICOES << ',' << arestas / REPETICOES << ',' << rec / REPETICOES / q
            << std::endl;
    }
}

}  // namespace

bool avaliar_d(const Base& base, const std::string& pasta_dados, const std::string& pasta_saida) {
    std::vector<Atrib> attr;
    std::string erro;
    if (!ler_atributos_da_pasta(pasta_dados, base, attr, erro)) {
        std::cerr << erro << '\n';
        return false;
    }
    std::filesystem::create_directories(pasta_saida);

    const ConfigD cfg = escolher_config(base, attr, pasta_saida);
    std::ofstream escolhida(pasta_saida + "/config_d.csv");
    escolhida << "k,mutuo,a,b,c,d\n"
              << cfg.k << ',' << cfg.mutuo << ',' << cfg.a << ',' << cfg.b << ',' << cfg.c << ',' << cfg.d << std::endl;
    std::cout << "Config escolhida na validacao: k = " << cfg.k << ", mutuo = " << cfg.mutuo << ", a = " << cfg.a
              << ", b = " << cfg.b << ", c = " << cfg.c << ", d = " << cfg.d << std::endl;

    std::vector<int> looks_teste;
    for (int l = 0; l < (int)base.looks.size(); l++) {
        if (!base.looks[l].treino) looks_teste.push_back(l);
    }
    // mesma funcao e mesma semente do avaliar_hit: todas as versoes escondem as mesmas pecas
    const std::vector<Escondida> testes = sortear_escondidas(base, looks_teste);
    std::ofstream lista_esc(pasta_saida + "/escondidas.csv");
    lista_esc << "look_id,peca_id\n";
    for (const Escondida& e : testes) {
        lista_esc << base.looks[e.look].id << ',' << base.pecas[base.looks[e.look].pecas[e.pos]].id << '\n';
    }
    lista_esc.flush();

    KDTree arvore((int)base.pecas[0].v.size());
    arvore.construir(base.pecas);
    GrafoLooks grafo;
    grafo.construir(base);
    Recomendador rec(base, arvore, grafo);
    const int tudo = (int)base.pecas.size();

    std::ofstream csv(pasta_saida + "/hit_d.csv");
    csv << "versao,looks,acertos1,acertos5,acertos10,hit1,hit5,hit10,cobertos,cobertura,candidatas_media\n";
    gravar_esperado(csv, base, testes);

    Linha aleat = rodar_aleatoria(base, testes);
    gravar_linha(csv, aleat);
    Linha pop = rodar("Popularidade", base, testes, [&](const std::vector<int>& l, const std::string& c) {
        return rec.popularidade(l, c, tudo);
    });
    gravar_linha(csv, pop);
    Linha va = rodar("A", base, testes, [&](const std::vector<int>& l, const std::string& c) { return rec.versao_a(l, c, tudo); });
    gravar_linha(csv, va);
    Linha vb = rodar("B", base, testes, [&](const std::vector<int>& l, const std::string& c) { return rec.versao_b(l, c, tudo); });
    gravar_linha(csv, vb);
    Linha vc = rodar("C", base, testes, [&](const std::vector<int>& l, const std::string& c) { return rec.versao_c(l, c, tudo); });
    gravar_linha(csv, vc);

    VersaoD versao(base, attr, grafo, cfg);
    const auto com_d = [&](const VersaoD& v) {
        return [&](const std::vector<int>& l, const std::string& c) { return v.completar(l, c, tudo); };
    };
    Linha vd = rodar("D", base, testes, com_d(versao));
    gravar_linha(csv, vd);

    std::ofstream dif(pasta_saida + "/diferencas_d.csv");
    dif << "x,y,k,so_x_acerta,so_y_acerta,diferenca,ic95_min,ic95_max\n";
    gravar_diferenca(dif, vd, vb);
    gravar_diferenca(dif, vd, vc);
    gravar_diferenca(dif, vd, pop);

    versao.trocar_pesos(cfg.a, 0.0, 0.0, cfg.d);
    gravar_linha(csv, rodar("D sem regras de cor e estampa", base, testes, com_d(versao)));
    versao.trocar_pesos(cfg.a, cfg.b, cfg.c, 0.0);
    gravar_linha(csv, rodar("D sem bonus de looks", base, testes, com_d(versao)));
    versao.trocar_pesos(1.0, 1.0, 1.0, 1.0);
    gravar_linha(csv, rodar("D com pesos 1 (mesmo k e regra)", base, testes, com_d(versao)));

    ConfigD outro = cfg;
    outro.mutuo = !cfg.mutuo;
    VersaoD versao_outra(base, attr, grafo, outro);
    gravar_linha(csv, rodar(outro.mutuo ? "D com kNN mutuo" : "D com kNN comum", base, testes, com_d(versao_outra)));

    medir_tempos_d(pasta_dados, pasta_saida, cfg);
    std::cout << "Tempos em " << pasta_saida << "/tempos_d.csv" << std::endl;

    // por ultimo: leva bem mais tempo que o resto
    ConfigD misturado = cfg;
    misturado.misturar = true;
    VersaoD versao_mist(base, attr, grafo, misturado);
    gravar_linha(csv, rodar("D com atributos misturados", base, testes, com_d(versao_mist)));
    return true;
}

bool ler_config_d(const std::string& caminho, ConfigD& cfg) {
    std::ifstream arquivo(caminho);
    std::string cabecalho;
    std::getline(arquivo, cabecalho);
    int mutuo = 0;
    char v;
    ConfigD lida;
    if (!(arquivo >> lida.k >> v >> mutuo >> v >> lida.a >> v >> lida.b >> v >> lida.c >> v >> lida.d)) return false;
    lida.mutuo = mutuo != 0;
    cfg = lida;
    return true;
}

bool reavaliar(const Base& base, const std::string& pasta_dados, const ConfigD& cfg, const std::string& pasta_saida) {
    std::vector<Atrib> attr;
    std::string erro;
    if (!ler_atributos_da_pasta(pasta_dados, base, attr, erro)) {
        std::cerr << erro << '\n';
        return false;
    }
    std::filesystem::create_directories(pasta_saida);

    std::vector<int> looks_teste;
    for (int l = 0; l < (int)base.looks.size(); l++) {
        if (!base.looks[l].treino) looks_teste.push_back(l);
    }
    const std::vector<Escondida> testes = sortear_escondidas(base, looks_teste);
    std::ofstream lista_esc(pasta_saida + "/escondidas.csv");
    lista_esc << "look_id,peca_id\n";
    for (const Escondida& e : testes) {
        lista_esc << base.looks[e.look].id << ',' << base.pecas[base.looks[e.look].pecas[e.pos]].id << '\n';
    }

    KDTree arvore((int)base.pecas[0].v.size());
    arvore.construir(base.pecas);
    GrafoLooks grafo;
    grafo.construir(base);
    Recomendador rec(base, arvore, grafo);
    VersaoD versao(base, attr, grafo, cfg);
    const int tudo = (int)base.pecas.size();

    std::ofstream csv(pasta_saida + "/reavaliar.csv");
    csv << "versao,looks,acertos1,acertos5,acertos10,hit1,hit5,hit10,cobertos,cobertura,candidatas_media\n";
    gravar_linha(csv, rodar("Popularidade", base, testes, [&](const std::vector<int>& l, const std::string& c) { return rec.popularidade(l, c, tudo); }));
    gravar_linha(csv, rodar("B", base, testes, [&](const std::vector<int>& l, const std::string& c) { return rec.versao_b(l, c, tudo); }));
    gravar_linha(csv, rodar("C", base, testes, [&](const std::vector<int>& l, const std::string& c) { return rec.versao_c(l, c, tudo); }));
    gravar_linha(csv, rodar("D", base, testes, [&](const std::vector<int>& l, const std::string& c) { return versao.completar(l, c, tudo); }));
    return true;
}
