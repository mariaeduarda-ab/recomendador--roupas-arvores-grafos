#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "articulacoes.hpp"
#include "avaliacao.hpp"
#include "avaliacao_d.hpp"
#include "avaliacao_e.hpp"
#include "avaliacao_fitb.hpp"
#include "avaliacao_justa.hpp"
#include "gephi.hpp"
#include "grafo_looks.hpp"
#include "grafo_similaridade.hpp"
#include "grupos_formas.hpp"
#include "grupos_kruskal.hpp"
#include "kdtree.hpp"
#include "leitura.hpp"
#include "pontos_atributos.hpp"
#include "recomendador.hpp"
#include "tempos_f.hpp"
#include "versao_d.hpp"
#include "versao_e.hpp"
#include "versao_f.hpp"
#include "pecas_novas.hpp"

namespace {

const std::string PASTA_RESULTADOS = "resultados";
const std::string ARQUIVO_CONFIG = "resultados/config_d.csv";
const std::string ARQUIVO_CONFIG_E = "resultados/config_e.csv";
const std::string TABELA_CORES = "regras/cores.csv";
const std::vector<std::string> CATEGORIAS_MONTAR = {"top", "pants", "shoes", "bag"};
constexpr int N_COMPLETAR = 10;

int mostrar_uso() {
    std::cerr << "Uso:\n"
                 "  ./recomendador parecidas <id_peca> <k>          k peças parecidas da mesma categoria\n"
                 "  ./recomendador completar <id_look> <categoria>  completa um look (versão C)\n"
                 "  ./recomendador avaliar                          Hit@k das versões e tempos\n"
                 "  ./recomendador avaliar_d                        compara todas as versões, com a D (demora)\n"
                 "  ./recomendador reavaliar                        popularidade, B, C e D com a config já escolhida\n"
                 "  ./recomendador vetor_d                          escolhe na validacao o vetor da D com pecas_v4.csv\n"
                 "  ./recomendador estampa_d                        valida a regra de estampa na D nova\n"
                 "  ./recomendador avaliar_e                        escolhe os pesos da E e roda o teste\n"
                 "  ./recomendador avaliar_fitb                     prova de múltipla escolha com todas as versões\n"
                 "  ./recomendador avaliar_justo [--validacao] [--novas] todas as versões na categoria inteira, 10 ordens\n"
                 "  ./recomendador completar_e <id_look> <categoria> completa um look (versão E)\n"
                 "  ./recomendador gephi <k>                        grupos com esse k e arquivos do Gephi\n"
                 "  ./recomendador validacao                        lista os looks de validacao\n"
                 "  ./recomendador grupos [K ...]                   grupos com Kruskal por categoria (padrão: 20 50 100)\n"
                 "  ./recomendador grupos_formas                    compara 3 formas de agrupar e escolhe uma\n"
                 "  ./recomendador completar_d <id_look> <categoria> completa um look (versão D)\n"
                 "  ./recomendador montar <id_peca> [categorias]    3 melhores looks a partir de uma peça\n"
                 "  ./recomendador pontos                           tabelas de pontos por atributo e diagnóstico na validação\n"
                 "  ./recomendador ajustar_novas                   variacoes para a peca nova, so na validacao\n"
                 "  ./recomendador tempos_f                         tempos das estruturas para 1 mil, 10 mil e 100 mil peças\n"
                 "  ./recomendador articulacoes                     Tarjan no grafo entre grupos (só análise)\n"
                 "  ./recomendador ajustar_f                        escolhe os pesos da F na validação (não monta o teste)\n"
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

// outra base grava em subpasta, senao o exemplo sobrescreve as tabelas da base real
std::string pasta_de_saida(const std::string& pasta_dados) {
    std::filesystem::path p = std::filesystem::path(pasta_dados).lexically_normal();
    if (!p.has_filename()) p = p.parent_path();
    if (p == "dados") return PASTA_RESULTADOS;
    return PASTA_RESULTADOS + "/" + p.filename().string();
}

ConfigD config_escolhida() {
    ConfigD cfg;
    if (!ler_config_d(ARQUIVO_CONFIG, cfg)) {
        std::cerr << "Aviso: sem " << ARQUIVO_CONFIG << ". Usando a config inicial (k = 5, mútuo).\n";
    }
    return cfg;
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
    const bool formato_ok = ((comando == "parecidas" || comando == "completar" || comando == "completar_d" || comando == "completar_e") && argumentos.size() == 3) ||
                            (comando == "montar" && argumentos.size() >= 2) ||
                            ((comando == "avaliar" || comando == "avaliar_d" || comando == "avaliar_e" || comando == "avaliar_fitb" || comando == "reavaliar" || comando == "vetor_d" || comando == "estampa_d" || comando == "validacao") && argumentos.size() == 1) ||
                            (comando == "gephi" && argumentos.size() == 2) || comando == "grupos" ||
                            ((comando == "tempos_f" || comando == "pontos" || comando == "grupos_formas" || comando == "ajustar_f" || comando == "ajustar_novas" || comando == "articulacoes") && argumentos.size() == 1) ||
                            (comando == "avaliar_justo" && argumentos.size() <= 3 &&
                             std::all_of(argumentos.begin() + 1, argumentos.end(),
                                         [](const std::string& a) { return a == "--validacao" || a == "--novas"; }));
    if (!formato_ok) return mostrar_uso();

    // usa os subconjuntos pecas_N.csv, entao nao precisa ler a base inteira
    if (comando == "tempos_f") return medir_tempos_f(pasta_dados, pasta_de_saida(pasta_dados)) ? 0 : 1;

    const std::string saida = pasta_de_saida(pasta_dados);

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

    // o catalogo de avaliacao do CLIP precisa dos mesmos looks de validacao sorteados aqui
    if (comando == "validacao") {
        std::cout << "look_id\n";
        for (int l : sortear_validacao(base)) std::cout << base.looks[l].id << '\n';
        return 0;
    }

    if (comando == "grupos") {
        std::vector<int> lista = {20, 50, 100};
        if (argumentos.size() > 1) lista.clear();
        for (std::size_t i = 1; i < argumentos.size(); i++) {
            int k = 0;
            if (!ler_inteiro(argumentos[i], k) || k <= 0) return mostrar_uso();
            lista.push_back(k);
        }
        return rodar_grupos(base, pasta_dados, lista, saida) ? 0 : 1;
    }
    if (comando == "grupos_formas") return comparar_formas(base, pasta_dados, saida) ? 0 : 1;
    if (comando == "pontos") return rodar_pontos(base, pasta_dados, saida) ? 0 : 1;
    if (comando == "ajustar_novas") {
        return ajustar_novas(base, pasta_dados, saida) ? 0 : 1;
    }
    if (comando == "ajustar_f") return ajustar_f(base, pasta_dados, saida) ? 0 : 1;
    if (comando == "articulacoes") return rodar_articulacoes(base, pasta_dados, saida) ? 0 : 1;

    KDTree arvore((int)base.pecas[0].v.size());
    arvore.construir(base.pecas);
    GrafoLooks grafo_looks;
    grafo_looks.construir(base);
    Recomendador rec(base, arvore, grafo_looks, ajuste);

    if (comando == "avaliar_d") {
        return avaliar_d(base, pasta_dados, saida) ? 0 : 1;
    }
    if (comando == "avaliar_e") {
        return avaliar_e(base, pasta_dados, TABELA_CORES, saida) ? 0 : 1;
    }
    if (comando == "avaliar_justo") {
        const auto tem = [&](const char* o) { return std::find(argumentos.begin(), argumentos.end(), o) != argumentos.end(); };
        return avaliar_justo(base, pasta_dados, tem("--validacao"), saida, tem("--novas")) ? 0 : 1;
    }
    if (comando == "avaliar_fitb") {
        return avaliar_fitb(base, pasta_dados, TABELA_CORES, saida) ? 0 : 1;
    }
    if (comando == "completar_e") {
        std::vector<Atrib> attr;
        std::vector<AtribE> attr_e;
        TabelaCores cores;
        ConfigE cfg;
        int id_look = 0;
        if (!ler_inteiro(argumentos[1], id_look) || !base.indice_do_look.count(id_look)) return mostrar_uso();
        if (!ler_atributos_da_pasta(pasta_dados, base, attr, erro) || !cores.ler(TABELA_CORES, erro) ||
            !ler_atributos_e(pasta_dados + "/pecas_v3.csv", base, cores, attr_e, erro)) {
            std::cerr << erro << '\n';
            return 1;
        }
        ler_config_e(ARQUIVO_CONFIG_E, cfg);
        VersaoE versao(base, attr, attr_e, cores, grafo_looks);
        const Look& look = base.looks[base.indice_do_look.at(id_look)];
        for (const Candidata& c : versao.completar(look.pecas, argumentos[2], cfg, N_COMPLETAR)) {
            std::cout << base.pecas[c.indice].id << ' ' << c.pontuacao << '\n';
        }
        return 0;
    }
    if (comando == "vetor_d") {
        return escolher_vetor_d(base, pasta_dados, config_escolhida(), saida) ? 0 : 1;
    }
    if (comando == "estampa_d") {
        return validar_estampa_d(base, pasta_dados, config_escolhida(), saida) ? 0 : 1;
    }
    if (comando == "reavaliar") {
        return reavaliar(base, pasta_dados, config_escolhida(), saida) ? 0 : 1;
    }

    if (comando == "completar_d" || comando == "montar") {
        std::vector<Atrib> attr;
        if (!ler_atributos_da_pasta(pasta_dados, base, attr, erro)) {
            std::cerr << erro << '\n';
            return 1;
        }
        int id = 0;
        if (!ler_inteiro(argumentos[1], id)) return mostrar_uso();
        VersaoD versao(base, attr, grafo_looks, config_escolhida());

        if (comando == "completar_d") {
            const auto look = base.indice_do_look.find(id);
            if (look == base.indice_do_look.end()) {
                std::cerr << "Look " << id << " não encontrado.\n";
                return 1;
            }
            const std::string& categoria = argumentos[2];
            std::vector<int> pecas = base.looks[look->second].pecas;
            pecas.erase(std::remove_if(pecas.begin(), pecas.end(),
                                       [&](int p) { return base.pecas[p].categoria == categoria; }),
                        pecas.end());
            const std::vector<Candidata> lista = versao.completar(pecas, categoria, N_COMPLETAR);
            if (lista.empty()) std::cout << "Nenhuma candidata de " << categoria << " para esse look.\n";
            imprimir_lista(base, lista);
            return 0;
        }

        const auto peca = base.indice_da_peca.find(id);
        if (peca == base.indice_da_peca.end()) {
            std::cerr << "Peça " << id << " não encontrada.\n";
            return 1;
        }
        std::vector<std::string> cats(argumentos.begin() + 2, argumentos.end());
        if (cats.empty()) cats = CATEGORIAS_MONTAR;
        cats.erase(std::remove(cats.begin(), cats.end(), base.pecas[peca->second].categoria), cats.end());
        const std::vector<LookMontado> looks = versao.montar(peca->second, cats);
        for (const LookMontado& l : looks) {
            std::cout << "soma " << l.soma << ':';
            for (int p : l.pecas) std::cout << ' ' << base.pecas[p].id;
            std::cout << '\n';
        }
        if (looks.empty() || looks[0].pecas.size() < 2) std::cout << "Essa peça não tem arestas nas categorias pedidas.\n";
    } else if (comando == "parecidas") {
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
        avaliar_hit(base, rec, grafo_looks, saida);
        medir_tempos(pasta_dados, saida);
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
        if (exportar_gephi(base, grupos, grafo.adjacencia(), saida)) {
            std::cout << "Arquivos em " << saida << "/nodes.csv e edges.csv\n";
        } else {
            std::cerr << "Não foi possível exportar para o Gephi.\n";
            return 1;
        }
    }
    return 0;
}
