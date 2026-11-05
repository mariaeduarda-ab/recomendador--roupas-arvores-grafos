#include "articulacoes.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>

#include "atributos.hpp"
#include "grupos_formas.hpp"
#include "grupos_kruskal.hpp"
#include "pontos_atributos.hpp"
#include "tarjan.hpp"

namespace {

const std::vector<std::string> NOMES_LIMIAR = {"q1", "q2", "q3"};
const std::vector<std::string> TEXTO_LIMIAR = {"1º quartil", "mediana", "3º quartil"};
const int PRINCIPAL = 1;  // a mediana ganha a tabela completa
const int N_EXEMPLOS = 3;

std::string num(double x, int casas) {
    std::ostringstream s;
    s << std::fixed << std::setprecision(casas) << x;
    std::string r = s.str();
    std::replace(r.begin(), r.end(), '.', ',');
    return r;
}

struct Desc {
    std::string cat, cor, estampa, formal;
};

std::string nome(const Desc& d) { return d.cat + " " + d.cor + " " + d.estampa + " formalidade " + d.formal; }

std::string lista(const ValoresAtrib& val, int a, int i) {
    std::string s;
    for (int x : val.val[a][i]) {
        std::string v = val.nomes[a][x] == "dia" ? "dia a dia" : val.nomes[a][x];
        s += (s.empty() ? "" : ", ") + v;
    }
    return s.empty() ? "nenhum" : s;
}

// a chave da forma 3 e igual em todo o grupo, entao a primeira peca descreve o grupo
std::vector<Desc> descrever(const Base& base, const Grupos& gr, const std::vector<Atrib>& attr,
                            const ValoresAtrib& val) {
    std::vector<Desc> d(gr.tamanho.size());
    std::vector<bool> feito(gr.tamanho.size(), false);
    for (int i = 0; i < (int)gr.grupo.size(); i++) {
        int g = gr.grupo[i];
        if (feito[g]) continue;
        feito[g] = true;
        double e = valor_estampa(attr[i].estampa);
        double f = attr[i].g1.size() > 10 ? attr[i].g1[10] : 0.5;
        d[g].cat = base.pecas[i].categoria;
        d[g].cor = val.val[COR][i].empty() ? "desconhecido" : val.nomes[COR][val.val[COR][i][0]];
        d[g].estampa = e == 0.0 ? "lisa" : (e == 1.0 ? "estampada" : "estampa?");
        d[g].formal = faixa_formalidade(f);
    }
    return d;
}

struct Rodada {
    double limiar = 0.0;
    std::vector<ParGrupos> arestas;
    Tarjan t;
    double micro = 0.0;  // media de uma execucao do Tarjan
    std::vector<int> grau;
    std::vector<int> separados;  // grupos fora do maior pedaco se o grupo sair
    std::vector<int> ordem_art;  // articulacoes com MIN_SEPARADOS ou mais, da mais importante
    std::vector<int> ordem_ponte;
    int componentes = 0;  // com 2 grupos ou mais
    int maior = 0;
    int com_aresta = 0;
    int maior_solto = 0;  // segundo maior pedaco entre todas as articulacoes
};

Rodada rodar(int n, const std::vector<ParGrupos>& todas, double limiar) {
    Rodada r;
    r.limiar = limiar;
    std::vector<std::pair<int, int>> pares;
    for (const ParGrupos& a : todas) {
        if (a.n < MIN_N_ARESTA || a.pontos <= limiar) continue;
        r.arestas.push_back(a);
        pares.push_back({a.g, a.h});
    }
    auto ini = std::chrono::steady_clock::now();
    for (int k = 0; k < REPETICOES_TEMPO; k++) r.t = tarjan(n, pares);
    r.micro = std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - ini).count() /
              REPETICOES_TEMPO;

    r.grau.assign(n, 0);
    for (const auto& p : pares) {
        r.grau[p.first]++;
        r.grau[p.second]++;
    }
    for (int v = 0; v < n; v++) r.com_aresta += r.grau[v] > 0;
    for (int s : r.t.tam_comp) {
        if (s >= 2) r.componentes++;
        r.maior = std::max(r.maior, s);
    }
    r.separados.assign(n, 0);
    for (int v = 0; v < n; v++) {
        const std::vector<int>& p = r.t.pedacos[v];
        for (std::size_t k = 1; k < p.size(); k++) r.separados[v] += p[k];
        if (p.size() > 1) r.maior_solto = std::max(r.maior_solto, p[1]);
        if (r.t.articulacao[v] && r.separados[v] >= MIN_SEPARADOS) r.ordem_art.push_back(v);
    }
    std::stable_sort(r.ordem_art.begin(), r.ordem_art.end(), [&](int a, int b) {
        if (r.separados[a] != r.separados[b]) return r.separados[a] > r.separados[b];
        return r.grau[a] > r.grau[b];
    });
    for (int e = 0; e < (int)pares.size(); e++) {
        if (r.t.ponte[e]) r.ordem_ponte.push_back(e);
    }
    std::stable_sort(r.ordem_ponte.begin(), r.ordem_ponte.end(), [&](int a, int b) {
        if (r.t.separa[a] != r.t.separa[b]) return r.t.separa[a] > r.t.separa[b];
        return r.arestas[a].pontos > r.arestas[b].pontos;
    });
    return r;
}

// grupos que se soltam se v sair: busca a partir dos vizinhos sem passar por v
std::vector<int> soltos(int n, const Rodada& r, int v) {
    std::vector<std::vector<int>> adj(n);
    for (const ParGrupos& a : r.arestas) {
        adj[a.g].push_back(a.h);
        adj[a.h].push_back(a.g);
    }
    std::vector<int> rot(n, -1);
    rot[v] = -2;
    std::vector<std::vector<int>> pedacos;
    for (int s : adj[v]) {
        if (rot[s] != -1) continue;
        std::vector<int> membros = {s};
        rot[s] = (int)pedacos.size();
        for (std::size_t k = 0; k < membros.size(); k++) {
            for (int w : adj[membros[k]]) {
                if (rot[w] == -1) {
                    rot[w] = rot[s];
                    membros.push_back(w);
                }
            }
        }
        pedacos.push_back(membros);
    }
    std::stable_sort(pedacos.begin(), pedacos.end(),
                     [](const std::vector<int>& a, const std::vector<int>& b) { return a.size() > b.size(); });
    std::vector<int> res;
    for (std::size_t k = 1; k < pedacos.size(); k++) res.insert(res.end(), pedacos[k].begin(), pedacos[k].end());
    return res;
}

// "4 earrings, 2 rings"
std::string por_categoria(const std::vector<int>& grupos, const std::vector<Desc>& d) {
    std::map<std::string, int> c;
    for (int g : grupos) c[d[g].cat]++;
    std::vector<std::pair<std::string, int>> v(c.begin(), c.end());
    std::stable_sort(v.begin(), v.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
    std::string s;
    for (const auto& x : v) s += (s.empty() ? "" : ", ") + std::to_string(x.second) + " " + x.first;
    return s;
}

bool gravar_gephi(const std::string& prefixo, const Rodada& r, const std::vector<Desc>& d, const Grupos& gr) {
    std::ofstream nos(prefixo + "_nodes.csv");
    std::ofstream ar(prefixo + "_edges.csv");
    nos << "Id,Label,Categoria,Cor,Estampa,Formalidade,Pecas,Grau,articulacao,grupos_separados,destaque\n";
    for (int g = 0; g < (int)d.size(); g++) {
        bool art = r.t.articulacao[g];
        nos << g << ',' << nome(d[g]) << ',' << d[g].cat << ',' << d[g].cor << ',' << d[g].estampa << ','
            << d[g].formal << ',' << gr.tamanho[g] << ',' << r.grau[g] << ',' << (art ? 1 : 0) << ','
            << r.separados[g] << ',' << (art && r.separados[g] >= MIN_SEPARADOS ? 1 : 0) << '\n';
    }
    ar << "Source,Target,Weight,n,ponte,Type\n";
    for (int e = 0; e < (int)r.arestas.size(); e++) {
        const ParGrupos& a = r.arestas[e];
        ar << a.g << ',' << a.h << ',' << std::fixed << std::setprecision(4) << a.pontos << ',' << a.n << ','
           << (r.t.ponte[e] ? 1 : 0) << ",Undirected\n";
    }
    return (bool)nos && (bool)ar;
}

}  // namespace

bool rodar_articulacoes(const Base& base, const std::string& pasta_dados, const std::string& pasta_saida) {
    std::vector<Atrib> attr;
    ValoresAtrib val;
    Grupos gr;
    std::string erro;
    if (!ler_atributos(pasta_dados + "/pecas_v4.csv", base, attr, erro) ||
        !ler_valores(pasta_dados + "/pecas_v5.csv", base, val, erro) || !grupos_escolhidos(base, pasta_dados, gr, erro)) {
        std::cerr << erro << '\n';
        return false;
    }
    const int n = (int)gr.tamanho.size();
    const std::vector<Desc> d = descrever(base, gr, attr, val);
    // PontosGrupos conta so os looks com treino = true
    PontosGrupos pg(base, gr);
    const std::vector<ParGrupos> todas = pg.arestas(1);

    std::vector<double> positivas;
    int com_n = 0;
    int n_baixo = 0;
    for (const ParGrupos& a : todas) {
        if (a.n < MIN_N_ARESTA) {
            if (a.pontos > 0) n_baixo++;
            continue;
        }
        com_n++;
        if (a.pontos > 0) positivas.push_back(a.pontos);
    }
    if (positivas.empty()) {
        std::cerr << "Nenhuma aresta positiva com n >= " << MIN_N_ARESTA << ".\n";
        return false;
    }
    std::sort(positivas.begin(), positivas.end());
    std::vector<double> limiares;
    for (double f : {0.25, 0.5, 0.75}) limiares.push_back(positivas[(std::size_t)(f * (positivas.size() - 1))]);

    std::vector<Rodada> rodadas;
    for (double l : limiares) rodadas.push_back(rodar(n, todas, l));

    // aparicoes de cada peca nos looks de treino, pra escolher os exemplos
    std::vector<int> aparece(base.pecas.size(), 0);
    int looks_treino = 0;
    for (const Look& l : base.looks) {
        if (!l.treino) continue;
        looks_treino++;
        for (int p : l.pecas) aparece[p]++;
    }
    std::vector<std::vector<int>> membros(n);
    for (int i = 0; i < (int)gr.grupo.size(); i++) membros[gr.grupo[i]].push_back(i);
    for (auto& m : membros) {
        std::stable_sort(m.begin(), m.end(), [&](int a, int b) { return aparece[a] > aparece[b]; });
    }

    const std::string pasta_gephi = pasta_saida + "/gephi_articulacoes";
    std::filesystem::create_directories(pasta_gephi);
    for (int k = 0; k < 3; k++) {
        if (!gravar_gephi(pasta_gephi + "/" + NOMES_LIMIAR[k], rodadas[k], d, gr)) {
            std::cerr << "Não foi possível gravar " << pasta_gephi << '\n';
            return false;
        }
    }

    std::ofstream md(pasta_saida + "/articulacoes.md");
    md << "# Articulações e pontes entre grupos\n\n"
       << "Gerado por `./recomendador articulacoes`. É uma análise feita depois do congelamento. "
          "Nenhuma recomendação muda.\n\n"
       << "## O que é uma articulação\n\n"
       << "Uma articulação é um grupo que é a única ligação entre estilos. Se ele sai do grafo, um conjunto de "
          "grupos fica sem caminho até o resto. Isso não quer dizer que o grupo é o mais versátil. "
          "O grupo mais versátil é o que combina com mais grupos, e isso é medido pelo grau. "
          "O grau conta os vizinhos. A articulação conta os grupos que só chegam ao resto por ela.\n\n"
       << "## Grafo\n\n"
       << "Vértices: os " << n << " grupos da forma escolhida (forma 3, mesma combinação de grupo_cor, estampa e "
          "formalidade dentro de uma categoria). Os grupos estão em `grupos_formas.md`. Nas tabelas, "
          "\"estampa?\" quer dizer estampa desconhecida.\n\n"
       << "Arestas: pares de grupos de categorias diferentes que aparecem juntos nos " << looks_treino
       << " looks de treino. O peso é `pontos = log((n + 1) / (esperado + 1))` de `PontosGrupos`. "
          "Positivo quer dizer que o par aparece mais do que o esperado pelo tamanho dos grupos. "
          "Há " << todas.size() << " pares com n ≥ 1.\n\n"
       << "Mínimo de contagem: a aresta só entra com n ≥ " << MIN_N_ARESTA << " pares de peças. "
          "Com n = 1 ou 2, o par pode vir de um look só. Sem esse corte, "
       << n_baixo << " das " << (n_baixo + (int)positivas.size())
       << " arestas positivas teriam n = 1 ou 2 e dominariam o grafo. Com o corte, ficam " << com_n
       << " pares, dos quais " << positivas.size() << " positivos.\n\n"
       << "Limiares: os 3 quartis dos pontos das " << positivas.size()
       << " arestas positivas com n ≥ " << MIN_N_ARESTA << ". Assim cada limiar guarda 75%, 50% e 25% "
          "dessas arestas, e o grafo vai do mais denso ao mais esparso. "
          "Como os pontos são um log, o limiar l quer dizer (n + 1) / (esperado + 1) acima de e^l. "
          "Os limiares não foram ajustados para nenhum resultado. A tabela completa usa a mediana.\n\n";

    md << "Articulações que soltam só 1 ou 2 grupos ficam de fora das contas \"com " << MIN_SEPARADOS
       << " ou mais\". Grupos separados = grupos que ficam fora do maior pedaço quando a articulação sai.\n\n"
       << "| limiar | pontos > | razão > | arestas | grupos com aresta | componentes (2+ grupos) | maior componente "
          "| articulações | com " << MIN_SEPARADOS << "+ separados | maior pedaço solto | pontes | pontes com " << MIN_SEPARADOS
       << "+ separados | tempo do Tarjan |\n"
       << "| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |\n";
    for (int k = 0; k < 3; k++) {
        const Rodada& r = rodadas[k];
        int arts = 0;
        for (int v = 0; v < n; v++) arts += r.t.articulacao[v];
        int pontes = (int)r.ordem_ponte.size();
        int pontes_grandes = 0;
        for (int e : r.ordem_ponte) pontes_grandes += r.t.separa[e] >= MIN_SEPARADOS;
        md << "| " << TEXTO_LIMIAR[k] << " | " << num(r.limiar, 3) << " | " << num(std::exp(r.limiar), 2) << " | "
           << r.arestas.size() << " | " << r.com_aresta << " | " << r.componentes << " | " << r.maior << " | " << arts
           << " | " << r.ordem_art.size() << " | " << r.maior_solto << " | " << pontes << " | " << pontes_grandes << " | " << num(r.micro, 0)
           << " µs |\n";
        std::cout << TEXTO_LIMIAR[k] << ": limiar " << r.limiar << ", " << r.arestas.size() << " arestas, "
                  << r.ordem_art.size() << " articulacoes com " << MIN_SEPARADOS << "+, " << pontes << " pontes, "
                  << r.micro << " us" << std::endl;
    }
    md << "\nO tempo é a média de " << REPETICOES_TEMPO << " execuções do Tarjan no mesmo grafo. Inclui a lista de "
          "adjacência que o Tarjan monta, mas não a escolha das arestas pelo limiar. O custo é O(V + E): a DFS passa uma vez por vértice e duas vezes por aresta. "
          "A DFS usa uma pilha explícita, sem recursão.\n";

    const Rodada& r = rodadas[PRINCIPAL];
    md << "\n## As " << N_TABELA << " articulações mais importantes (limiar na mediana, pontos > "
       << num(r.limiar, 3) << ")\n\n"
       << "Ordem: mais grupos separados, depois maior grau. Pedaços = tamanho de cada pedaço que sobra quando o "
          "grupo sai, o primeiro é o maior.\n\n"
       << "| # | grupo | categoria | cor | estampa | formalidade | peças | grau | grupos separados | pedaços | o que se "
          "solta |\n| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |\n";
    const int mostrar = std::min(N_TABELA, (int)r.ordem_art.size());
    for (int k = 0; k < mostrar; k++) {
        int v = r.ordem_art[k];
        std::string ped;
        for (std::size_t j = 0; j < r.t.pedacos[v].size(); j++) {
            ped += (j ? " + " : "") + std::to_string(r.t.pedacos[v][j]);
        }
        md << "| " << k + 1 << " | " << v << " | " << d[v].cat << " | " << d[v].cor << " | " << d[v].estampa << " | "
           << d[v].formal << " | " << gr.tamanho[v] << " | " << r.grau[v] << " | " << r.separados[v]
           << " | " << ped << " | " << por_categoria(soltos(n, r, v), d) << " |\n";
    }
    if (mostrar == 0) md << "\nNenhuma articulação separa " << MIN_SEPARADOS << " grupos ou mais nesse limiar.\n";
    // segundo maior pedaco: diz se o que se solta e um bloco ou so grupos pendurados
    const int maior_solto = r.maior_solto;
    md << "\nEm todas as " << std::count(r.t.articulacao.begin(), r.t.articulacao.end(), true)
       << " articulações desse limiar, o maior pedaço que se solta tem " << maior_solto
       << (maior_solto == 1 ? " grupo. Ou seja, o que se solta são grupos com uma ligação só, pendurados na "
                              "articulação, e não um estilo inteiro com várias ligações internas."
                            : " grupos.")
       << " Fora do grafo ficam " << n - r.com_aresta << " grupos sem nenhuma aresta nesse limiar.\n";

    md << "\n### Exemplos de peças\n\nAs " << N_EXEMPLOS
       << " peças de cada grupo que mais aparecem nos looks de treino.\n\n"
       << "| # | id | categoria | grupo_cor | estampa | formalidade | estilo | ocasião | looks de treino |\n"
       << "| --- | --- | --- | --- | --- | --- | --- | --- | --- |\n";
    for (int k = 0; k < mostrar; k++) {
        int v = r.ordem_art[k];
        for (int j = 0; j < N_EXEMPLOS && j < (int)membros[v].size(); j++) {
            int i = membros[v][j];
            double f = attr[i].g1.size() > 10 ? attr[i].g1[10] : 0.5;
            md << "| " << k + 1 << " | " << base.pecas[i].id << " | " << base.pecas[i].categoria << " | " << d[v].cor
               << " | " << attr[i].estampa << " | " << faixa_formalidade(f) << " (" << num(f, 2) << ") | "
               << lista(val, ESTILO, i) << " | " << lista(val, OCASIAO, i) << " | " << aparece[i] << " |\n";
        }
    }

    md << "\n## Os " << N_TABELA << " grupos de maior grau (mesmo limiar)\n\n"
       << "Para comparar: o grau mede com quantos grupos de outras categorias o grupo combina.\n\n"
       << "| # | grupo | descrição | peças | grau | articulação? | grupos separados |\n"
       << "| --- | --- | --- | --- | --- | --- | --- |\n";
    std::vector<int> por_grau(n);
    for (int v = 0; v < n; v++) por_grau[v] = v;
    std::stable_sort(por_grau.begin(), por_grau.end(), [&](int a, int b) { return r.grau[a] > r.grau[b]; });
    int grau_art = 0;
    int grau_qualquer = 0;
    for (int k = 0; k < N_TABELA && k < n; k++) {
        int v = por_grau[k];
        bool art = r.t.articulacao[v];
        grau_art += art && r.separados[v] >= MIN_SEPARADOS;
        grau_qualquer += art;
        md << "| " << k + 1 << " | " << v << " | " << nome(d[v]) << " | " << gr.tamanho[v] << " | " << r.grau[v]
           << " | " << (art ? "sim" : "não") << " | " << r.separados[v] << " |\n";
    }
    int grau_max_art = 0;
    for (int v : r.ordem_art) grau_max_art = std::max(grau_max_art, r.grau[v]);
    md << "\nDos " << N_TABELA << " grupos de maior grau, " << grau_qualquer << " são articulações, mas só "
       << grau_art << " separa" << (grau_art == 1 ? "" : "m") << " " << MIN_SEPARADOS
       << " grupos ou mais. O maior grau entre as articulações da tabela acima é " << grau_max_art << ", contra "
       << (n > 0 ? r.grau[por_grau[0]] : 0)
       << " do grupo de maior grau. Um grupo com muitos vizinhos tem mais chance de ser a única ligação de algum "
          "grupo pequeno, mas o grau sozinho não diz quantos grupos dependem dele.\n";

    md << "\n## Pontes (mesmo limiar)\n\n"
       << "Uma ponte é uma aresta que, se sair, separa o grafo. Há " << r.ordem_ponte.size()
       << " pontes. Separa = grupos do lado menor. Nenhuma ponte separa mais de "
       << (r.ordem_ponte.empty() ? 0 : r.t.separa[r.ordem_ponte[0]])
       << (!r.ordem_ponte.empty() && r.t.separa[r.ordem_ponte[0]] == 1
               ? " grupo, então a lista abaixo vem na ordem dos pontos.\n\n"
               : " grupos.\n\n")
       << "| # | grupo A | grupo B | n | pontos | separa |\n| --- | --- | --- | --- | --- | --- |\n";
    for (int k = 0; k < N_TABELA && k < (int)r.ordem_ponte.size(); k++) {
        int e = r.ordem_ponte[k];
        const ParGrupos& a = r.arestas[e];
        md << "| " << k + 1 << " | " << a.g << " " << nome(d[a.g]) << " | " << a.h << " " << nome(d[a.h]) << " | "
           << a.n << " | " << num(a.pontos, 3) << " | " << r.t.separa[e] << " |\n";
    }

    md << "\n## Nos outros limiares\n";
    for (int k = 0; k < 3; k++) {
        if (k == PRINCIPAL) continue;
        const Rodada& o = rodadas[k];
        md << "\n" << TEXTO_LIMIAR[k] << " (pontos > " << num(o.limiar, 3) << "), as " << N_TABELA
           << " primeiras articulações:\n\n| # | grupo | descrição | grau | grupos separados |\n"
           << "| --- | --- | --- | --- | --- |\n";
        for (int j = 0; j < N_TABELA && j < (int)o.ordem_art.size(); j++) {
            int v = o.ordem_art[j];
            md << "| " << j + 1 << " | " << v << " | " << nome(d[v]) << " | " << o.grau[v] << " | " << o.separados[v]
               << " |\n";
        }
        if (o.ordem_art.empty()) md << "| | nenhuma | | | |\n";
        int pontes_grandes = 0;
        for (int e : o.ordem_ponte) pontes_grandes += o.t.separa[e] >= MIN_SEPARADOS;
        md << "\n" << o.ordem_ponte.size() << " pontes, " << pontes_grandes << " com " << MIN_SEPARADOS
           << " ou mais grupos do lado menor.\n";
    }

    md << "\n## Gephi\n\n"
       << "`gephi_articulacoes/q1_*`, `q2_*` e `q3_*`, um par por limiar. Nos nós, `articulacao` = 1 marca toda "
          "articulação do Tarjan, `grupos_separados` diz quantos grupos se soltam e `destaque` = 1 marca as que "
          "soltam " << MIN_SEPARADOS << " ou mais. Nas arestas, `Weight` são os pontos, `n` a contagem e `ponte` = 1 "
          "marca as pontes. O Id do nó é o número do grupo usado nas tabelas.\n";
    std::cout << "Resumo em " << pasta_saida << "/articulacoes.md" << std::endl;
    return (bool)md;
}
