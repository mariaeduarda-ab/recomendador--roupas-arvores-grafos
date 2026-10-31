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

double medir_ms(const std::function<void()>& tarefa) {
    double total = 0.0;
    for (int i = 0; i < REPETICOES; i++) {
        const auto inicio = std::chrono::steady_clock::now();
        tarefa();
        const auto fim = std::chrono::steady_clock::now();
        total += std::chrono::duration<double, std::milli>(fim - inicio).count();
    }
    return total / REPETICOES;
}

void medir_base(const Base& base, const std::string& rotulo, std::ostream& saida) {
    if (base.pecas.empty()) return;
    const int dim = static_cast<int>(base.pecas[0].v.size());
    const int n = static_cast<int>(base.pecas.size());

    const double construir = medir_ms([&] {
        KDTree arvore(dim);
        arvore.construir(base.pecas);
    });

    KDTree arvore(dim);
    arvore.construir(base.pecas);

    std::mt19937 gerador(SEMENTE);
    std::uniform_int_distribution<int> sorteio(0, n - 1);
    std::vector<int> consultas(CONSULTAS);
    for (int& consulta : consultas) consulta = sorteio(gerador);

    const double busca_kd = medir_ms([&] {
        for (const int c : consultas) arvore.knn(base.pecas[c].v, K_TEMPOS);
    }) / CONSULTAS;
    const double busca_bruta = medir_ms([&] {
        for (const int c : consultas) arvore.forca_bruta(base.pecas[c].v, K_TEMPOS);
    }) / CONSULTAS;
    const double grafo_union_find = medir_ms([&] {
        GrafoSimilaridade grafo;
        grafo.construir(base.pecas, arvore, K_TEMPOS);
        grafo.grupos();
    });

    GrafoLooks grafo_looks;
    grafo_looks.construir(base);
    const Recomendador recomendador(base, arvore, grafo_looks);
    const double recomendacao = medir_ms([&] {
        for (const int c : consultas) {
            recomendador.versao_c({c}, base.pecas[c].categoria, K_TEMPOS);
        }
    }) / CONSULTAS;

    saida << rotulo << ',' << construir << ',' << busca_kd << ',' << busca_bruta << ','
          << grafo_union_find << ',' << recomendacao << '\n';
}

}  // namespace

int hit_em_k(const std::vector<Candidata>& lista, int alvo, int k) {
    const int limite = std::min(k, static_cast<int>(lista.size()));
    for (int i = 0; i < limite; i++) {
        if (lista[i].indice == alvo) return 1;
    }
    return 0;
}

bool avaliar_hit(const Base& base, const Recomendador& recomendador, const std::string& pasta_saida) {
    using Versao = std::function<std::vector<Candidata>(const std::vector<int>&, const std::string&, int)>;
    const std::vector<std::pair<std::string, Versao>> versoes = {
        {"Popularidade", [&](const std::vector<int>&, const std::string& c, int n) { return recomendador.popularidade(c, n); }},
        {"A. So semelhanca", [&](const std::vector<int>& l, const std::string& c, int n) { return recomendador.versao_a(l, c, n); }},
        {"B. So grafo", [&](const std::vector<int>& l, const std::string& c, int n) { return recomendador.versao_b(l, c, n); }},
        {"C. Grafo + semelhanca", [&](const std::vector<int>& l, const std::string& c, int n) { return recomendador.versao_c(l, c, n); }},
    };
    const std::vector<int> valores_k = {1, 5, 10};
    std::vector<std::vector<long long>> acertos(versoes.size(), std::vector<long long>(valores_k.size(), 0));

    std::mt19937 gerador(SEMENTE);
    long long total = 0;
    bool houve_candidatas = false;
    for (const Look& look : base.looks) {
        if (look.treino || look.pecas.size() < 2) continue;
        std::uniform_int_distribution<int> sorteio(0, static_cast<int>(look.pecas.size()) - 1);
        const int posicao = sorteio(gerador);
        const int escondida = look.pecas[posicao];
        std::vector<int> restantes = look.pecas;
        restantes.erase(restantes.begin() + posicao);

        total++;
        for (std::size_t v = 0; v < versoes.size(); v++) {
            const std::vector<Candidata> lista =
                versoes[v].second(restantes, base.pecas[escondida].categoria, valores_k.back());
            if (!lista.empty()) houve_candidatas = true;
            for (std::size_t i = 0; i < valores_k.size(); i++) {
                acertos[v][i] += hit_em_k(lista, escondida, valores_k[i]);
            }
        }
    }

    std::filesystem::create_directories(pasta_saida);
    std::ofstream arquivo(pasta_saida + "/hit_k.csv");
    arquivo << "versao,hit@1,hit@5,hit@10\n";
    for (std::size_t v = 0; v < versoes.size(); v++) {
        arquivo << versoes[v].first;
        for (std::size_t i = 0; i < valores_k.size(); i++) {
            arquivo << ',' << (total > 0 ? static_cast<double>(acertos[v][i]) / total : 0.0);
        }
        arquivo << '\n';
    }
    std::cout << "Looks de teste avaliados: " << total << ". Tabela em " << pasta_saida << "/hit_k.csv\n";
    return houve_candidatas;
}

void medir_tempos(const std::string& pasta_dados, const std::string& pasta_saida) {
    std::filesystem::create_directories(pasta_saida);
    std::ofstream arquivo(pasta_saida + "/tempos.csv");
    arquivo << "pecas,construir_kdtree_ms,busca_kdtree_ms,busca_forca_bruta_ms,grafo_e_union_find_ms,recomendacao_c_ms\n";

    bool achou_subconjunto = false;
    for (const std::string tamanho : {"1000", "10000", "100000"}) {
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
