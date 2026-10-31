#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "avaliacao.hpp"
#include "gephi.hpp"
#include "grafo_looks.hpp"
#include "grafo_similaridade.hpp"
#include "kdtree.hpp"
#include "leitura.hpp"
#include "recomendador.hpp"

namespace {

const std::string PASTA_RESULTADOS = "resultados";
constexpr int N_COMPLETAR = 10;

int mostrar_uso() {
    std::cerr << "Uso:\n"
                 "  ./recomendador parecidas <id_peca> <k>          k peças parecidas da mesma categoria\n"
                 "  ./recomendador completar <id_look> <categoria>  completa um look (versão C)\n"
                 "  ./recomendador avaliar                          Hit@k das versões e tempos\n"
                 "  ./recomendador gephi <k>                        grupos com esse k e arquivos do Gephi\n"
                 "Opção:\n"
                 "  --dados <pasta>                                 pasta dos CSVs (padrão: dados)\n"
                 "  --ajuste                                        divide a contagem pela popularidade da peça\n";
    return 1;
}

bool ler_inteiro(const std::string& texto, int& valor) {
    try {
        std::size_t usados = 0;
        valor = std::stoi(texto, &usados);
        return usados == texto.size();
    } catch (const std::exception&) {
        return false;
    }
}

void imprimir_lista(const Base& base, const std::vector<Candidata>& lista) {
    for (const Candidata& candidata : lista) {
        const Peca& peca = base.pecas[candidata.indice];
        std::cout << peca.id << ' ' << peca.categoria;
        if (!peca.nome.empty()) std::cout << ' ' << peca.nome;
        std::cout << ' ' << candidata.pontuacao << '\n';
    }
}

void imprimir_resumo(const std::vector<int>& grupos, int k) {
    ResumoGrupos resumo = resumir_grupos(grupos);
    std::cout << "k = " << k << ": " << resumo.quantidade << " grupos. Maior grupo com " << resumo.maior
              << " peças.\n";
}

}  // namespace

int main(int argc, char** argv) {
    std::string pasta_dados = "dados";
    bool ajuste = false;
    std::vector<std::string> argumentos;
    for (int i = 1; i < argc; i++) {
        const std::string atual = argv[i];
        if (atual == "--dados") {
            if (i + 1 >= argc) return mostrar_uso();
            pasta_dados = argv[++i];
        } else if (atual == "--ajuste") {
            ajuste = true;
        } else {
            argumentos.push_back(atual);
        }
    }
    if (argumentos.empty()) return mostrar_uso();
    const std::string comando = argumentos[0];
    const bool formato_ok = ((comando == "parecidas" || comando == "completar") && argumentos.size() == 3) ||
                            (comando == "avaliar" && argumentos.size() == 1) ||
                            (comando == "gephi" && argumentos.size() == 2);
    if (!formato_ok) return mostrar_uso();

    Base base;
    std::string erro;
    if (!ler_base(pasta_dados, base, erro)) {
        std::cerr << erro << '\n';
        return 1;
    }
    if (base.pecas.empty()) {
        std::cerr << "A base não tem peças.\n";
        return 1;
    }

    KDTree arvore((int)base.pecas[0].v.size());
    arvore.construir(base.pecas);
    GrafoLooks grafo_looks;
    grafo_looks.construir(base);
    Recomendador rec(base, arvore, grafo_looks, ajuste);

    if (comando == "parecidas") {
        int id = 0;
        int k = 0;
        if (!ler_inteiro(argumentos[1], id) || !ler_inteiro(argumentos[2], k) || k <= 0) return mostrar_uso();
        const auto peca = base.indice_da_peca.find(id);
        if (peca == base.indice_da_peca.end()) {
            std::cerr << "Peça " << id << " não encontrada.\n";
            return 1;
        }
        const std::vector<Candidata> lista = rec.parecidas(peca->second, k);
        if (lista.empty()) std::cout << "Nenhuma outra peça nessa categoria.\n";
        imprimir_lista(base, lista);
    } else if (comando == "completar") {
        int id = 0;
        if (!ler_inteiro(argumentos[1], id)) return mostrar_uso();
        const auto look = base.indice_do_look.find(id);
        if (look == base.indice_do_look.end()) {
            std::cerr << "Look " << id << " não encontrado.\n";
            return 1;
        }
        const std::string& categoria = argumentos[2];
        // a peca que falta nao pode ser ponto de partida
        std::vector<int> pecas = base.looks[look->second].pecas;
        pecas.erase(std::remove_if(pecas.begin(), pecas.end(),
                                   [&](int p) { return base.pecas[p].categoria == categoria; }),
                    pecas.end());
        const std::vector<Candidata> lista = rec.versao_c(pecas, categoria, N_COMPLETAR);
        if (lista.empty()) std::cout << "Nenhuma candidata de " << categoria << " para esse look.\n";
        imprimir_lista(base, lista);
    } else if (comando == "avaliar") {
        avaliar_hit(base, rec, grafo_looks, PASTA_RESULTADOS);
        medir_tempos(pasta_dados, PASTA_RESULTADOS);
        for (int k : {5, 10, 15}) {
            GrafoSimilaridade grafo;
            grafo.construir(base.pecas, arvore, k);
            imprimir_resumo(grafo.grupos(), k);
        }
    } else {
        int k = 0;
        if (!ler_inteiro(argumentos[1], k) || k <= 0) return mostrar_uso();
        GrafoSimilaridade grafo;
        grafo.construir(base.pecas, arvore, k);
        const std::vector<int> grupos = grafo.grupos();
        imprimir_resumo(grupos, k);
        if (exportar_gephi(base, grupos, grafo.adjacencia(), PASTA_RESULTADOS)) {
            std::cout << "Arquivos em " << PASTA_RESULTADOS << "/nodes.csv e edges.csv\n";
        } else {
            std::cerr << "Não foi possível exportar para o Gephi.\n";
            return 1;
        }
    }
    return 0;
}
