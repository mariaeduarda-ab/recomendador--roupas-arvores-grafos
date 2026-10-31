#include "gephi.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <random>

bool exportar_gephi(const Base& base, const std::vector<int>& grupos,
                    const std::vector<std::vector<Aresta>>& adjacencia, const std::string& pasta,
                    int limite) {
    if (grupos.size() != base.pecas.size() || adjacencia.size() != base.pecas.size()) return false;
    std::filesystem::create_directories(pasta);

    std::vector<int> ordem(base.pecas.size());
    std::iota(ordem.begin(), ordem.end(), 0);
    std::mt19937 gerador(42);
    std::shuffle(ordem.begin(), ordem.end(), gerador);
    if (static_cast<int>(ordem.size()) > limite) ordem.resize(limite);

    std::vector<char> na_amostra(base.pecas.size(), 0);
    for (const int i : ordem) na_amostra[i] = 1;

    std::ofstream nos(pasta + "/nodes.csv");
    nos << "Id,Label,Categoria,Grupo\n";
    for (const int i : ordem) {
        const Peca& peca = base.pecas[i];
        const std::string rotulo = peca.nome.empty() ? std::to_string(peca.id) : peca.nome;
        nos << peca.id << ',' << rotulo << ',' << peca.categoria << ',' << grupos[i] << '\n';
    }

    std::ofstream arestas(pasta + "/edges.csv");
    arestas << "Source,Target,Weight,Type\n";
    for (const int i : ordem) {
        for (const Aresta& aresta : adjacencia[i]) {
            // i < destino: cada aresta aparece duas vezes na lista, mas é gravada uma vez.
            if (i < aresta.destino && na_amostra[aresta.destino]) {
                arestas << base.pecas[i].id << ',' << base.pecas[aresta.destino].id << ','
                        << aresta.peso << ",Undirected\n";
            }
        }
    }
    return true;
}
