#include "avaliacao.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>

#include "grafo_looks.hpp"
#include "grafo_similaridade.hpp"

namespace {

constexpr unsigned SEMENTE = 42;
constexpr int REPETICOES = 3;
constexpr int CONSULTAS = 100;
constexpr int K_TEMPOS = 10;
constexpr int N_VERSOES = 4;
constexpr int N_K = 3;

double medir_ms(const std::function<void()>& tarefa) {
    double total = 0.0;
    for (int i = 0; i < REPETICOES; i++) {
        auto inicio = std::chrono::steady_clock::now();
        tarefa();
        auto fim = std::chrono::steady_clock::now();
        total += std::chrono::duration<double, std::milli>(fim - inicio).count();
    }
    return total / REPETICOES;
}

void medir_base(const Base& base, const std::string& rotulo, std::ostream& saida) {
    if (base.pecas.empty()) return;
    int dim = (int)base.pecas[0].v.size();
    int n = (int)base.pecas.size();

    double construir = medir_ms([&] {
        KDTree arvore(dim);
        arvore.construir(base.pecas);
    });

    KDTree arvore(dim);
    arvore.construir(base.pecas);

    std::mt19937 gerador(SEMENTE);
    std::uniform_int_distribution<int> sorteio(0, n - 1);
    std::vector<int> consultas(CONSULTAS);
    for (int& c : consultas) c = sorteio(gerador);

    double busca_kd = medir_ms([&] {
        for (int c : consultas) arvore.knn(base.pecas[c].v, K_TEMPOS);
    }) / CONSULTAS;
    double busca_bruta = medir_ms([&] {
        for (int c : consultas) arvore.forca_bruta(base.pecas[c].v, K_TEMPOS);
    }) / CONSULTAS;
    double grafo_union_find = medir_ms([&] {
        GrafoSimilaridade grafo;
        grafo.construir(base.pecas, arvore, K_TEMPOS);
        grafo.grupos();
    });

    GrafoLooks grafo_looks;
    grafo_looks.construir(base);
    Recomendador rec(base, arvore, grafo_looks);
    double recomendacao = medir_ms([&] {
        for (int c : consultas) {
            rec.versao_c({c}, base.pecas[c].categoria, K_TEMPOS);
        }
    }) / CONSULTAS;

    saida << rotulo << ',' << construir << ',' << busca_kd << ',' << busca_bruta << ','
          << grafo_union_find << ',' << recomendacao << '\n';
}

}  // namespace

std::vector<Escondida> sortear_escondidas(const Base& base, const std::vector<int>& looks) {
    std::mt19937 gerador(SEMENTE);
    std::vector<Escondida> res;
    for (int l : looks) {
        if (base.looks[l].pecas.size() < 2) continue;
        std::uniform_int_distribution<int> sorteio(0, (int)base.looks[l].pecas.size() - 1);
        res.push_back({l, sorteio(gerador)});
    }
    return res;
}

int hit_em_k(const std::vector<Candidata>& lista, int alvo, int k) {
    int limite = std::min(k, (int)lista.size());
    for (int i = 0; i < limite; i++) {
        if (lista[i].indice == alvo) return 1;
    }
    return 0;
}

void avaliar_hit(const Base& base, const Recomendador& rec, const GrafoLooks& grafo, const std::string& pasta_saida) {
    const std::string nomes[N_VERSOES] = {"Popularidade", "A. So semelhanca", "B. So grafo", "C. Grafo + semelhanca"};
    const int valores_k[N_K] = {1, 5, 10};
    long long acertos[N_VERSOES][N_K] = {};

    std::vector<int> looks_teste;
    for (int l = 0; l < (int)base.looks.size(); l++) {
        if (!base.looks[l].treino) looks_teste.push_back(l);
    }

    long long total = 0;
    long long no_treino = 0;
    for (const Escondida& e : sortear_escondidas(base, looks_teste)) {
        const Look& look = base.looks[e.look];
        int escondida = look.pecas[e.pos];
        std::vector<int> resto = look.pecas;
        resto.erase(resto.begin() + e.pos);
        const std::string& cat = base.pecas[escondida].categoria;

        total++;
        if (grafo.num_looks(escondida) > 0) no_treino++;

        const std::vector<Candidata> listas[N_VERSOES] = {
            rec.popularidade(resto, cat, valores_k[N_K - 1]),
            rec.versao_a(resto, cat, valores_k[N_K - 1]),
            rec.versao_b(resto, cat, valores_k[N_K - 1]),
            rec.versao_c(resto, cat, valores_k[N_K - 1]),
        };
        for (int v = 0; v < N_VERSOES; v++) {
            for (int i = 0; i < N_K; i++) acertos[v][i] += hit_em_k(listas[v], escondida, valores_k[i]);
        }
    }

    std::filesystem::create_directories(pasta_saida);
    std::ofstream arquivo(pasta_saida + "/hit_k.csv");
    arquivo << "versao,hit@1,hit@5,hit@10\n";
    for (int v = 0; v < N_VERSOES; v++) {
        arquivo << nomes[v];
        std::cout << nomes[v];
        for (int i = 0; i < N_K; i++) {
            double taxa = total > 0 ? (double)acertos[v][i] / total : 0.0;
            arquivo << ',' << taxa;
            std::cout << "  hit@" << valores_k[i] << " = " << taxa;
        }
        arquivo << '\n';
        std::cout << '\n';
    }
    // o grafo nunca acerta uma peca que nao esta no treino, entao esse e o teto de B e C
    std::cout << "Looks de teste: " << total << ". Peca escondida presente no treino: " << no_treino << ".\n";
    if (!arquivo) std::cerr << "Erro ao gravar " << pasta_saida << "/hit_k.csv\n";
}

void medir_tempos(const std::string& pasta_dados, const std::string& pasta_saida) {
    std::filesystem::create_directories(pasta_saida);
    std::ofstream arquivo(pasta_saida + "/tempos.csv");
    arquivo << "pecas,construir_kdtree_ms,busca_kdtree_ms,busca_forca_bruta_ms,grafo_e_union_find_ms,recomendacao_c_ms\n";

    bool achou_subconjunto = false;
    for (std::string tamanho : {"1000", "10000", "100000"}) {
        const std::string pecas = pasta_dados + "/pecas_" + tamanho + ".csv";
        const std::string looks = pasta_dados + "/looks_" + tamanho + ".csv";
        if (!std::filesystem::exists(pecas)) continue;
        Base base;
        std::string erro;
        if (!ler_pecas(pecas, base, erro) || !ler_looks(looks, base, erro)) {
            std::cerr << erro << '\n';
            continue;
        }
        achou_subconjunto = true;
        medir_base(base, tamanho, arquivo);
    }
    if (!achou_subconjunto) {
        Base base;
        std::string erro;
        if (ler_base(pasta_dados, base, erro)) {
            medir_base(base, std::to_string(base.pecas.size()), arquivo);
        } else {
            std::cerr << erro << '\n';
        }
    }
    std::cout << "Tempos em " << pasta_saida << "/tempos.csv\n";
}
