// Testes simples, sem framework externo.
// Os testes das estruturas falham até a dupla implementar cada parte. Isso é esperado.

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "avaliacao.hpp"
#include "grafo_looks.hpp"
#include "grafo_similaridade.hpp"
#include "kdtree.hpp"
#include "leitura.hpp"
#include "recomendador.hpp"
#include "union_find.hpp"

namespace {

int aprovados = 0;
int reprovados = 0;

void checar(bool condicao, const std::string& nome) {
    std::cout << (condicao ? "OK      " : "FALHOU  ") << nome << '\n';
    (condicao ? aprovados : reprovados)++;
}

bool quase_igual(double a, double b) { return std::fabs(a - b) < 1e-9; }

// As 6 primeiras peças do exemplo são os pontos A a F do guia.
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
        pecas[i].id = static_cast<int>(i);
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
        // Listas vazias não contam como iguais. Compara a distância por causa de empates.
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
        pecas[i].id = static_cast<int>(i);
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

}  // namespace

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

    std::cout << "\nResultado: " << aprovados << " OK e " << reprovados << " FALHOU\n";
    // Sempre devolve 0. Testes falhando são esperados enquanto as estruturas são esqueletos.
    return 0;
}
