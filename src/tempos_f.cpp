#include "tempos_f.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <queue>
#include <random>

#include "grafo_looks.hpp"
#include "leitura.hpp"
#include "union_find.hpp"

namespace {

constexpr unsigned SEMENTE = 42;
constexpr int REPETICOES = 5;
constexpr int CONSULTAS = 1000;
constexpr int K = 10;

struct Medida {
    double media = 0.0;
    double desvio = 0.0;
};

// desvio amostral das 5 repeticoes
template <typename F>
Medida medir(F tarefa, int divisor = 1) {
    std::vector<double> t(REPETICOES);
    for (double& x : t) {
        auto inicio = std::chrono::steady_clock::now();
        tarefa();
        auto fim = std::chrono::steady_clock::now();
        x = std::chrono::duration<double, std::milli>(fim - inicio).count() / divisor;
    }
    Medida m;
    for (double x : t) m.media += x;
    m.media /= REPETICOES;
    for (double x : t) m.desvio += (x - m.media) * (x - m.media);
    m.desvio = std::sqrt(m.desvio / (REPETICOES - 1));
    return m;
}

void gravar(std::ofstream& arq, int n, const std::string& medida, Medida m) {
    arq << n << ',' << medida << ',' << m.media << ',' << m.desvio << '\n';
    std::cout << "  " << medida << ": " << m.media << " ms (desvio " << m.desvio << ")\n";
}

std::string carga() {
    std::ifstream f("/proc/loadavg");
    std::string a, b, c;
    if (!(f >> a >> b >> c)) return "?,?,?";
    return a + ',' + b + ',' + c;
}

// une 0 com 1, depois 0 com 2 e assim por diante. sem tamanho, a raiz antiga
// vira filha da peca nova e o 0 fica no fim de uma corrente que so cresce.
// depois faz n buscas sorteadas. a conta no fim e igual em toda variante
template <typename UF>
long long sequencia_uf(UF& uf, int n, const std::vector<int>& buscas) {
    long long conta = 0;
    for (int i = 1; i < n; i++) conta += uf.unite(0, i);
    for (int x : buscas) conta += uf.find(x) == uf.find(0);
    return conta;
}

std::vector<int> sortear(int n, int quantos, unsigned semente) {
    std::mt19937 gerador(semente);
    std::uniform_int_distribution<int> sorteio(0, n - 1);
    std::vector<int> res(quantos);
    for (int& x : res) x = sorteio(gerador);
    return res;
}

}  // namespace

UFVariante::UFVariante(int n, bool por_tamanho, bool compressao)
    : pai(n), tam(n, 1), por_tamanho_(por_tamanho), compressao_(compressao) {
    for (int i = 0; i < n; i++) pai[i] = i;
}

int UFVariante::find(int x) {
    int raiz = x;
    while (pai[raiz] != raiz) raiz = pai[raiz];
    if (compressao_) {
        while (pai[x] != raiz) {
            int prox = pai[x];
            pai[x] = raiz;
            x = prox;
        }
    }
    return raiz;
}

bool UFVariante::unite(int a, int b) {
    int ra = find(a);
    int rb = find(b);
    if (ra == rb) return false;
    // sem tamanho, a primeira raiz sempre vai para baixo da segunda
    if (por_tamanho_ && tam[ra] > tam[rb]) std::swap(ra, rb);
    pai[ra] = rb;
    tam[rb] += tam[ra];
    return true;
}

std::vector<ArestaAGM> grafo_knn_agm(const std::vector<Peca>& pecas, const KDTree& arvore, int k) {
    std::vector<ArestaAGM> arestas;
    for (int i = 0; i < (int)pecas.size(); i++) {
        for (const Vizinho& viz : arvore.knn(pecas[i].v, k, nullptr, i)) {
            int a = std::min(i, viz.indice);
            int b = std::max(i, viz.indice);
            arestas.push_back({a, b, std::sqrt(viz.dist2)});
        }
    }
    std::sort(arestas.begin(), arestas.end(), [](const ArestaAGM& x, const ArestaAGM& y) {
        return x.a != y.a ? x.a < y.a : x.b < y.b;
    });
    arestas.erase(std::unique(arestas.begin(), arestas.end(),
                              [](const ArestaAGM& x, const ArestaAGM& y) { return x.a == y.a && x.b == y.b; }),
                  arestas.end());
    return arestas;
}

void ordenar_arestas_agm(std::vector<ArestaAGM>& arestas) {
    // desempate pelos vertices pra floresta sair igual em toda execucao
    std::sort(arestas.begin(), arestas.end(), [](const ArestaAGM& x, const ArestaAGM& y) {
        if (x.peso != y.peso) return x.peso < y.peso;
        return x.a != y.a ? x.a < y.a : x.b < y.b;
    });
}

Floresta agm_kruskal(int n, const std::vector<ArestaAGM>& ordenadas) {
    UnionFind uf(n);
    Floresta f;
    for (const ArestaAGM& e : ordenadas) {
        if (f.arestas == n - 1) break;  // ja virou uma arvore so
        if (uf.unite(e.a, e.b)) {
            f.peso += e.peso;
            f.arestas++;
        }
    }
    return f;
}

ListaAdj lista_adj_agm(int n, const std::vector<ArestaAGM>& arestas) {
    ListaAdj adj(n);
    for (const ArestaAGM& e : arestas) {
        adj[e.a].push_back({e.b, e.peso});
        adj[e.b].push_back({e.a, e.peso});
    }
    return adj;
}

Floresta agm_prim(const ListaAdj& adj) {
    int n = (int)adj.size();
    std::vector<char> dentro(n, 0);
    // heap de minimo. versao preguicosa: entrada velha so e jogada fora quando sai
    std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, std::greater<>> heap;
    Floresta f;
    for (int inicio = 0; inicio < n; inicio++) {
        if (dentro[inicio]) continue;
        heap.push({0.0, inicio});
        bool raiz = true;  // a raiz de cada componente nao traz aresta
        while (!heap.empty()) {
            auto [peso, u] = heap.top();
            heap.pop();
            if (dentro[u]) continue;
            dentro[u] = 1;
            if (!raiz) {
                f.peso += peso;
                f.arestas++;
            }
            raiz = false;
            for (const auto& [v, w] : adj[u]) {
                if (!dentro[v]) heap.push({w, v});
            }
        }
    }
    return f;
}

bool medir_tempos_f(const std::string& pasta_dados, const std::string& pasta_saida) {
    const std::string pasta = pasta_saida + "/tempos_f";
    std::filesystem::create_directories(pasta);
    std::ofstream kd(pasta + "/kdtree.csv");
    std::ofstream uf(pasta + "/union_find.csv");
    std::ofstream agm(pasta + "/agm.csv");
    std::ofstream conf(pasta + "/agm_conferencia.csv");
    std::ofstream bfs(pasta + "/bfs.csv");
    std::ofstream cg(pasta + "/carga.csv");
    for (std::ofstream* a : {&kd, &uf, &agm, &bfs}) *a << "n,medida,media_ms,desvio_ms\n";
    conf << "n,arestas_grafo,peso_kruskal,peso_prim,arestas_kruskal,arestas_prim,componentes\n";
    cg << "momento,carga_1min,carga_5min,carga_15min\n";
    cg << "inicio," << carga() << '\n';

    bool achou = false;
    long long lixo = 0;  // usa os resultados pro compilador nao apagar as chamadas
    for (std::string tamanho : {"1000", "10000", "100000"}) {
        const std::string pecas = pasta_dados + "/pecas_" + tamanho + ".csv";
        const std::string looks = pasta_dados + "/looks_" + tamanho + ".csv";
        Base base;
        std::string erro;
        if (!std::filesystem::exists(pecas) || !ler_pecas(pecas, base, erro) || !ler_looks(looks, base, erro)) {
            std::cerr << "Pulando " << tamanho << ": " << (erro.empty() ? "sem " + pecas : erro) << '\n';
            continue;
        }
        int n = (int)base.pecas.size();
        if (n < 2) continue;
        achou = true;
        int dim = (int)base.pecas[0].v.size();
        std::cout << "n = " << n << '\n';

        // KD-tree e kNN. consultas sao pecas sorteadas da propria base
        gravar(kd, n, "construir_kdtree", medir([&] {
                   KDTree t(dim);
                   t.construir(base.pecas);
               }));
        KDTree arvore(dim);
        arvore.construir(base.pecas);
        std::vector<int> consultas = sortear(n, CONSULTAS, SEMENTE);
        gravar(kd, n, "knn_kdtree", medir([&] {
                   for (int c : consultas) lixo += arvore.knn(base.pecas[c].v, K)[0].indice;
               }, CONSULTAS));
        gravar(kd, n, "knn_forca_bruta", medir([&] {
                   for (int c : consultas) lixo += arvore.forca_bruta(base.pecas[c].v, K)[0].indice;
               }, CONSULTAS));

        // Union-Find: mesma sequencia nas 4 variantes e na classe do projeto
        std::vector<int> buscas = sortear(n, n, SEMENTE + 1);
        const std::pair<const char*, std::pair<bool, bool>> variantes[] = {
            {"uf_sem_tecnicas", {false, false}},
            {"uf_so_tamanho", {true, false}},
            {"uf_so_compressao", {false, true}},
            {"uf_tamanho_e_compressao", {true, true}},
        };
        for (const auto& [nome, liga] : variantes) {
            gravar(uf, n, nome, medir([&] {
                       UFVariante u(n, liga.first, liga.second);
                       lixo += sequencia_uf(u, n, buscas);
                   }));
        }
        gravar(uf, n, "uf_projeto_posto_e_compressao", medir([&] {
                   UnionFind u(n);
                   lixo += sequencia_uf(u, n, buscas);
               }));

        // Kruskal e Prim no grafo kNN de todas as pecas
        gravar(agm, n, "montar_grafo_knn", medir([&] { lixo += grafo_knn_agm(base.pecas, arvore, K).size(); }));
        const std::vector<ArestaAGM> arestas = grafo_knn_agm(base.pecas, arvore, K);
        std::vector<ArestaAGM> ordenadas;
        // copia fora do relogio: so a ordenacao conta
        std::vector<double> t_ord(REPETICOES);
        for (double& x : t_ord) {
            ordenadas = arestas;
            auto inicio = std::chrono::steady_clock::now();
            ordenar_arestas_agm(ordenadas);
            x = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - inicio).count();
        }
        Medida ord;
        for (double x : t_ord) ord.media += x / REPETICOES;
        for (double x : t_ord) ord.desvio += (x - ord.media) * (x - ord.media) / (REPETICOES - 1);
        ord.desvio = std::sqrt(ord.desvio);
        gravar(agm, n, "kruskal_ordenar_arestas", ord);
        Floresta fk;
        gravar(agm, n, "kruskal_uniao", medir([&] { fk = agm_kruskal(n, ordenadas); }));
        ListaAdj adj;
        gravar(agm, n, "prim_lista_adj", medir([&] { adj = lista_adj_agm(n, arestas); }));
        Floresta fp;
        gravar(agm, n, "prim_heap", medir([&] { fp = agm_prim(adj); }));
        conf << n << ',' << arestas.size() << ',' << fk.peso << ',' << fp.peso << ',' << fk.arestas << ','
             << fp.arestas << ',' << n - fk.arestas << '\n';
        double dif = std::fabs(fk.peso - fp.peso);
        bool igual = fk.arestas == fp.arestas && dif <= 1e-9 * std::max(1.0, fk.peso);
        std::cout << "  floresta: Kruskal " << fk.peso << " e Prim " << fp.peso << ", " << fk.arestas << " arestas"
                  << (igual ? " (mesmo peso)" : " (PESOS DIFERENTES)") << '\n';

        // BFS de 2 passos so a partir de pecas que estao em algum look de treino
        GrafoLooks grafo;
        grafo.construir(base);
        std::vector<int> com_look;
        for (int p = 0; p < n; p++) {
            if (grafo.num_looks(p) > 0) com_look.push_back(p);
        }
        if (com_look.empty()) {
            std::cerr << "  sem peca em look de treino, BFS fica de fora\n";
        } else {
            std::vector<int> origens = sortear((int)com_look.size(), CONSULTAS, SEMENTE + 2);
            for (int& o : origens) o = com_look[o];
            gravar(bfs, n, "bfs_2_passos", medir([&] {
                       for (int o : origens) lixo += grafo.bfs_2_passos(o).size();
                   }, CONSULTAS));
        }
    }
    cg << "fim," << carga() << '\n';
    std::cout << "Conta de controle: " << lixo << '\n';
    if (!achou) {
        std::cerr << "Nenhum pecas_N.csv em " << pasta_dados << ".\n";
        return false;
    }
    std::cout << "Tempos em " << pasta << "/\n";
    return true;
}
