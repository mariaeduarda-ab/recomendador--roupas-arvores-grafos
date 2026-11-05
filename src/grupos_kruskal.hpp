#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "atributos.hpp"
#include "leitura.hpp"
#include "union_find.hpp"

// Uso na versao F:
//   GrafoKnn g = montar_grafo_knn(base, attr);  // attr de pecas_v4.csv. nao olha os looks
//   Grupos gr = agrupar(g, K, base.pecas.size()); // K grupos por categoria (mais, se o grafo ja for desconexo)
//   PontosGrupos pg(base, gr);                  // conta so os looks com treino = true
//   pg.pontos(p, c);                            // p e c sao indices em base.pecas. mesma categoria da 0
//   pg.arestas();                               // grafo entre grupos com os pontos (Tarjan)
// As outras formas (agrupar_limitado, agrupar_mutuo, agrupar_por_chave) devolvem o mesmo Grupos.
// Na validacao, passe uma copia da base com os looks de validacao marcados treino = false.

constexpr int K_VIZINHOS_GRUPOS = 10;
// estampa diferente vale como delta E 10, umas 3 vezes a aresta media do grafo (delta E 3).
// com 0 os grupos ignoram a estampa e com 0,2 ou mais nenhum grupo mistura lisa com estampada
constexpr double PESO_ESTAMPA = 0.1;

struct ArestaK {
    int u = 0;  // u < v, indices dentro da categoria
    int v = 0;
    double peso = 0.0;
    bool mutua = false;  // u esta entre os k de v e v entre os k de u
};

struct GrafoKnn {
    std::vector<std::string> categorias;
    std::vector<std::vector<int>> membros;      // membros[c] = indices em base.pecas
    std::vector<std::vector<ArestaK>> arestas;  // por categoria, ja ordenadas pelo peso
    std::vector<int> componentes;               // componentes do grafo knn de cada categoria
};

struct Grupos {
    std::vector<int> grupo;            // grupo[i] da peca i, de 0 a tamanho.size() - 1
    std::vector<int> tamanho;          // pecas em cada grupo
    std::vector<int> cat_do_grupo;     // indice em GrafoKnn::categorias
    std::vector<int> por_categoria;    // quantos grupos ficaram em cada categoria
    std::vector<std::vector<ArestaK>> floresta;  // arestas usadas nas unioes, por categoria
};

// 1 com estampa, 0 lisa, 0,5 desconhecido
double valor_estampa(const std::string& estampa);

// Lab em unidade de verdade / 100 e a estampa com peso
std::vector<double> vetor_grupos(const Atrib& a, double peso_estampa);

// une as arestas na ordem ate restarem alvo grupos. devolve quantos grupos ficaram
int kruskal(int n, const std::vector<ArestaK>& arestas, int alvo, UnionFind& uf,
            std::vector<ArestaK>* usadas = nullptr);

GrafoKnn montar_grafo_knn(const Base& base, const std::vector<Atrib>& attr, int k = K_VIZINHOS_GRUPOS,
                          double peso_estampa = PESO_ESTAMPA);

Grupos agrupar(const GrafoKnn& grafo, int alvo, int n_pecas);

// como o kruskal, mas pula a uniao se o grupo novo passar de limite pecas
int kruskal_limitado(int n, const std::vector<ArestaK>& arestas, int alvo, int limite, UnionFind& uf,
                     std::vector<ArestaK>* usadas = nullptr);

// forma 1: kruskal com grupo de no maximo fracao das pecas da categoria (pelo menos 2)
Grupos agrupar_limitado(const GrafoKnn& grafo, int alvo, double fracao, int n_pecas);

// forma 2: union-find so nas arestas mutuas. floresta = as arestas mutuas
Grupos agrupar_mutuo(const GrafoKnn& grafo, int n_pecas);

// forma 3: mesma chave, mesmo grupo (chave[i] da peca i). nao usa as arestas
Grupos agrupar_por_chave(const GrafoKnn& grafo, const std::vector<std::string>& chave, int n_pecas);

// aresta do grafo entre grupos (supervertices), para o Tarjan
struct ParGrupos {
    int g = 0;  // g < h
    int h = 0;
    int n = 0;
    double pontos = 0.0;
};

class PontosGrupos {
public:
    PontosGrupos(const Base& base, const Grupos& grupos);

    double pontos(int p, int c) const;
    double pontos_entre(int g, int h) const;  // por grupo
    int contagem(int g, int h) const;
    double esperado(int g, int h) const;
    // pares de grupos que aparecem juntos em pelo menos min_n pares de pecas, ordenados por (g, h)
    std::vector<ParGrupos> arestas(int min_n = 1) const;

private:
    int n_cat_ = 0;
    std::vector<int> grupo_;
    std::vector<int> cat_;
    std::vector<double> fracao_;      // tamanho do grupo / pecas da categoria
    std::vector<double> pares_cat_;   // n_cat x n_cat, pares nos looks de treino
    std::unordered_map<long long, int> n_;
};

// comando grupos: roda cada K, grava resumo, csv e arquivos do Gephi
bool rodar_grupos(const Base& base, const std::string& pasta_dados, const std::vector<int>& lista_k,
                  const std::string& pasta_saida);
