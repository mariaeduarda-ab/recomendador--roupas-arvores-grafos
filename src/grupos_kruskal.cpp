#include "grupos_kruskal.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <sstream>

#include "avaliacao_d.hpp"
#include "kdtree.hpp"

namespace {

constexpr unsigned SEMENTE_SORTEIO = 7;
const std::vector<std::string> CATEGORIAS_GEPHI = {"top", "pants", "shoes"};

double segundos_desde(std::chrono::steady_clock::time_point ini) {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - ini).count();
}

std::string num(double x, int casas) {
    std::ostringstream s;
    s << std::fixed << std::setprecision(casas) << x;
    return s.str();
}

}  // namespace

double valor_estampa(const std::string& estampa) {
    if (estampa == "lisa") return 0.0;
    if (estampa == "desconhecido" || estampa.empty()) return 0.5;
    return 1.0;
}

std::vector<double> vetor_grupos(const Atrib& a, double peso_estampa) {
    // o csv guarda L/100 e (a+128)/255. vezes 2,55 o a e o b voltam pra mesma escala do L,
    // e a distancia fica o delta E de verdade dividido por 100
    std::vector<double> v(4, 0.0);
    if (a.lab.size() == 3) {
        v[0] = a.lab[0];
        v[1] = a.lab[1] * 2.55;
        v[2] = a.lab[2] * 2.55;
    }
    v[3] = peso_estampa * valor_estampa(a.estampa);
    return v;
}

int kruskal(int n, const std::vector<ArestaK>& arestas, int alvo, UnionFind& uf, std::vector<ArestaK>* usadas) {
    int grupos = n;
    alvo = std::max(alvo, 1);
    for (const ArestaK& a : arestas) {
        // parar antes e o mesmo que nao fazer as ultimas alvo - 1 unioes da arvore
        if (grupos <= alvo) break;
        if (uf.unite(a.u, a.v)) {
            grupos--;
            if (usadas) usadas->push_back(a);
        }
    }
    return grupos;
}

GrafoKnn montar_grafo_knn(const Base& base, const std::vector<Atrib>& attr, int k, double peso_estampa) {
    GrafoKnn g;
    std::map<std::string, int> indice;
    for (const Peca& p : base.pecas) indice.emplace(p.categoria, 0);
    for (auto& par : indice) {
        par.second = (int)g.categorias.size();
        g.categorias.push_back(par.first);
    }
    g.membros.assign(g.categorias.size(), {});
    for (int i = 0; i < (int)base.pecas.size(); i++) g.membros[indice[base.pecas[i].categoria]].push_back(i);
    g.arestas.assign(g.categorias.size(), {});
    g.componentes.assign(g.categorias.size(), 0);

    for (int c = 0; c < (int)g.categorias.size(); c++) {
        const std::vector<int>& m = g.membros[c];
        int n = (int)m.size();
        // uma arvore por categoria. a arvore da base toda com filtro visitaria as outras a toa
        std::vector<Peca> locais(n);
        for (int i = 0; i < n; i++) locais[i].v = vetor_grupos(attr[m[i]], peso_estampa);
        KDTree arvore(4);
        arvore.construir(locais);

        std::vector<ArestaK>& lista = g.arestas[c];
        for (int i = 0; i < n; i++) {
            for (const Vizinho& viz : arvore.knn(locais[i].v, k, nullptr, i)) {
                ArestaK a;
                a.u = std::min(i, viz.indice);
                a.v = std::max(i, viz.indice);
                a.peso = std::sqrt(viz.dist2);
                lista.push_back(a);
            }
        }
        // i perto de j e j perto de i viram a mesma aresta
        std::sort(lista.begin(), lista.end(), [](const ArestaK& x, const ArestaK& y) {
            return x.u != y.u ? x.u < y.u : x.v < y.v;
        });
        lista.erase(std::unique(lista.begin(), lista.end(),
                                [](const ArestaK& x, const ArestaK& y) { return x.u == y.u && x.v == y.v; }),
                    lista.end());
        // empate no peso desempata pelos indices pra sair igual em toda execucao
        std::sort(lista.begin(), lista.end(), [](const ArestaK& x, const ArestaK& y) {
            if (x.peso != y.peso) return x.peso < y.peso;
            return x.u != y.u ? x.u < y.u : x.v < y.v;
        });
        UnionFind uf(n);
        g.componentes[c] = kruskal(n, lista, 1, uf);
    }
    return g;
}

Grupos agrupar(const GrafoKnn& grafo, int alvo, int n_pecas) {
    Grupos r;
    r.grupo.assign(n_pecas, -1);
    r.por_categoria.assign(grafo.categorias.size(), 0);
    r.floresta.assign(grafo.categorias.size(), {});
    for (int c = 0; c < (int)grafo.categorias.size(); c++) {
        const std::vector<int>& m = grafo.membros[c];
        int n = (int)m.size();
        UnionFind uf(n);
        r.por_categoria[c] = kruskal(n, grafo.arestas[c], alvo, uf, &r.floresta[c]);
        std::vector<int> id_da_raiz(n, -1);
        for (int i = 0; i < n; i++) {
            int raiz = uf.find(i);
            if (id_da_raiz[raiz] < 0) {
                id_da_raiz[raiz] = (int)r.tamanho.size();
                r.tamanho.push_back(0);
                r.cat_do_grupo.push_back(c);
            }
            r.grupo[m[i]] = id_da_raiz[raiz];
            r.tamanho[id_da_raiz[raiz]]++;
        }
    }
    return r;
}

PontosGrupos::PontosGrupos(const Base& base, const Grupos& grupos)
    : grupo_(grupos.grupo), cat_(grupos.cat_do_grupo) {
    for (int c : cat_) n_cat_ = std::max(n_cat_, c + 1);
    std::vector<int> pecas_cat(n_cat_, 0);
    for (int g = 0; g < (int)cat_.size(); g++) pecas_cat[cat_[g]] += grupos.tamanho[g];
    fracao_.assign(cat_.size(), 0.0);
    for (int g = 0; g < (int)cat_.size(); g++) {
        if (pecas_cat[cat_[g]] > 0) fracao_[g] = (double)grupos.tamanho[g] / pecas_cat[cat_[g]];
    }

    pares_cat_.assign((std::size_t)n_cat_ * n_cat_, 0.0);
    const long long total = (long long)cat_.size();
    for (const Look& look : base.looks) {
        if (!look.treino) continue;
        for (std::size_t i = 0; i < look.pecas.size(); i++) {
            for (std::size_t j = i + 1; j < look.pecas.size(); j++) {
                int g = grupo_[look.pecas[i]];
                int h = grupo_[look.pecas[j]];
                int cg = cat_[g];
                int ch = cat_[h];
                // dois brincos no mesmo look nao dizem nada sobre combinar categorias
                if (cg == ch) continue;
                pares_cat_[(std::size_t)cg * n_cat_ + ch]++;
                pares_cat_[(std::size_t)ch * n_cat_ + cg]++;
                n_[(long long)std::min(g, h) * total + std::max(g, h)]++;
            }
        }
    }
}

int PontosGrupos::contagem(int g, int h) const {
    auto it = n_.find((long long)std::min(g, h) * (long long)cat_.size() + std::max(g, h));
    return it == n_.end() ? 0 : it->second;
}

double PontosGrupos::esperado(int g, int h) const {
    return pares_cat_[(std::size_t)cat_[g] * n_cat_ + cat_[h]] * fracao_[g] * fracao_[h];
}

double PontosGrupos::pontos_entre(int g, int h) const {
    if (g < 0 || h < 0 || cat_[g] == cat_[h]) return 0.0;
    // o +1 nos dois lados evita log de 0 e puxa pra 0 os pares com pouca conta
    return std::log((contagem(g, h) + 1.0) / (esperado(g, h) + 1.0));
}

double PontosGrupos::pontos(int p, int c) const { return pontos_entre(grupo_[p], grupo_[c]); }

namespace {

// media do vetor e fracao com estampa de um grupo, pra descrever os pares no resumo
struct Descricao {
    double L = 0, a = 0, b = 0, estampa = 0;
};

std::vector<Descricao> descrever(const Grupos& gr, const std::vector<Atrib>& attr) {
    std::vector<Descricao> d(gr.tamanho.size());
    for (int i = 0; i < (int)gr.grupo.size(); i++) {
        Descricao& x = d[gr.grupo[i]];
        if (attr[i].lab.size() == 3) {
            x.L += attr[i].lab[0] * 100.0;
            x.a += attr[i].lab[1] * 255.0 - 128.0;
            x.b += attr[i].lab[2] * 255.0 - 128.0;
        }
        x.estampa += valor_estampa(attr[i].estampa) == 1.0 ? 1.0 : 0.0;
    }
    for (int g = 0; g < (int)d.size(); g++) {
        double t = std::max(gr.tamanho[g], 1);
        d[g].L /= t;
        d[g].a /= t;
        d[g].b /= t;
        d[g].estampa /= t;
    }
    return d;
}

std::string texto_grupo(const GrafoKnn& g, const Grupos& gr, const std::vector<Descricao>& d, int x) {
    return g.categorias[gr.cat_do_grupo[x]] + " (" + std::to_string(gr.tamanho[x]) + " peças, Lab " +
           num(d[x].L, 0) + "/" + num(d[x].a, 0) + "/" + num(d[x].b, 0) + ", " + num(100 * d[x].estampa, 0) +
           "% com estampa)";
}

// pontos do par certo contra o par com uma peca sorteada da mesma categoria, nos looks de validacao.
// so diagnostico: nao escolhe nada
void comparar_na_validacao(const Base& base, const GrafoKnn& g, const Grupos& gr, double& certo,
                           double& sorteado, long long& pares) {
    std::vector<int> val = sortear_validacao(base);
    Base sem_val = base;
    for (int l : val) sem_val.looks[l].treino = false;
    PontosGrupos pg(sem_val, gr);

    std::vector<int> cat_peca(base.pecas.size(), 0);
    for (int c = 0; c < (int)g.membros.size(); c++) {
        for (int i : g.membros[c]) cat_peca[i] = c;
    }
    std::mt19937 gerador(SEMENTE_SORTEIO);
    certo = sorteado = 0.0;
    pares = 0;
    for (int l : val) {
        const std::vector<int>& p = base.looks[l].pecas;
        for (std::size_t i = 0; i < p.size(); i++) {
            for (std::size_t j = 0; j < p.size(); j++) {
                if (i == j || cat_peca[p[i]] == cat_peca[p[j]]) continue;
                const std::vector<int>& m = g.membros[cat_peca[p[j]]];
                int outra = m[std::uniform_int_distribution<int>(0, (int)m.size() - 1)(gerador)];
                certo += pg.pontos(p[i], p[j]);
                sorteado += pg.pontos(p[i], outra);
                pares++;
            }
        }
    }
    if (pares > 0) {
        certo /= pares;
        sorteado /= pares;
    }
}

bool exportar_gephi_grupos(const Base& base, const GrafoKnn& g, const Grupos& gr, const std::string& prefixo) {
    std::ofstream nos(prefixo + "_nodes.csv");
    std::ofstream arestas(prefixo + "_edges.csv");
    nos << "Id,Label,Categoria,Grupo\n";
    arestas << "Source,Target,Weight,Type\n";
    for (int c = 0; c < (int)g.categorias.size(); c++) {
        if (std::find(CATEGORIAS_GEPHI.begin(), CATEGORIAS_GEPHI.end(), g.categorias[c]) == CATEGORIAS_GEPHI.end()) {
            continue;
        }
        const std::vector<int>& m = g.membros[c];
        for (int i : m) {
            const Peca& p = base.pecas[i];
            nos << p.id << ',' << p.id << ',' << p.categoria << ',' << gr.grupo[i] << '\n';
        }
        // so as arestas da floresta: cada grupo aparece como uma arvore
        for (const ArestaK& a : gr.floresta[c]) {
            // no gephi peso maior puxa mais, entao vai o inverso da distancia
            arestas << base.pecas[m[a.u]].id << ',' << base.pecas[m[a.v]].id << ',' << num(1.0 / (1.0 + 100.0 * a.peso), 4)
                    << ",Undirected\n";
        }
    }
    return (bool)nos && (bool)arestas;
}

}  // namespace

bool rodar_grupos(const Base& base, const std::string& pasta_dados, const std::vector<int>& lista_k,
                  const std::string& pasta_saida) {
    std::vector<Atrib> attr;
    std::string erro;
    const std::string v4 = pasta_dados + "/pecas_v4.csv";
    bool ok = std::filesystem::exists(v4) ? ler_atributos(v4, base, attr, erro)
                                           : ler_atributos_da_pasta(pasta_dados, base, attr, erro);
    if (!ok) {
        std::cerr << erro << '\n';
        return false;
    }
    std::filesystem::create_directories(pasta_saida + "/gephi_grupos");

    auto t0 = std::chrono::steady_clock::now();
    GrafoKnn g = montar_grafo_knn(base, attr);
    double t_grafo = segundos_desde(t0);
    long long n_arestas = 0;
    for (const auto& l : g.arestas) n_arestas += (long long)l.size();
    std::cout << "Grafo kNN (k = " << K_VIZINHOS_GRUPOS << "): " << n_arestas << " arestas em " << num(t_grafo, 2)
              << " s\n";

    std::ofstream csv(pasta_saida + "/grupos_kruskal.csv");
    csv << "K,categoria,pecas,componentes_knn,grupos,maior,fracao_maior,corrente\n";
    std::ofstream md(pasta_saida + "/grupos_kruskal.md");
    md << "# Grupos com Kruskal\n\n"
       << "Gerado por `./recomendador grupos`. Vetor de cada peça: L, a, b (delta E / 100) e estampa com peso "
       << num(PESO_ESTAMPA, 2) << " (lisa 0, com estampa 1, desconhecido 0,5).\n"
       << "Na descrição dos grupos, desconhecido conta como sem estampa.\n"
       << "Grafo kNN com k = " << K_VIZINHOS_GRUPOS << " dentro de cada categoria: " << n_arestas
       << " arestas, montado em " << num(t_grafo, 2) << " s.\n\n"
       << "Corrente = categoria com um grupo que tem mais da metade das peças.\n\n"
       << "| K | grupos no total | categorias que não chegaram a K | maior grupo | categorias com corrente | "
          "Kruskal (s) | pontos (s) |\n"
       << "| --- | --- | --- | --- | --- | --- | --- |\n";

    std::ostringstream detalhes;
    for (int alvo : lista_k) {
        auto t1 = std::chrono::steady_clock::now();
        Grupos gr = agrupar(g, alvo, (int)base.pecas.size());
        double t_kruskal = segundos_desde(t1);
        auto t2 = std::chrono::steady_clock::now();
        PontosGrupos pg(base, gr);
        double t_pontos = segundos_desde(t2);

        std::vector<int> maior(g.categorias.size(), 0);
        for (int x = 0; x < (int)gr.tamanho.size(); x++) {
            maior[gr.cat_do_grupo[x]] = std::max(maior[gr.cat_do_grupo[x]], gr.tamanho[x]);
        }
        int sem_k = 0;
        int correntes = 0;
        int cat_maior = 0;
        std::string lista_corrente;
        for (int c = 0; c < (int)g.categorias.size(); c++) {
            int n = (int)g.membros[c].size();
            bool corrente = 2 * maior[c] > n && n > 1;
            if (gr.por_categoria[c] > alvo) sem_k++;
            if (corrente) {
                correntes++;
                lista_corrente += (lista_corrente.empty() ? "" : ", ") + g.categorias[c] + " " +
                                  num(100.0 * maior[c] / n, 0) + "%";
            }
            if (maior[c] > maior[cat_maior]) cat_maior = c;
            csv << alvo << ',' << g.categorias[c] << ',' << n << ',' << g.componentes[c] << ',' << gr.por_categoria[c]
                << ',' << maior[c] << ',' << num(n > 0 ? (double)maior[c] / n : 0.0, 4) << ',' << corrente << '\n';
        }
        md << "| " << alvo << " | " << gr.tamanho.size() << " | " << sem_k << " | " << maior[cat_maior] << " ("
           << g.categorias[cat_maior] << ", " << num(100.0 * maior[cat_maior] / g.membros[cat_maior].size(), 0)
           << "%) | " << correntes << " de " << g.categorias.size() << " | " << num(t_kruskal, 3) << " | "
           << num(t_pontos, 3) << " |\n";

        // os 10 pares de grupos com mais pontos, so entre pares que aparecem pelo menos 20 vezes
        std::vector<Descricao> d = descrever(gr, attr);
        std::vector<std::pair<double, std::pair<int, int>>> pares;
        int n_grupos = (int)gr.tamanho.size();
        for (int x = 0; x < n_grupos; x++) {
            for (int y = x + 1; y < n_grupos; y++) {
                if (gr.cat_do_grupo[x] == gr.cat_do_grupo[y] || pg.contagem(x, y) < 20) continue;
                pares.push_back({pg.pontos_entre(x, y), {x, y}});
            }
        }
        std::sort(pares.rbegin(), pares.rend());
        double certo = 0.0, sorteado = 0.0;
        long long n_val = 0;
        comparar_na_validacao(base, g, gr, certo, sorteado, n_val);

        detalhes << "\n## K = " << alvo << "\n\n";
        if (!lista_corrente.empty()) detalhes << "Categorias com corrente: " << lista_corrente << ".\n\n";
        detalhes << "Pares de peças nos looks de validação (pontos contados sem eles): média "
                 << num(certo, 3) << " no par certo e " << num(sorteado, 3)
                 << " quando a segunda peça é sorteada da mesma categoria (" << n_val << " pares).\n\n"
                 << "Pares de grupos com mais pontos (contagem de pelo menos 20 pares):\n\n"
                 << "| grupo 1 | grupo 2 | n | esperado | pontos |\n| --- | --- | --- | --- | --- |\n";
        for (int i = 0; i < (int)pares.size() && i < 10; i++) {
            int x = pares[i].second.first;
            int y = pares[i].second.second;
            detalhes << "| " << texto_grupo(g, gr, d, x) << " | " << texto_grupo(g, gr, d, y) << " | "
                     << pg.contagem(x, y) << " | " << num(pg.esperado(x, y), 1) << " | " << num(pares[i].first, 2)
                     << " |\n";
        }

        const std::string prefixo = pasta_saida + "/gephi_grupos/k" + std::to_string(alvo);
        if (!exportar_gephi_grupos(base, g, gr, prefixo)) {
            std::cerr << "Não foi possível gravar " << prefixo << "_nodes.csv\n";
            return false;
        }
        std::cout << "K = " << alvo << ": " << gr.tamanho.size() << " grupos, " << correntes
                  << " categorias com corrente, Kruskal em " << num(t_kruskal, 3) << " s, pontos em "
                  << num(t_pontos, 3) << " s\n";
    }
    md << detalhes.str();
    md << "\nArquivos do Gephi (top, pants e shoes; arestas = floresta do Kruskal): "
          "`gephi_grupos/k<K>_nodes.csv` e `_edges.csv`.\n";
    std::cout << "Resumo em " << pasta_saida << "/grupos_kruskal.md\n";
    return (bool)csv && (bool)md;
}
