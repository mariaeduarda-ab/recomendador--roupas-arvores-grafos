#pragma once

#include <string>
#include <vector>

#include "kdtree.hpp"
#include "peca.hpp"

// nome proprio pq grafo_similaridade.hpp ja tem uma Aresta com so destino
struct ArestaAGM {
    int a = -1;
    int b = -1;
    double peso = 0.0;
};

struct Floresta {
    double peso = 0.0;
    int arestas = 0;
};

// so pra medir: liga e desliga cada tecnica. o projeto usa union_find.hpp
class UFVariante {
public:
    UFVariante(int n, bool por_tamanho, bool compressao);
    int find(int x);
    bool unite(int a, int b);

private:
    std::vector<int> pai;
    std::vector<int> tam;
    bool por_tamanho_;
    bool compressao_;
};

// arestas sem repeticao: a-b entra uma vez mesmo se b tambem achar a
std::vector<ArestaAGM> grafo_knn_agm(const std::vector<Peca>& pecas, const KDTree& arvore, int k);

void ordenar_arestas_agm(std::vector<ArestaAGM>& arestas);

// espera as arestas ja ordenadas, assim o tempo da ordenacao sai separado
Floresta agm_kruskal(int n, const std::vector<ArestaAGM>& ordenadas);

using ListaAdj = std::vector<std::vector<std::pair<int, double>>>;
ListaAdj lista_adj_agm(int n, const std::vector<ArestaAGM>& arestas);

// recomeca em cada vertice ainda fora da arvore, entao cobre todos os componentes
Floresta agm_prim(const ListaAdj& adj);

// grava resultados/tempos_f/*.csv. usa pecas_N.csv e looks_N.csv
bool medir_tempos_f(const std::string& pasta_dados, const std::string& pasta_saida);
