#include "grupos_formas.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>

#include "atributos.hpp"
#include "avaliacao_justa.hpp"
#include "grafo_looks.hpp"
#include "grupos_kruskal.hpp"
#include "kdtree.hpp"
#include "pontos_atributos.hpp"
#include "recomendador.hpp"

namespace {

const std::vector<double> PESOS_ESTAMPA = {0.0, 0.1, 0.2};
const std::vector<std::string> NOMES_OCASIAO = {"dia a dia", "trabalho", "festa", "esporte", "praia"};
const std::string CAT_EXEMPLO = "top";
const std::vector<std::string> CATEGORIAS_GEPHI = {"top", "pants", "shoes"};

std::string num(double x, int casas) {
    std::ostringstream s;
    s << std::fixed << std::setprecision(casas) << x;
    std::string r = s.str();
    std::replace(r.begin(), r.end(), '.', ',');
    return r;
}

std::string pct(double x) { return num(100.0 * x, 1) + "%"; }

std::string estampa3(const std::string& e) {
    double v = valor_estampa(e);
    return v == 0.0 ? "lisa" : (v == 1.0 ? "estampada" : "desconhecido");
}

std::string cor_da_peca(const ValoresAtrib& val, int i) {
    const std::vector<int>& x = val.val[COR][i];
    return x.empty() ? "desconhecido" : val.nomes[COR][x[0]];
}

bool ler_tudo(const Base& base, const std::string& pasta, std::vector<Atrib>& attr, ValoresAtrib& val,
              std::string& erro) {
    return ler_atributos(pasta + "/pecas_v4.csv", base, attr, erro) && ler_valores(pasta + "/pecas_v5.csv", base, val, erro);
}

// forma 3: cor, estampa e faixa de formalidade. ocasiao fica de fora porque e marcada em
// varias ao mesmo tempo (32 combinacoes) e picaria demais os grupos
std::vector<std::string> chaves(const std::vector<Atrib>& attr, const ValoresAtrib& val) {
    std::vector<std::string> chave(attr.size());
    for (int i = 0; i < (int)attr.size(); i++) {
        double f = attr[i].g1.size() > 10 ? attr[i].g1[10] : 0.5;
        chave[i] = cor_da_peca(val, i) + "|" + estampa3(attr[i].estampa) + "|" + faixa_formalidade(f);
    }
    return chave;
}

struct Forma {
    std::string nome;
    bool usa_vetor = false;
    bool so_comparacao = false;
    Grupos gr;
};

struct Resumo {
    int grupos = 0;
    int unitarios = 0;
    double sozinhas = 0.0;      // fracao das pecas em grupo de 1
    double media_maior = 0.0;   // media entre categorias da fracao do maior grupo
    double pior = 0.0;
    std::string cat_pior;
    std::vector<double> maior_cat;
    std::vector<int> grupos_cat;
};

Resumo resumir(const GrafoKnn& g, const Grupos& gr) {
    Resumo r;
    int n_cat = (int)g.categorias.size();
    std::vector<int> maior(n_cat, 0);
    long long sozinhas = 0;
    long long total = 0;
    for (int x = 0; x < (int)gr.tamanho.size(); x++) {
        int c = gr.cat_do_grupo[x];
        maior[c] = std::max(maior[c], gr.tamanho[x]);
        if (gr.tamanho[x] == 1) {
            r.unitarios++;
            sozinhas++;
        }
        total += gr.tamanho[x];
    }
    r.grupos = (int)gr.tamanho.size();
    r.sozinhas = total > 0 ? (double)sozinhas / total : 0.0;
    r.grupos_cat = gr.por_categoria;
    for (int c = 0; c < n_cat; c++) {
        double f = g.membros[c].empty() ? 0.0 : (double)maior[c] / g.membros[c].size();
        r.maior_cat.push_back(f);
        r.media_maior += f / n_cat;
        if (f > r.pior) {
            r.pior = f;
            r.cat_pior = g.categorias[c];
        }
    }
    return r;
}

// pelo criterio do .hpp. devolve o indice da forma escolhida
int escolher(const std::vector<Forma>& formas, const std::vector<Resumo>& res) {
    int melhor = -1;
    for (int i = 0; i < (int)formas.size(); i++) {
        if (formas[i].so_comparacao || res[i].grupos == 0) continue;
        if ((double)res[i].unitarios / res[i].grupos > MAX_UNITARIOS) continue;
        if (melhor < 0 || res[i].media_maior < res[melhor].media_maior) melhor = i;
    }
    if (melhor >= 0) return melhor;
    for (int i = 0; i < (int)formas.size(); i++) {
        if (formas[i].so_comparacao) continue;
        double f = (double)res[i].unitarios / std::max(res[i].grupos, 1);
        if (melhor < 0 || f < (double)res[melhor].unitarios / std::max(res[melhor].grupos, 1)) melhor = i;
    }
    return melhor;
}

std::string descrever_peca(const Base& base, const std::vector<Atrib>& attr, const ValoresAtrib& val, int i) {
    const Atrib& a = attr[i];
    std::string oc;
    for (int j = 0; j < 5 && 5 + j < (int)a.g1.size(); j++) {
        if (a.g1[5 + j] > 0.5) oc += (oc.empty() ? "" : ", ") + NOMES_OCASIAO[j];
    }
    double f = a.g1.size() > 10 ? a.g1[10] : 0.5;
    std::string lab = a.lab.size() == 3 ? num(a.lab[0] * 100, 0) + "/" + num(a.lab[1] * 255 - 128, 0) + "/" +
                                              num(a.lab[2] * 255 - 128, 0)
                                        : "?";
    return "| " + std::to_string(base.pecas[i].id) + " | " + cor_da_peca(val, i) + " | " + lab + " | " + a.estampa +
           " | " + faixa_formalidade(f) + " | " + (oc.empty() ? "nenhuma" : oc) + " |";
}

// 3 grupos da categoria de exemplo: o maior, o do meio e o menor com 5 pecas ou mais
std::string exemplos(const Base& base, const GrafoKnn& g, const Grupos& gr, const std::vector<Atrib>& attr,
                     const ValoresAtrib& val) {
    auto it = std::find(g.categorias.begin(), g.categorias.end(), CAT_EXEMPLO);
    int c = it == g.categorias.end() ? 0 : (int)(it - g.categorias.begin());
    std::vector<int> grupos;
    for (int x = 0; x < (int)gr.tamanho.size(); x++) {
        if (gr.cat_do_grupo[x] == c) grupos.push_back(x);
    }
    if (grupos.empty()) return "";
    std::stable_sort(grupos.begin(), grupos.end(), [&](int x, int y) { return gr.tamanho[x] > gr.tamanho[y]; });
    std::vector<int> escolhidos = {grupos[0], grupos[grupos.size() / 2]};
    int menor = grupos.back();
    for (int x : grupos) {
        if (gr.tamanho[x] >= 5) menor = x;
    }
    escolhidos.push_back(menor);

    std::ostringstream s;
    const char* nome[] = {"maior grupo", "grupo do meio", "menor grupo com 5 peças ou mais"};
    for (int k = 0; k < 3; k++) {
        int x = escolhidos[k];
        s << "\n" << g.categorias[c] << ", " << nome[k] << " (" << gr.tamanho[x] << (gr.tamanho[x] == 1 ? " peça" : " peças") << "):\n\n"
          << "| id | grupo_cor | Lab | estampa | formalidade | ocasião |\n| --- | --- | --- | --- | --- | --- |\n";
        int mostradas = 0;
        for (int i : g.membros[c]) {
            if (gr.grupo[i] != x) continue;
            s << descrever_peca(base, attr, val, i) << "\n";
            if (++mostradas == 5) break;
        }
    }
    return s.str();
}

Grupos montar_forma(int forma, const GrafoKnn& g, const std::vector<std::string>& chave, int n) {
    if (forma == 0) return agrupar(g, K_FORMAS, n);
    if (forma == 1) return agrupar_limitado(g, K_FORMAS, FRACAO_LIMITE, n);
    if (forma == 2) return agrupar_mutuo(g, n);
    return agrupar_por_chave(g, chave, n);
}

void normalizar(std::vector<double>& x) {
    if (x.empty()) return;
    auto mm = std::minmax_element(x.begin(), x.end());
    double lo = *mm.first;
    double d = *mm.second - lo;
    for (double& v : x) v = d > 0 ? (v - lo) / d : 0.0;
}

double media_ordens(const Posicoes& p, int k) {
    double s = 0.0;
    for (int o = 0; o < N_ORDENS; o++) s += taxa(p, k, o);
    return s / N_ORDENS;
}

bool exportar(const Base& base, const GrafoKnn& g, const Grupos& gr, const std::string& prefixo) {
    std::ofstream nos(prefixo + "_nodes.csv");
    std::ofstream arestas(prefixo + "_edges.csv");
    nos << "Id,Label,Categoria,Grupo,Tamanho\n";
    arestas << "Source,Target,Weight,Type\n";
    for (int c = 0; c < (int)g.categorias.size(); c++) {
        if (std::find(CATEGORIAS_GEPHI.begin(), CATEGORIAS_GEPHI.end(), g.categorias[c]) == CATEGORIAS_GEPHI.end()) {
            continue;
        }
        const std::vector<int>& m = g.membros[c];
        for (int i : m) {
            const Peca& p = base.pecas[i];
            nos << p.id << ',' << p.id << ',' << p.categoria << ',' << gr.grupo[i] << ',' << gr.tamanho[gr.grupo[i]]
                << '\n';
        }
        // arestas da forma (floresta ou mutuas). sem elas, as mutuas dentro do mesmo grupo
        std::vector<ArestaK> lista = gr.floresta[c];
        if (lista.empty()) {
            for (const ArestaK& a : g.arestas[c]) {
                if (a.mutua && gr.grupo[m[a.u]] == gr.grupo[m[a.v]]) lista.push_back(a);
            }
        }
        for (const ArestaK& a : lista) {
            std::ostringstream peso;
            peso << std::fixed << std::setprecision(4) << 1.0 / (1.0 + 100.0 * a.peso);
            arestas << base.pecas[m[a.u]].id << ',' << base.pecas[m[a.v]].id << ',' << peso.str() << ",Undirected\n";
        }
    }
    return (bool)nos && (bool)arestas;
}

}  // namespace

bool comparar_formas(const Base& base, const std::string& pasta_dados, const std::string& pasta_saida) {
    std::vector<Atrib> attr;
    ValoresAtrib val;
    std::string erro;
    if (!ler_tudo(base, pasta_dados, attr, val, erro)) {
        std::cerr << erro << '\n';
        return false;
    }
    const int n = (int)base.pecas.size();
    const std::vector<std::string> chave = chaves(attr, val);

    GrafoKnn g = montar_grafo_knn(base, attr);
    std::vector<Forma> formas(4);
    formas[0].nome = "Kruskal puro (K = " + std::to_string(K_FORMAS) + ")";
    formas[0].usa_vetor = formas[0].so_comparacao = true;
    formas[1].nome = "1. Kruskal com limite de " + num(100 * FRACAO_LIMITE, 0) + "% (K = " + std::to_string(K_FORMAS) + ")";
    formas[1].usa_vetor = true;
    formas[2].nome = "2. kNN mútuo + Union-Find (k = " + std::to_string(K_VIZINHOS_GRUPOS) + ")";
    formas[2].usa_vetor = true;
    formas[3].nome = "3. Mesma combinação de grupo_cor, estampa e formalidade";
    std::vector<Resumo> res;
    for (int f = 0; f < 4; f++) {
        formas[f].gr = montar_forma(f, g, chave, n);
        res.push_back(resumir(g, formas[f].gr));
    }
    const int escolhida = escolher(formas, res);

    std::filesystem::create_directories(pasta_saida + "/gephi_grupos");
    std::ofstream md(pasta_saida + "/grupos_formas.md");
    md << "# Formas de agrupar as peças\n\n"
       << "Gerado por `./recomendador grupos_formas`. Nenhuma forma usa looks. Grupos sempre dentro de uma categoria.\n"
       << "As formas 1 e 2 usam o grafo kNN (k = " << K_VIZINHOS_GRUPOS << ") do vetor L, a, b e estampa com peso "
       << num(PESO_ESTAMPA, 1) << ".\n"
       << "A forma 1 para quando chega a " << K_FORMAS << " grupos ou quando nenhuma aresta une dois grupos sem passar "
       << "o limite. O limite é " << num(100 * FRACAO_LIMITE, 0) << "% das peças da categoria, e no mínimo 2.\n"
       << "A forma 3 não usa ocasião porque a peça pode ter várias ao mesmo tempo, e as combinações picariam os grupos.\n\n"
       << "Critério, escrito antes de rodar: fica a forma de menor média da fração do maior grupo (média entre as "
       << g.categorias.size() << " categorias), entre as que têm no máximo " << num(100 * MAX_UNITARIOS, 0)
       << "% dos grupos com 1 peça só. O Kruskal puro fica só como comparação.\n\n"
       << "| Forma | grupos | maior grupo (média das categorias) | pior categoria | grupos com 1 peça | peças sozinhas |\n"
       << "| --- | --- | --- | --- | --- | --- |\n";
    for (int f = 0; f < 4; f++) {
        const Resumo& r = res[f];
        md << "| " << formas[f].nome << (f == escolhida ? " **(escolhida)**" : "") << " | " << r.grupos << " | "
           << pct(r.media_maior) << " | " << r.cat_pior << " " << pct(r.pior) << " | " << r.unitarios << " ("
           << pct((double)r.unitarios / std::max(r.grupos, 1)) << ") | " << pct(r.sozinhas) << " |\n";
    }
    md << "\nNo Kruskal puro, as arestas mais curtas são unidas primeiro. Num grafo denso de cores parecidas, "
          "sempre existe uma aresta curta que liga o grupo grande a mais uma peça, então ele cresce como uma corrente. "
          "As peças de cor rara ficam por último e viram os outros grupos, quase todos pequenos.\n\n";

    md << "## Por categoria (grupos / maior grupo)\n\n| categoria | peças |";
    for (int f = 0; f < 4; f++) md << " forma " << (f == 0 ? std::string("pura") : std::to_string(f)) << " |";
    md << "\n| --- | --- | --- | --- | --- | --- |\n";
    for (int c = 0; c < (int)g.categorias.size(); c++) {
        md << "| " << g.categorias[c] << " | " << g.membros[c].size() << " |";
        for (int f = 0; f < 4; f++) md << " " << res[f].grupos_cat[c] << " / " << pct(res[f].maior_cat[c]) << " |";
        md << "\n";
    }

    md << "\n## Exemplos\n\nCinco peças (as de menor índice) de 3 grupos de " << CAT_EXEMPLO << " em cada forma.\n";
    for (int f = 0; f < 4; f++) md << "\n### " << formas[f].nome << "\n" << exemplos(base, g, formas[f].gr, attr, val);
    std::cout << "Forma escolhida: " << formas[escolhida].nome << std::endl;

    // peso da estampa so muda as formas que usam o vetor
    double peso = PESO_ESTAMPA;
    Grupos final_gr = formas[escolhida].gr;
    GrafoKnn final_g = g;
    if (!formas[escolhida].usa_vetor) {
        md << "\n## Peso da estampa\n\nA forma escolhida não usa o vetor, então o peso da estampa não se aplica.\n";
    } else {
        // validacao: grafo de looks, C e pontos sem os looks de validacao. o teste nao e montado
        const Base b = base_sem_validacao(base);
        const Consultas cons = consultas_validacao(base);
        KDTree arvore((int)b.pecas[0].v.size());
        arvore.construir(b.pecas);
        GrafoLooks grafo;
        grafo.construir(b);
        Recomendador rec(b, arvore, grafo);
        const int tudo = (int)b.pecas.size();
        Notas nota_c = de_lista(b, [&](const std::vector<int>& l, const std::string& cat) { return rec.versao_c(l, cat, tudo); });

        std::ofstream csv(pasta_saida + "/peso_estampa_grupos.csv");
        csv << "peso_estampa,hit1,hit5,hit10,fitb\n";
        md << "\n## Peso da estampa na validação\n\n"
           << "Nota = C normalizada + pontos_grupos normalizado (pesos 1, min-max em cada consulta). "
           << "pontos_grupos(c) = soma dos pontos de c com cada peça do look. " << cons.alvo.size()
           << " consultas de validação, média das " << N_ORDENS << " ordens. Fica o maior Hit@10; no empate, o menor peso.\n\n"
           << "| peso | grupos | Hit@1 | Hit@5 | Hit@10 | FITB | tempo (s) |\n| --- | --- | --- | --- | --- | --- | --- |\n";
        double melhor = -1.0;
        for (double w : PESOS_ESTAMPA) {
            auto ini = std::chrono::steady_clock::now();
            GrafoKnn gw = montar_grafo_knn(base, attr, K_VIZINHOS_GRUPOS, w);
            Grupos grw = montar_forma(escolhida, gw, chave, n);
            PontosGrupos pg(b, grw);
            Posicoes p = avaliar_notas(b, cons, "peso", [&](int q, const std::vector<int>& look,
                                                            const std::vector<int>& cand, std::vector<double>& nota) {
                nota_c(q, look, cand, nota);
                normalizar(nota);
                std::vector<double> pts(cand.size(), 0.0);
                for (std::size_t i = 0; i < cand.size(); i++) {
                    for (int x : look) pts[i] += pg.pontos(x, cand[i]);
                }
                normalizar(pts);
                for (std::size_t i = 0; i < cand.size(); i++) nota[i] += pts[i];
            });
            double seg = std::chrono::duration<double>(std::chrono::steady_clock::now() - ini).count();
            double h10 = media_ordens(p, 10);
            csv << w << ',' << media_ordens(p, 1) << ',' << media_ordens(p, 5) << ',' << h10 << ',' << media_ordens(p, 0) << '\n';
            md << "| " << num(w, 1) << " | " << grw.tamanho.size() << " | " << pct(media_ordens(p, 1)) << " | "
               << pct(media_ordens(p, 5)) << " | " << pct(h10) << " | " << pct(media_ordens(p, 0)) << " | " << num(seg, 0)
               << " |\n";
            std::cout << "peso " << w << ": Hit@10 = " << h10 << " (" << seg << " s)" << std::endl;
            if (h10 > melhor) {
                melhor = h10;
                peso = w;
                final_gr = grw;
                final_g = gw;
            }
        }
        md << "\nPeso escolhido: " << num(peso, 1) << ".\n";
    }

    const std::string prefixo = pasta_saida + "/gephi_grupos/escolhida";
    if (!exportar(base, final_g, final_gr, prefixo)) {
        std::cerr << "Não foi possível gravar " << prefixo << "_nodes.csv\n";
        return false;
    }
    md << "\nGephi da forma escolhida (top, pants e shoes): `gephi_grupos/escolhida_nodes.csv` e `_edges.csv`. "
          "A coluna Grupo serve para colorir (Partition).\n";
    std::cout << "Resumo em " << pasta_saida << "/grupos_formas.md" << std::endl;
    return (bool)md;
}

bool grupos_escolhidos(const Base& base, const std::string& pasta_dados, Grupos& gr, std::string& erro) {
    std::vector<Atrib> attr;
    ValoresAtrib val;
    if (!ler_tudo(base, pasta_dados, attr, val, erro)) return false;
    // k = 0: so separa as categorias, a forma 3 nao usa as arestas
    GrafoKnn g = montar_grafo_knn(base, attr, 0);
    gr = agrupar_por_chave(g, chaves(attr, val), (int)base.pecas.size());
    return true;
}
