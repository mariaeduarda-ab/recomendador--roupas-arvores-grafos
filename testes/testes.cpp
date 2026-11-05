#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "avaliacao.hpp"
#include "avaliacao_d.hpp"
#include "avaliacao_fitb.hpp"
#include "grafo_looks.hpp"
#include "grafo_similaridade.hpp"
#include "grupos_kruskal.hpp"
#include "kdtree.hpp"
#include "leitura.hpp"
#include "recomendador.hpp"
#include "tempos_f.hpp"
#include "union_find.hpp"
#include "versao_d.hpp"
#include "versao_e.hpp"

namespace {

int aprovados = 0;
int reprovados = 0;

void checar(bool condicao, const std::string& nome) {
    std::cout << (condicao ? "OK      " : "FALHOU  ") << nome << '\n';
    (condicao ? aprovados : reprovados)++;
}

bool quase_igual(double a, double b) { return std::fabs(a - b) < 1e-9; }

void testar_regras_e() {
    TabelaCores cores;
    cores.adicionar("red", {"brown"});
    cores.adicionar("green", {"blue"});
    int red = cores.id("red");
    int brown = cores.id("brown");
    int green = cores.id("green");
    int black = cores.id("black");

    checar(quase_igual(regra_cor_e(cores, red, Tipo::CIMA, red, Tipo::BAIXO), -1.0), "Cor E: vermelho em cima e embaixo perde");
    checar(quase_igual(regra_cor_e(cores, red, Tipo::CIMA, red, Tipo::COMPLEMENTO), 1.0), "Cor E: vermelho em cima e no sapato ganha");
    checar(quase_igual(regra_cor_e(cores, red, Tipo::CIMA, brown, Tipo::BAIXO), 0.5), "Cor E: cores da mesma linha da tabela");
    checar(quase_igual(regra_cor_e(cores, red, Tipo::CIMA, green, Tipo::BAIXO), -0.5), "Cor E: cores fora da tabela");
    checar(quase_igual(regra_cor_e(cores, black, Tipo::CIMA, black, Tipo::BAIXO), 0.0), "Cor E: preto com preto nao perde");

    checar(quase_igual(regra_atencao(1.0, 0.0), 1.0), "Atencao: chamativa com calma ganha");
    checar(quase_igual(regra_atencao(1.0, 1.0), -1.0), "Atencao: duas chamativas perdem");
    checar(quase_igual(regra_atencao(0.0, 0.0), 0.0), "Atencao: duas calmas ficam neutras");
}

void testar_versao_e() {
    Base base;
    const char* cats[] = {"top", "pants", "pants", "shoes"};
    for (int i = 0; i < 4; i++) {
        Peca p;
        p.id = i;
        p.categoria = cats[i];
        p.v = {0.0};
        base.pecas.push_back(p);
        base.indice_da_peca[i] = i;
    }
    Look treino;
    treino.pecas = {0, 2};
    base.looks.push_back(treino);

    TabelaCores cores;
    cores.adicionar("red", {"brown"});
    std::vector<AtribE> attr_e(4);
    attr_e[0].cor = cores.id("red");
    attr_e[1].cor = cores.id("red");
    attr_e[2].cor = cores.id("brown");
    attr_e[3].cor = cores.id("red");
    std::vector<Atrib> attr(4);
    for (Atrib& a : attr) a.g1 = std::vector<double>(11, 0.0);

    GrafoLooks grafo;
    grafo.construir(base);
    VersaoE versao(base, attr, attr_e, cores, grafo);
    ConfigE cfg;

    std::vector<Candidata> calcas = versao.completar({0}, "pants", cfg, 10);
    checar(calcas.size() == 1 && calcas[0].indice == 2, "Versao E: blusa vermelha nao busca calca vermelha");
    std::vector<Candidata> sapatos = versao.completar({0}, "shoes", cfg, 10);
    checar(sapatos.size() == 1 && sapatos[0].indice == 3, "Versao E: blusa vermelha busca sapato vermelho");
    checar(versao.completar({2}, "pants", cfg, 10).empty(), "Versao E: sem peca de outra categoria nao ha candidata");

    cfg.filtrar_cor = false;
    checar(versao.completar({0}, "pants", cfg, 10).size() == 2, "Versao E: sem filtro as duas calcas entram");
    cfg.filtrar_cor = true;
    cfg.categoria_inteira = true;
    checar(versao.completar({0}, "pants", cfg, 10).size() == 1, "Versao E: categoria inteira respeita o filtro");

    Partes x;
    x.soma = {1.0, 2.0, 3.0, 4.0};
    checar(quase_igual(x.nota(1.0, 0.0, 0.0, 0.5), 3.0), "Versao E: peso 0 desliga a parte");
}

void testar_fitb() {
    Pergunta p;
    p.certa = 1;
    p.opcoes = {1, 2, 3, 4};
    checar(quase_igual(credito({{1, 5.0}, {2, 3.0}}, p), 1.0), "Multipla escolha: certa com a maior nota");
    checar(quase_igual(credito({{1, 3.0}, {2, 5.0}}, p), 0.0), "Multipla escolha: errada com a maior nota");
    checar(quase_igual(credito({{1, 5.0}, {2, 5.0}}, p), 0.5), "Multipla escolha: empate divide o ponto");
    checar(quase_igual(credito({}, p), 0.25), "Multipla escolha: sem nota nenhuma vale 1/4");
    checar(quase_igual(credito({{2, 1.0}}, p), 0.0), "Multipla escolha: certa fora da lista perde");
    checar(quase_igual(credito({{9, 7.0}, {1, 1.0}}, p), 1.0), "Multipla escolha: peca fora das opcoes nao conta");

    Base base;
    for (int i = 0; i < 12; i++) {
        Peca x;
        x.id = i;
        x.categoria = i < 8 ? "shoes" : "top";
        base.pecas.push_back(x);
    }
    Look look;
    look.pecas = {0, 8};
    base.looks.push_back(look);
    Look outro;
    outro.pecas = {1, 2, 3, 9};
    base.looks.push_back(outro);
    Escondida e;
    e.look = 0;
    e.pos = 0;
    std::vector<Pergunta> a = montar_perguntas(base, {e}, 5);
    std::vector<Pergunta> b = montar_perguntas(base, {e}, 5);
    bool ok = a.size() == 1 && a[0].opcoes.size() == 4 && a[0].opcoes[0] == 0;
    std::set<int> sem_repetir(a[0].opcoes.begin(), a[0].opcoes.end());
    ok = ok && sem_repetir.size() == 4;
    for (int o : a[0].opcoes) ok = ok && o < 8;
    checar(ok, "Multipla escolha: 4 opcoes diferentes da mesma categoria");
    checar(a[0].opcoes == b[0].opcoes, "Multipla escolha: mesma semente, mesmas perguntas");
    std::set<int> no_treino(a[0].opcoes.begin() + 1, a[0].opcoes.end());
    checar(no_treino == std::set<int>({1, 2, 3}), "Multipla escolha: erradas aparecem no treino como a certa");
}

std::vector<Peca> pontos_do_guia(const Base& base) {
    return std::vector<Peca>(base.pecas.begin(), base.pecas.begin() + 6);
}

std::set<int> indices(const std::vector<Vizinho>& vizinhos) {
    std::set<int> saida;
    for (const Vizinho& vizinho : vizinhos) saida.insert(vizinho.indice);
    return saida;
}

void testar_leitura(const Base& base) {
    checar(base.pecas.size() == 16, "Leitura: 16 peças no exemplo");
    checar(base.looks.size() == 5, "Leitura: 5 looks no exemplo");
    checar(base.pecas[0].v.size() == 2, "Leitura: vetor com a dimensão do arquivo");
}

void testar_hit() {
    const std::vector<Candidata> lista = {{3, 1.0}, {7, 0.5}};
    checar(hit_em_k(lista, 7, 2) == 1 && hit_em_k(lista, 7, 1) == 0, "Hit@k: posição 2 acerta com k = 2 e erra com k = 1");
}

void testar_union_find() {
    UnionFind uf(6);
    uf.unite(0, 1);
    uf.unite(2, 3);
    uf.unite(1, 3);
    uf.find(3);
    checar(uf.pais() == std::vector<int>({0, 0, 0, 0, 4, 5}), "Union-Find: parent = [0, 0, 0, 0, 4, 5]");
    checar(uf.contar_grupos() == 3, "Union-Find: 3 grupos");

    UnionFind contagem(5);
    contagem.definir_pais({2, 2, 2, 3, 3});
    checar(contagem.contar_grupos() == 2, "Union-Find: parent = [2, 2, 2, 3, 3] tem 2 grupos");
}

void testar_kdtree_exemplo(const Base& base) {
    const std::vector<Peca> pecas = pontos_do_guia(base);
    KDTree arvore(2);
    arvore.construir(pecas);
    checar(arvore.preordem() == std::vector<int>({0, 1, 3, 4, 2, 5}), "KD-tree: pré-ordem A, B, D, E, C, F");

    int visitados = 0;
    const Vizinho melhor = arvore.mais_proximo({9, 2}, visitados);
    checar(melhor.indice == 5, "KD-tree: q = (9, 2) acha F");
    checar(visitados == 3, "KD-tree: q = (9, 2) visita 3 nós");
    checar(indices(arvore.knn({9, 2}, 2)) == std::set<int>({5, 0}), "KD-tree: k = 2 acha {F, A}");
}

void testar_kdtree_contra_forca_bruta() {
    std::mt19937 gerador(42);
    std::uniform_real_distribution<double> sorteio(0.0, 1.0);
    std::vector<Peca> pecas(10000);
    for (std::size_t i = 0; i < pecas.size(); i++) {
        pecas[i].id = (int)i;
        pecas[i].categoria = "camisa";
        for (int d = 0; d < 5; d++) pecas[i].v.push_back(sorteio(gerador));
    }
    KDTree arvore(5);
    arvore.construir(pecas);

    bool iguais = true;
    for (int consulta = 0; consulta < 1000 && iguais; consulta++) {
        std::vector<double> q;
        for (int d = 0; d < 5; d++) q.push_back(sorteio(gerador));
        const std::vector<Vizinho> a = arvore.knn(q, 5);
        const std::vector<Vizinho> b = arvore.forca_bruta(q, 5);
        // compara distancia e nao indice por causa de empates
        iguais = a.size() == 5 && b.size() == 5;
        for (std::size_t i = 0; i < a.size() && iguais; i++) iguais = quase_igual(a[i].dist2, b[i].dist2);
    }
    checar(iguais, "KD-tree: igual à força bruta em 1.000 consultas");
}

void testar_knn_mutuo() {
    std::mt19937 gerador(42);
    std::normal_distribution<double> ruido(0.0, 1.0);
    std::vector<Peca> pecas(400);
    for (std::size_t i = 0; i < pecas.size(); i++) {
        const double centro = i < 200 ? 0.0 : 100.0;
        pecas[i].id = (int)i;
        pecas[i].categoria = "camisa";
        pecas[i].v = {centro + ruido(gerador), centro + ruido(gerador)};
    }
    KDTree arvore(2);
    arvore.construir(pecas);
    GrafoSimilaridade grafo;
    grafo.construir(pecas, arvore, 5);
    const std::vector<int> grupos = grafo.grupos();

    bool sem_mistura = grupos.size() == pecas.size();
    std::map<int, int> nuvem_do_grupo;
    for (std::size_t i = 0; i < grupos.size() && sem_mistura; i++) {
        const int nuvem = i < 200 ? 0 : 1;
        const auto novo = nuvem_do_grupo.emplace(grupos[i], nuvem);
        if (novo.first->second != nuvem) sem_mistura = false;
    }
    checar(nuvem_do_grupo.size() >= 2, "kNN mútuo: pelo menos 2 grupos");
    checar(sem_mistura, "kNN mútuo: nenhum grupo mistura as nuvens");
}

void testar_bfs(const Base& base) {
    GrafoLooks grafo;
    grafo.construir(base);
    const int grafite = base.indice_da_peca.at(9);
    const auto contagem = grafo.bfs_2_passos(grafite);
    const auto valor = [&](int id) {
        const auto it = contagem.find(base.indice_da_peca.at(id));
        return it == contagem.end() ? 0.0 : it->second;
    };
    checar(quase_igual(valor(7), 1.0), "BFS: saia cinza com contagem 1");
    checar(quase_igual(valor(11), 1.0), "BFS: saia jeans com contagem 1");
}

void testar_versao_c(const Base& base) {
    checar(quase_igual(peso_vizinha(0.2, 1.0), 0.8), "Peso: d = 0,2 e d_ref = 1 dá 0,8");
    checar(quase_igual(peso_vizinha(1.5, 1.0), 0.0), "Peso: nunca fica negativo");

    KDTree arvore(2);
    arvore.construir(base.pecas);
    GrafoLooks grafo;
    grafo.construir(base);
    const Recomendador recomendador(base, arvore, grafo);

    const int preta = base.indice_da_peca.at(6);
    const int grafite = base.indice_da_peca.at(9);
    const int seda = base.indice_da_peca.at(13);
    const std::vector<Candidata> lista =
        recomendador.pontuar_partidas({{preta, 1.0}, {grafite, 0.8}, {seda, 0.5}}, {preta}, "saia", 10);

    const bool certo = lista.size() == 3 &&
                       lista[0].indice == base.indice_da_peca.at(7) && quase_igual(lista[0].pontuacao, 1.8) &&
                       lista[1].indice == base.indice_da_peca.at(11) && quase_igual(lista[1].pontuacao, 0.8) &&
                       lista[2].indice == base.indice_da_peca.at(14) && quase_igual(lista[2].pontuacao, 0.5);
    checar(certo, "Versão C: saia cinza 1,8, saia jeans 0,8 e saia preta 0,5");
}

void testar_casos_de_borda(const Base& base) {
    KDTree arvore(2);
    arvore.construir(base.pecas);
    GrafoLooks grafo;
    grafo.construir(base);
    const Recomendador rec(base, arvore, grafo);

    checar(arvore.knn({0, 0}, 0).empty(), "Borda: k = 0 devolve vazio");
    checar(arvore.knn({0, 0}, 100).size() == base.pecas.size(), "Borda: k maior que a base devolve todas");

    const std::string saia = "saia";
    checar(arvore.knn({0, 0}, 100, &saia).size() == 3, "Borda: filtro de categoria");
    checar(rec.versao_a({}, saia, 5).empty(), "Borda: look vazio na versão A");
    checar(rec.versao_c({}, saia, 5).empty(), "Borda: look vazio na versão C");
    checar(rec.popularidade({}, "nao existe", 5).empty(), "Borda: categoria desconhecida");

    const int teste_preta = base.indice_da_peca.at(6);
    checar(grafo.num_looks(teste_preta) == 1, "Vazamento: look de teste fora do grafo");

    const std::vector<Candidata> pop = rec.popularidade({}, saia, 1);
    checar(pop.size() == 1 && pop[0].indice == base.indice_da_peca.at(7), "Popularidade: saia cinza primeiro");
    const std::vector<Candidata> sem_cinza = rec.popularidade({base.indice_da_peca.at(7)}, saia, 10);
    checar(sem_cinza.size() == 2 && sem_cinza[0].indice != base.indice_da_peca.at(7), "Popularidade: peça do look sai da lista");

    const std::vector<Candidata> c = rec.versao_c({teste_preta}, saia, 10);
    checar(c.size() == 3 && c[0].indice == base.indice_da_peca.at(7), "Versão C completa: saia cinza primeiro");
    checar(rec.versao_b({teste_preta}, saia, 10).size() == 1, "Versão B: só acha a saia cinza");
}

void testar_regras() {
    Atrib lisa, listrada, floral, nao_sei;
    lisa.estampa = "lisa";
    listrada.estampa = "listrada";
    floral.estampa = "floral";
    checar(regra_estampa(listrada, floral) == -1.0, "Estampa: duas estampadas tiram ponto");
    checar(regra_estampa(lisa, floral) == 1.0, "Estampa: lisa com estampada dá ponto");
    checar(regra_estampa(lisa, lisa) == 0.0, "Estampa: duas lisas é neutro");
    checar(regra_estampa(nao_sei, floral) == 0.0, "Estampa: desconhecido é neutro");

    Atrib neutra;
    neutra.neutra = true;
    checar(regra_cor(neutra, lisa) == BONUS_COR_NEUTRA, "Cor: uma neutra ganha o bônus");
    checar(regra_cor(lisa, floral) == 0.0, "Cor: nenhuma neutra fica sem bônus");
}

void testar_versao_d(const Base& base) {
    std::vector<Atrib> attr;
    std::string erro;
    checar(ler_atributos_da_pasta("dados/exemplo", base, attr, erro), "Versão D: lê os atributos do exemplo");
    GrafoLooks grafo;
    grafo.construir(base);
    VersaoD d(base, attr, grafo, ConfigD{});

    const int preta = base.indice_da_peca.at(6);
    const int cinza = base.indice_da_peca.at(7);

    bool sem_mesma_cat = true;
    for (int p = 0; p < (int)d.arestas().size(); p++) {
        for (int q : d.arestas()[p]) {
            if (base.pecas[p].categoria == base.pecas[q].categoria) sem_mesma_cat = false;
        }
    }
    checar(sem_mesma_cat, "Versão D: aresta só entre categorias");
    const std::vector<int>& adj_preta = d.arestas()[preta];
    checar(std::find(adj_preta.begin(), adj_preta.end(), cinza) != adj_preta.end(), "Versão D: camisa preta liga com saia cinza");

    // vetores iguais: 1 de proximidade, 0,5 de cor, 0 de estampa, 1 / raiz(2) de bonus
    const double esperada = 1.0 + BONUS_COR_NEUTRA + 1.0 / std::sqrt(2.0);
    checar(quase_igual(d.nota(preta, cinza), esperada), "Versão D: nota da camisa preta com a saia cinza");

    const std::vector<Candidata> lista = d.completar({preta}, "saia", 10);
    checar(!lista.empty() && lista[0].indice == cinza && quase_igual(lista[0].pontuacao, esperada),
           "Versão D: saia cinza primeiro");

    const int bota = base.indice_da_peca.at(8);
    const std::vector<Candidata> soma = d.completar({preta, bota}, "saia", 10);
    bool soma_certa = false;
    for (const Candidata& c : soma) {
        if (c.indice == cinza) soma_certa = quase_igual(c.pontuacao, d.nota(preta, cinza) + d.nota(bota, cinza));
    }
    checar(soma_certa, "Versão D: soma as notas com cada peça do look");

    const std::vector<LookMontado> looks = d.montar(preta, {"saia", "sapato"});
    bool montou = !looks.empty() && looks.size() <= 3;
    for (int i = 1; i < (int)looks.size(); i++) montou = montou && looks[i - 1].soma >= looks[i].soma;
    if (montou) {
        const std::vector<int>& l = looks[0].pecas;
        montou = l.size() == 3 && l[0] == preta &&
                 quase_igual(looks[0].soma, d.nota(l[0], l[1]) + d.nota(l[0], l[2]) + d.nota(l[1], l[2]));
    }
    checar(montou, "Montar: até 3 looks, em ordem, com a soma das duplas");

    d.trocar_pesos(1.0, 0.0, 0.0, 0.0);
    checar(quase_igual(d.nota(preta, cinza), 1.0), "Versão D: peso 0 desliga a parte");

    // nenhum look no treino: sem bonus de peca nem de grupo
    Base sem_treino = base;
    for (Look& look : sem_treino.looks) look.treino = false;
    GrafoLooks grafo_vazio;
    grafo_vazio.construir(sem_treino);
    VersaoD d2(sem_treino, attr, grafo_vazio, ConfigD{});
    checar(quase_igual(d2.nota(preta, cinza), 1.0 + BONUS_COR_NEUTRA), "Versão D: bônus só vem dos looks de treino");

    // L1 fora: a camisa preta nao tem look, entao vale o bonus do grupo (camisa grafite, L2)
    Base sem_l1 = base;
    sem_l1.looks[base.indice_do_look.at(1)].treino = false;
    GrafoLooks grafo_l1;
    grafo_l1.construir(sem_l1);
    VersaoD d3(sem_l1, attr, grafo_l1, ConfigD{});
    checar(d3.bonus_grupo(preta, cinza) > 0.0 && quase_igual(d3.nota(preta, cinza), 1.0 + BONUS_COR_NEUTRA + d3.bonus_grupo(preta, cinza)),
           "Versão D: usa o bônus do grupo quando o da peça é zero");

    ConfigD um_lado;
    um_lado.k = 1;
    um_lado.mutuo = false;
    ConfigD mutuo = um_lado;
    mutuo.mutuo = true;
    VersaoD lado(base, attr, grafo, um_lado);
    VersaoD mut(base, attr, grafo, mutuo);
    long long total_lado = 0;
    long long total_mut = 0;
    for (const auto& l : lado.arestas()) total_lado += l.size();
    for (const auto& l : mut.arestas()) total_mut += l.size();
    checar(total_lado > total_mut, "Versão D: basta um lado dá mais arestas que o mútuo");
}

void testar_d_contra_forca_bruta() {
    std::mt19937 gerador(42);
    std::bernoulli_distribution bit(0.3);
    std::uniform_int_distribution<int> forma(0, 4);
    const std::vector<std::string> cats = {"top", "saia", "sapato"};

    Base base;
    std::vector<Atrib> attr;
    for (int i = 0; i < 3000; i++) {
        Peca p;
        p.id = i;
        p.categoria = cats[i % 3];
        p.v = {0.0};
        base.pecas.push_back(p);
        Atrib a;
        for (int j = 0; j < 10; j++) a.g1.push_back(bit(gerador) ? 1.0 : 0.0);
        a.g1.push_back(forma(gerador) / 4.0);  // poucos valores pra forcar empate
        a.lab = {0.5, 0.5, 0.5};
        attr.push_back(a);
    }
    GrafoLooks grafo;
    grafo.construir(base);
    ConfigD cfg;
    cfg.k = 2;
    VersaoD d(base, attr, grafo, cfg);

    bool iguais = true;
    for (int i = 0; i < 1000 && iguais; i++) {
        const std::vector<Vizinho> x = d.vizinhos(i, cats[i % 3], 5);
        const std::vector<Vizinho> y = d.vizinhos_bruta(i, cats[i % 3], 5);
        const std::vector<Vizinho> x2 = d.vizinhos(i, cats[(i + 1) % 3], 5);
        const std::vector<Vizinho> y2 = d.vizinhos_bruta(i, cats[(i + 1) % 3], 5);
        iguais = x.size() == 5 && y.size() == 5 && x2.size() == 5 && y2.size() == 5;
        for (int j = 0; j < 5 && iguais; j++) iguais = quase_igual(x[j].dist2, y[j].dist2) && quase_igual(x2[j].dist2, y2[j].dist2);
    }
    checar(iguais, "Versão D: KD-tree por categoria igual à força bruta com empates");

    VersaoD outra(base, attr, grafo, cfg);
    checar(d.arestas() == outra.arestas(), "Versão D: arestas iguais em duas execuções");
}

void testar_pontos_iguais() {
    std::vector<Peca> pecas(50000);
    for (int i = 0; i < (int)pecas.size(); i++) {
        pecas[i].id = i;
        pecas[i].v = {1.0, 0.0, 1.0};
    }
    KDTree arvore(3);
    arvore.construir(pecas);
    int visitados = 0;
    arvore.mais_proximo({1.0, 0.0, 1.0}, visitados);
    checar(visitados < 100, "KD-tree: pontos iguais não degeneram a busca");
    checar(arvore.knn({1.0, 0.0, 1.0}, 5).size() == 5, "KD-tree: kNN com pontos iguais");
}

void testar_ajuste_popularidade() {
    Base base;
    for (int i = 0; i < 3; i++) {
        Peca p;
        p.id = i;
        p.categoria = "x";
        p.v = {(double)i};
        base.pecas.push_back(p);
    }
    Look a, b;
    a.pecas = {0, 1};
    b.pecas = {1, 2};
    base.looks = {a, b};
    GrafoLooks grafo;
    grafo.construir(base);
    const auto cont = grafo.bfs_2_passos(0, true);
    checar(quase_igual(cont.at(1), 1.0 / std::sqrt(2.0)), "BFS: ajuste divide pela raiz dos looks");
}

void testar_kruskal() {
    // ordem: 0-1 (1), 3-4 (1,5), 1-2 (2), 0-2 (5, fecha ciclo), 2-3 (10)
    std::vector<ArestaK> arestas = {{0, 1, 1.0}, {3, 4, 1.5}, {1, 2, 2.0}, {0, 2, 5.0}, {2, 3, 10.0}};
    UnionFind uf(5);
    std::vector<ArestaK> usadas;
    int grupos = kruskal(5, arestas, 1, uf, &usadas);
    double soma = 0.0;
    for (const ArestaK& a : usadas) soma += a.peso;
    checar(grupos == 1 && usadas.size() == 4 && quase_igual(soma, 14.5), "Kruskal: arvore minima com peso 14,5");
    checar(std::none_of(usadas.begin(), usadas.end(), [](const ArestaK& a) { return a.u == 0 && a.v == 2; }),
           "Kruskal: aresta que fecha ciclo fica de fora");

    UnionFind dois(5);
    grupos = kruskal(5, arestas, 2, dois);
    checar(grupos == 2 && dois.find(0) == dois.find(2) && dois.find(3) == dois.find(4) && dois.find(0) != dois.find(3),
           "Kruskal: para em 2 grupos sem a aresta mais cara");

    UnionFind solto(5);
    grupos = kruskal(5, {{0, 1, 1.0}, {2, 3, 1.0}}, 2, solto);
    checar(grupos == 3, "Kruskal: grafo desconexo para em 3 grupos quando pede 2");

    UnionFind vazio(1);
    checar(kruskal(1, {}, 20, vazio) == 1, "Kruskal: 1 peca e 1 grupo");

    checar(quase_igual(valor_estampa("lisa"), 0.0) && quase_igual(valor_estampa("floral"), 1.0) &&
               quase_igual(valor_estampa("desconhecido"), 0.5),
           "Grupos: estampa lisa 0, floral 1 e desconhecido 0,5");
}

void testar_pontos_grupos() {
    // 4 blusas (2 pretas, 2 brancas), 2 calcas e 1 chapeu sozinho na categoria
    Base base;
    const char* cats[] = {"top", "top", "top", "top", "pants", "pants", "hats"};
    const double L[] = {0.0, 0.0, 1.0, 1.0, 0.0, 1.0, 0.5};
    std::vector<Atrib> attr(7);
    for (int i = 0; i < 7; i++) {
        Peca p;
        p.id = i;
        p.categoria = cats[i];
        base.pecas.push_back(p);
        base.indice_da_peca[i] = i;
        attr[i].lab = {L[i], 0.5, 0.5};
        attr[i].estampa = "lisa";
    }
    for (std::vector<int> pecas : {std::vector<int>{0, 5}, {1, 5}, {2, 4}, {3, 4}}) {
        Look l;
        l.pecas = pecas;
        base.looks.push_back(l);
    }
    base.looks[3].treino = false;

    GrafoKnn g = montar_grafo_knn(base, attr);
    Grupos gr = agrupar(g, 2, 7);
    checar(gr.grupo[0] == gr.grupo[1] && gr.grupo[2] == gr.grupo[3] && gr.grupo[0] != gr.grupo[2],
           "Grupos: blusas pretas e brancas em 2 grupos");
    checar(gr.tamanho[gr.grupo[6]] == 1 && g.arestas[0].empty() && g.componentes[0] == 1,
           "Grupos: categoria com 1 peca fica com 1 grupo e sem aresta");

    PontosGrupos pg(base, gr);
    // 3 pares top-pants no treino. o look 3 e de teste e nao conta
    checar(pg.contagem(gr.grupo[0], gr.grupo[5]) == 2 && pg.contagem(gr.grupo[2], gr.grupo[4]) == 1,
           "Pontos de grupo: conta so os looks de treino");
    checar(quase_igual(pg.esperado(gr.grupo[0], gr.grupo[5]), 0.75), "Pontos de grupo: esperado 3 x 1/2 x 1/2");
    checar(quase_igual(pg.pontos(0, 5), std::log(3.0 / 1.75)) && quase_igual(pg.pontos(1, 4), std::log(1.0 / 1.75)),
           "Pontos de grupo: log((n + 1) / (esperado + 1))");
    checar(quase_igual(pg.pontos(0, 2), 0.0) && quase_igual(pg.pontos(6, 0), 0.0),
           "Pontos de grupo: mesma categoria e categoria sem par dao 0");
}

// as 4 variantes tem que dar os mesmos grupos que a classe do projeto
void testar_uf_variantes() {
    std::mt19937 gerador(3);
    const int n = 200;
    std::uniform_int_distribution<int> sorteio(0, n - 1);
    std::vector<std::pair<int, int>> unioes(150);
    for (auto& u : unioes) u = {sorteio(gerador), sorteio(gerador)};
    UnionFind ref(n);
    for (auto [a, b] : unioes) ref.unite(a, b);
    for (bool tam : {false, true}) {
        for (bool comp : {false, true}) {
            UFVariante u(n, tam, comp);
            bool ok = true;
            for (auto [a, b] : unioes) {
                bool juntos = u.find(a) == u.find(b);
                ok = ok && u.unite(a, b) == !juntos;
            }
            for (int a = 0; a < n; a++) {
                for (int b = a + 1; b < n; b++) ok = ok && ((u.find(a) == u.find(b)) == (ref.find(a) == ref.find(b)));
            }
            checar(ok, std::string("Union-Find variante: tamanho ") + (tam ? "sim" : "nao") + ", compressao " +
                           (comp ? "sim" : "nao") + " da os mesmos grupos");
        }
    }
    UFVariante u(3, false, false);
    checar(u.unite(0, 1) && !u.unite(1, 0) && u.find(2) == 2, "Union-Find variante: unir de novo devolve falso");
}

Floresta kruskal_teste(int n, std::vector<ArestaAGM> e) {
    ordenar_arestas_agm(e);
    return agm_kruskal(n, e);
}

void testar_prim_kruskal() {
    // quadrado 0-1-2-3 com diagonal. AGM: 1 + 2 + 3 = 6
    std::vector<ArestaAGM> e = {{0, 1, 1}, {1, 2, 2}, {2, 3, 3}, {3, 0, 4}, {0, 2, 5}};
    Floresta k = kruskal_teste(4, e);
    Floresta p = agm_prim(lista_adj_agm(4, e));
    checar(quase_igual(k.peso, 6.0) && k.arestas == 3, "Kruskal: quadrado com diagonal pesa 6");
    checar(quase_igual(p.peso, 6.0) && p.arestas == 3, "Prim: quadrado com diagonal pesa 6");

    // dois componentes e um vertice solto: floresta com n - 3 arestas
    e = {{0, 1, 2}, {1, 2, 1}, {0, 2, 3}, {3, 4, 5}};
    k = kruskal_teste(6, e);
    p = agm_prim(lista_adj_agm(6, e));
    checar(quase_igual(k.peso, 8.0) && k.arestas == 3 && quase_igual(p.peso, 8.0) && p.arestas == 3,
           "Prim e Kruskal: grafo desconexo da floresta com peso 8");

    checar(kruskal_teste(0, {}).arestas == 0 && agm_prim(ListaAdj()).arestas == 0, "Prim e Kruskal: grafo vazio");
    checar(agm_prim(lista_adj_agm(1, {})).arestas == 0, "Prim: um vertice so nao tem aresta");

    // grafo kNN de pontos sorteados, com empates de peso
    std::mt19937 gerador(5);
    std::uniform_int_distribution<int> coord(0, 9);
    std::vector<Peca> pecas(300);
    for (int i = 0; i < 300; i++) {
        pecas[i].id = i;
        pecas[i].v = {(double)coord(gerador), (double)coord(gerador)};
    }
    KDTree arvore(2);
    arvore.construir(pecas);
    std::vector<ArestaAGM> knn = grafo_knn_agm(pecas, arvore, 4);
    bool sem_repetida = true;
    for (std::size_t i = 1; i < knn.size(); i++) {
        sem_repetida = sem_repetida && knn[i].a < knn[i].b && !(knn[i].a == knn[i - 1].a && knn[i].b == knn[i - 1].b);
    }
    checar(sem_repetida, "Grafo kNN: cada aresta aparece uma vez");
    k = kruskal_teste(300, knn);
    p = agm_prim(lista_adj_agm(300, knn));
    checar(std::fabs(k.peso - p.peso) < 1e-6 && k.arestas == p.arestas, "Prim e Kruskal: mesmo peso no grafo kNN");
}

}  // namespace

void testar_config() {
    ConfigD cfg;
    checar(!ler_config_d("dados/exemplo/nao_existe.csv", cfg) && cfg.k == 5, "Config: sem arquivo não muda nada");
    const std::string caminho = "build/config_teste.csv";
    std::ofstream(caminho) << "k,mutuo,a,b,c,d\n20,0,0.5,0,0,1\n";
    checar(ler_config_d(caminho, cfg) && cfg.k == 20 && !cfg.mutuo && cfg.a == 0.5 && cfg.b == 0.0, "Config: lê k, mútuo e pesos");
}

int main() {
    Base base;
    std::string erro;
    if (!ler_base("dados/exemplo", base, erro)) {
        std::cerr << erro << "\nRode os testes na raiz do projeto, com make test.\n";
        return 1;
    }

    testar_leitura(base);
    testar_hit();
    testar_union_find();
    testar_kdtree_exemplo(base);
    testar_kdtree_contra_forca_bruta();
    testar_knn_mutuo();
    testar_bfs(base);
    testar_versao_c(base);
    testar_casos_de_borda(base);
    testar_ajuste_popularidade();
    testar_config();
    testar_regras();
    testar_versao_d(base);
    testar_d_contra_forca_bruta();
    testar_pontos_iguais();
    testar_regras_e();
    testar_versao_e();
    testar_fitb();
    testar_kruskal();
    testar_pontos_grupos();
    testar_uf_variantes();
    testar_prim_kruskal();

    std::cout << "\nResultado: " << aprovados << " OK e " << reprovados << " FALHOU\n";
    return reprovados > 0 ? 1 : 0;
}
