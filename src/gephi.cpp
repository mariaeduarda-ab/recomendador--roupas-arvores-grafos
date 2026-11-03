#include "gephi.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <random>
#include <unordered_map>

namespace {
constexpr unsigned SEMENTE_AMOSTRA = 42;
constexpr int MAX_GRUPOS_FATIA = 4;  // um grupo ocupa no maximo 1/4 do desenho
}

bool exportar_gephi(const Base& base, const std::vector<int>& grupos,
                    const std::vector<std::vector<Aresta>>& adjacencia, const std::string& pasta,
                    int limite) {
    if (grupos.size() != base.pecas.size() || adjacencia.size() != base.pecas.size()) return false;
    std::filesystem::create_directories(pasta);

    // sorteio solto quase nao pega as duas pontas de uma aresta e o desenho sai sem ligacoes.
    // por isso entram grupos inteiros, sem as pecas isoladas, comecando pelos maiores
    std::unordered_map<int, std::vector<int>> por_grupo;
    for (int i = 0; i < (int)grupos.size(); i++) {
        if (!adjacencia[i].empty()) por_grupo[grupos[i]].push_back(i);
    }
    std::vector<std::vector<int>> lista;
    for (auto& par : por_grupo) lista.push_back(par.second);
    std::sort(lista.begin(), lista.end(), [](const std::vector<int>& a, const std::vector<int>& b) {
        if (a.size() != b.size()) return a.size() > b.size();
        return a[0] < b[0];
    });
    std::mt19937 gerador(SEMENTE_AMOSTRA);
    std::vector<int> ordem;
    for (std::vector<int>& g : lista) {
        // um grupo gigante sozinho ocuparia o desenho todo
        std::shuffle(g.begin(), g.end(), gerador);
        int cabe = std::min({(int)g.size(), limite - (int)ordem.size(), limite / MAX_GRUPOS_FATIA});
        ordem.insert(ordem.end(), g.begin(), g.begin() + cabe);
        if ((int)ordem.size() >= limite) break;
    }

    std::vector<char> na_amostra(base.pecas.size(), 0);
    for (int i : ordem) na_amostra[i] = 1;

    std::ofstream nos(pasta + "/nodes.csv");
    nos << "Id,Label,Categoria,Grupo\n";
    for (int i : ordem) {
        const Peca& peca = base.pecas[i];
        const std::string rotulo = peca.nome.empty() ? std::to_string(peca.id) : peca.nome;
        nos << peca.id << ',' << rotulo << ',' << peca.categoria << ',' << grupos[i] << '\n';
    }

    std::ofstream arestas(pasta + "/edges.csv");
    arestas << "Source,Target,Weight,Type\n";
    for (int i : ordem) {
        for (const Aresta& aresta : adjacencia[i]) {
            // cada aresta aparece nas duas listas, grava uma vez so
            if (i < aresta.destino && na_amostra[aresta.destino]) {
                arestas << base.pecas[i].id << ',' << base.pecas[aresta.destino].id << ','
                        << aresta.peso << ",Undirected\n";
            }
        }
    }
    return (bool)nos && (bool)arestas;
}
