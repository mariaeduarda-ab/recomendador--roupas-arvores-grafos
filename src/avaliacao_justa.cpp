#include "avaliacao_justa.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <sstream>

#include "atributos.hpp"
#include "avaliacao.hpp"
#include "avaliacao_fitb.hpp"
#include "grafo_looks.hpp"
#include "kdtree.hpp"
#include "recomendador.hpp"
#include "versao_d.hpp"
#include "pecas_novas.hpp"
#include "versao_f.hpp"

namespace {

constexpr unsigned SEMENTE_PERGUNTAS_VAL = 11;  // as mesmas do avaliar_fitb
constexpr unsigned SEMENTE_PERGUNTAS_TESTE = 13;
constexpr unsigned SEMENTE_BOOTSTRAP = 2026;
const int METRICAS[4] = {1, 5, 10, 0};  // 0 = FITB
const char* NOMES_METRICAS[4] = {"hit1", "hit5", "hit10", "fitb"};

Consultas montar(const Base& base, const std::vector<int>& looks, unsigned semente_perguntas) {
    Consultas c;
    const std::vector<Escondida> esc = sortear_escondidas(base, looks);
    std::vector<Pergunta> perguntas = montar_perguntas(base, esc, semente_perguntas);
    std::unordered_map<int, const Pergunta*> pergunta_do_look;
    for (const Pergunta& p : perguntas) pergunta_do_look[p.escondida.look] = &p;

    for (const Escondida& e : esc) {
        c.indice_look.push_back(e.look);
        c.alvo.push_back(base.looks[e.look].pecas[e.pos]);
        c.look.push_back(sem_escondida(e, base));
        std::vector<int> erradas;
        auto it = pergunta_do_look.find(e.look);
        if (it != pergunta_do_look.end()) {
            for (int o : it->second->opcoes) {
                if (o != it->second->certa) erradas.push_back(o);
            }
        }
        c.erradas.push_back(erradas);
    }
    for (int i = 0; i < (int)base.pecas.size(); i++) c.por_cat[base.pecas[i].categoria].push_back(i);

    // sortear a ordem das pecas e o mesmo que embaralhar as linhas do catalogo
    int n = (int)base.pecas.size();
    c.ordem.assign(N_ORDENS, std::vector<int>(n));
    for (int s = 0; s < N_ORDENS; s++) {
        std::vector<int> perm(n);
        std::iota(perm.begin(), perm.end(), 0);
        std::mt19937 gerador(s + 1);
        std::shuffle(perm.begin(), perm.end(), gerador);
        for (int i = 0; i < n; i++) c.ordem[s][perm[i]] = i;
    }
    return c;
}

std::string num(double x, int casas) {
    std::ostringstream o;
    o << std::fixed << std::setprecision(casas) << x;
    std::string s = o.str();
    std::replace(s.begin(), s.end(), '.', ',');
    if (!s.empty() && s[0] == '-') s = "−" + s.substr(1);
    return s;
}

// hit em % com 3 casas: os valores ficam abaixo de 1%
std::string pct(double x, int k) {
    return num(100.0 * x, k == 0 ? 2 : 3) + "%";
}

struct Resumo {
    double media = 0.0;
    double min = 0.0;
    double max = 0.0;
};

Resumo resumir(const Posicoes& p, int k) {
    Resumo r;
    r.min = 2.0;
    r.max = -1.0;
    for (int s = 0; s < N_ORDENS; s++) {
        double t = taxa(p, k, s);
        r.media += t / N_ORDENS;
        r.min = std::min(r.min, t);
        r.max = std::max(r.max, t);
    }
    return r;
}

std::string conclusao(const std::string& x, const std::string& y, const Intervalo& i) {
    if (i.min > 0) return x + " ganha de " + y + " (diferença real)";
    if (i.max < 0) return x + " perde de " + y + " (diferença real)";
    return "pode ser sorte";
}

// confere com a lista gravada pelo avaliar_d em 31/10
bool confere_escondidas(const Base& base, const Consultas& c, const std::string& caminho, std::string& msg) {
    std::ifstream arq(caminho);
    if (!arq) {
        msg = "sem " + caminho + " para conferir";
        return false;
    }
    std::string linha;
    std::getline(arq, linha);
    std::size_t q = 0;
    while (std::getline(arq, linha)) {
        if (linha.empty()) continue;
        std::size_t virgula = linha.find(',');
        if (virgula == std::string::npos || q >= c.alvo.size()) {
            msg = "linha " + std::to_string(q + 2) + " de " + caminho + " não bate";
            return false;
        }
        int look_id = std::stoi(linha.substr(0, virgula));
        int peca_id = std::stoi(linha.substr(virgula + 1));
        if (base.looks[c.indice_look[q]].id != look_id || base.pecas[c.alvo[q]].id != peca_id) {
            msg = "consulta " + std::to_string(q + 1) + " diferente de " + caminho;
            return false;
        }
        q++;
    }
    if (q != c.alvo.size()) {
        msg = caminho + " tem " + std::to_string(q) + " linhas e aqui são " + std::to_string(c.alvo.size());
        return false;
    }
    msg = "as " + std::to_string(q) + " escondidas batem com " + caminho;
    return true;
}

double esperado_aleatorio(const Base& base, const Consultas& c, int k) {
    if (k == 0) return 0.25;
    double soma = 0.0;
    for (int q = 0; q < (int)c.alvo.size(); q++) {
        double m = (double)candidatas(base, c, q).size();
        soma += std::min((double)k, m) / m;
    }
    return c.alvo.empty() ? 0.0 : soma / c.alvo.size();
}

// taxa so nas consultas com dentro[q] = true. k = 0 da o FITB
double taxa_em(const Posicoes& p, int k, int s, const std::vector<bool>& dentro) {
    double soma = 0.0;
    int n = 0;
    for (int q = 0; q < (int)p.pos.size(); q++) {
        if (!dentro[q]) continue;
        if (k == 0) {
            if (p.fitb[q][s] < 0) continue;
            soma += p.fitb[q][s];
        } else {
            soma += p.pos[q][s] < k;
        }
        n++;
    }
    return n > 0 ? soma / n : 0.0;
}

std::vector<double> filtrar(const std::vector<double>& v, const std::vector<bool>& dentro) {
    std::vector<double> r;
    for (int q = 0; q < (int)v.size(); q++) {
        if (dentro[q]) r.push_back(v[q]);
    }
    return r;
}

}  // namespace

Consultas consultas_teste(const Base& base) {
    std::vector<int> looks;
    for (int l = 0; l < (int)base.looks.size(); l++) {
        if (!base.looks[l].treino) looks.push_back(l);
    }
    return montar(base, looks, SEMENTE_PERGUNTAS_TESTE);
}

Base base_sem_validacao(const Base& base) {
    Base sem_val = base;
    for (int l : sortear_validacao(base)) sem_val.looks[l].treino = false;
    return sem_val;
}

Consultas consultas_validacao(const Base& base) {
    // o FITB sorteia as erradas pela flag treino, entao as perguntas saem da base sem validacao
    return montar(base_sem_validacao(base), sortear_validacao(base), SEMENTE_PERGUNTAS_VAL);
}

std::vector<int> candidatas(const Base& base, const Consultas& c, int q) {
    const std::vector<int>& look = c.look[q];
    std::vector<int> res;
    auto it = c.por_cat.find(base.pecas[c.alvo[q]].categoria);
    if (it == c.por_cat.end()) return res;
    res.reserve(it->second.size());
    for (int p : it->second) {
        if (p == c.alvo[q] || std::find(look.begin(), look.end(), p) == look.end()) res.push_back(p);
    }
    return res;
}

Notas de_lista(const Base& base, const Lista& gerar) {
    return [&base, gerar](int, const std::vector<int>& look, const std::vector<int>& cand, std::vector<double>& nota) {
        if (cand.empty()) return;
        std::unordered_map<int, double> lista;
        for (const Candidata& x : gerar(look, base.pecas[cand[0]].categoria)) lista[x.indice] = x.pontuacao;
        for (std::size_t i = 0; i < cand.size(); i++) {
            auto it = lista.find(cand[i]);
            if (it != lista.end()) nota[i] = it->second;
        }
    };
}

Posicoes avaliar_notas(const Base& base, const Consultas& c, const std::string& nome, const Notas& f,
                       const std::vector<double>* desempate) {
    Posicoes res;
    res.nome = nome;
    std::vector<double> nota;
    for (int q = 0; q < (int)c.alvo.size(); q++) {
        const int alvo = c.alvo[q];
        const std::vector<int> cand = candidatas(base, c, q);
        nota.assign(cand.size(), 0.0);
        f(q, c.look[q], cand, nota);

        double na = 0.0;
        for (std::size_t i = 0; i < cand.size(); i++) {
            if (cand[i] == alvo) na = nota[i];
        }
        const double da = desempate ? (*desempate)[alvo] : 0.0;
        // rank = notas maiores + empatadas que vem antes na ordem. nao precisa ordenar a categoria
        int maiores = 0;
        std::vector<int> empate;
        for (std::size_t i = 0; i < cand.size(); i++) {
            if (nota[i] > na) {
                maiores++;
            } else if (nota[i] == na && cand[i] != alvo) {
                double d = desempate ? (*desempate)[cand[i]] : 0.0;
                if (d > da) maiores++;
                else if (d == da) empate.push_back(cand[i]);
            }
        }
        std::array<int, N_ORDENS> pos;
        for (int s = 0; s < N_ORDENS; s++) {
            int antes = 0;
            for (int p : empate) antes += c.ordem[s][p] < c.ordem[s][alvo];
            pos[s] = maiores + antes;
        }

        std::array<signed char, N_ORDENS> fitb;
        fitb.fill(c.erradas[q].empty() ? -1 : 1);
        for (int o : c.erradas[q]) {
            auto it = std::find(cand.begin(), cand.end(), o);
            double no = it != cand.end() ? nota[it - cand.begin()] : -INFINITY;
            double d = desempate ? (*desempate)[o] : 0.0;
            for (int s = 0; s < N_ORDENS; s++) {
                bool antes = d > da || (d == da && c.ordem[s][o] < c.ordem[s][alvo]);
                if (no > na || (no == na && antes)) fitb[s] = 0;
            }
        }

        res.tam.push_back((int)cand.size());
        res.empatadas.push_back((int)empate.size());
        res.pos.push_back(pos);
        res.fitb.push_back(fitb);
    }
    return res;
}

double taxa(const Posicoes& p, int k, int s) {
    int acertos = 0;
    int total = 0;
    for (std::size_t q = 0; q < p.pos.size(); q++) {
        if (k == 0) {
            if (p.fitb[q][s] < 0) continue;
            acertos += p.fitb[q][s];
        } else {
            acertos += p.pos[q][s] < k;
        }
        total++;
    }
    return total > 0 ? (double)acertos / total : 0.0;
}

std::vector<double> por_consulta(const Posicoes& p, int k) {
    std::vector<double> res(p.pos.size(), 0.0);
    for (std::size_t q = 0; q < p.pos.size(); q++) {
        if (k == 0 && p.fitb[q][0] < 0) {
            res[q] = -1.0;
            continue;
        }
        for (int s = 0; s < N_ORDENS; s++) res[q] += (k == 0 ? p.fitb[q][s] : p.pos[q][s] < k) / (double)N_ORDENS;
    }
    return res;
}

Intervalo bootstrap(const std::vector<double>& x, const std::vector<double>& y) {
    Intervalo res;
    // consulta sem pergunta de FITB vem com -1 e nao entra no sorteio
    std::vector<double> d;
    for (std::size_t i = 0; i < x.size() && i < y.size(); i++) {
        if (x[i] >= 0 && y[i] >= 0) d.push_back(x[i] - y[i]);
    }
    int n = (int)d.size();
    if (n == 0) return res;
    res.dif = std::accumulate(d.begin(), d.end(), 0.0) / n;

    // mesma semente em toda comparacao: todas usam as mesmas 1.000 amostras
    std::mt19937 gerador(SEMENTE_BOOTSTRAP);
    std::uniform_int_distribution<int> sorteio(0, n - 1);
    std::vector<double> medias(N_BOOTSTRAP);
    for (int b = 0; b < N_BOOTSTRAP; b++) {
        double soma = 0.0;
        for (int i = 0; i < n; i++) soma += d[sorteio(gerador)];
        medias[b] = soma / n;
    }
    std::sort(medias.begin(), medias.end());
    res.min = medias[(int)(0.025 * N_BOOTSTRAP)];
    res.max = medias[(int)(0.975 * N_BOOTSTRAP) - 1];
    return res;
}

bool avaliar_justo(const Base& base, const std::string& pasta_dados, bool validacao, const std::string& pasta_saida,
                   bool novas) {
    std::vector<Atrib> attr;
    std::vector<Atrib> attr_v4;
    std::string erro;
    if (!ler_atributos_da_pasta(pasta_dados, base, attr, erro)) {
        std::cerr << erro << '\n';
        return false;
    }
    // D nova: pecas_v4.csv com os grupos que o vetor_d escolheu na validacao
    bool usa[3] = {true, true, true};
    const bool tem_v4 = ler_atributos(pasta_dados + "/pecas_v4.csv", base, attr_v4, erro);
    {
        std::ifstream arq(pasta_saida + "/vetor_d.csv");
        std::string cab;
        std::getline(arq, cab);
        int e = 0, o = 0, f = 0;
        char v;
        if (arq >> e >> v >> o >> v >> f) {
            usa[0] = e != 0;
            usa[1] = o != 0;
            usa[2] = f != 0;
        } else if (tem_v4) {
            std::cerr << "Aviso: sem " << pasta_saida << "/vetor_d.csv. A D nova usa os 3 grupos.\n";
        }
    }
    if (tem_v4) attr_v4 = com_grupos(attr_v4, usa);
    ConfigD cfg;
    if (!ler_config_d(pasta_saida + "/config_d.csv", cfg)) {
        std::cerr << "Aviso: sem " << pasta_saida << "/config_d.csv. A D usa a config inicial.\n";
    }
    std::filesystem::create_directories(pasta_saida);

    // na validacao a base perde os looks de validacao: grafo, popularidade e bonus nao veem esses looks
    const Base b = validacao ? base_sem_validacao(base) : base;
    const Consultas c = validacao ? consultas_validacao(base) : consultas_teste(base);
    std::string conferencia;
    if (!validacao) {
        confere_escondidas(base, c, pasta_saida + "/escondidas.csv", conferencia);
        std::cout << "Escondidas: " << conferencia << std::endl;
    }

    KDTree arvore((int)b.pecas[0].v.size());
    arvore.construir(b.pecas);
    GrafoLooks grafo;
    grafo.construir(b);
    Recomendador rec(b, arvore, grafo);
    const int tudo = (int)b.pecas.size();

    std::vector<Posicoes> res;
    const auto rodar_versao = [&](const std::string& nome, const Notas& f, const std::vector<double>* desempate = nullptr) {
        auto ini = std::chrono::steady_clock::now();
        res.push_back(avaliar_notas(b, c, nome, f, desempate));
        double seg = std::chrono::duration<double>(std::chrono::steady_clock::now() - ini).count();
        std::cout << nome << ": Hit@10 = " << resumir(res.back(), 10).media << ", FITB = " << resumir(res.back(), 0).media
                  << " (" << seg << " s)" << std::endl;
    };

    rodar_versao("Aleatória", [](int, const std::vector<int>&, const std::vector<int>&, std::vector<double>&) {});
    rodar_versao("Popularidade", [&](int, const std::vector<int>&, const std::vector<int>& cand, std::vector<double>& nota) {
        for (std::size_t i = 0; i < cand.size(); i++) nota[i] = grafo.num_looks(cand[i]);
    });
    // A em todas as pecas: a mesma nota do knn, sem cortar a lista
    rodar_versao("A", [&](int, const std::vector<int>& look, const std::vector<int>& cand, std::vector<double>& nota) {
        if (look.empty()) return;
        std::vector<double> media(b.pecas[look[0]].v.size(), 0.0);
        for (int p : look) {
            for (std::size_t i = 0; i < media.size(); i++) media[i] += b.pecas[p].v[i];
        }
        for (double& x : media) x /= look.size();
        for (std::size_t i = 0; i < cand.size(); i++) nota[i] = 1.0 / (1.0 + std::sqrt(distancia2(media, b.pecas[cand[i]].v)));
    });
    rodar_versao("B", de_lista(b, [&](const std::vector<int>& l, const std::string& cat) { return rec.versao_b(l, cat, tudo); }));
    rodar_versao("C", de_lista(b, [&](const std::vector<int>& l, const std::string& cat) { return rec.versao_c(l, cat, tudo); }));

    VersaoD vd(b, attr, grafo, cfg);
    rodar_versao("D", [&](int, const std::vector<int>& look, const std::vector<int>& cand, std::vector<double>& nota) {
        nota = vd.notas(look, cand);
    });
    // sem pecas_v4.csv (base de exemplo) a D nova e igual a D
    VersaoD vn(b, tem_v4 ? attr_v4 : attr, grafo, cfg);
    if (tem_v4) {
        rodar_versao("D nova", [&](int, const std::vector<int>& look, const std::vector<int>& cand, std::vector<double>& nota) {
            nota = vn.notas(look, cand);
        });
    }
    // F e C + popularidade com os pesos escolhidos na validacao (config_f.csv). desempate pela popularidade
    VersoesF vf;
    std::string erro_f;
    const bool tem_f = vf.montar(b, rec, grafo, pasta_dados, pasta_saida + "/config_f.csv", erro_f);
    int i_c_pop = -1;
    if (tem_f) {
        i_c_pop = (int)res.size();
        rodar_versao("C + popularidade", vf.notas(vf.c_pop), &vf.versao->popularidade());
        rodar_versao("F", vf.notas(vf.f), &vf.versao->popularidade());
    } else {
        std::cerr << "Aviso: " << erro_f << ". F e C + popularidade ficam de fora.\n";
    }
    // configuracoes da peca nova escolhidas pelo ajustar_novas (so com --novas)
    Novas nvn;
    std::vector<ConfigN> cfgs_novas;
    if (novas) {
        std::string erro_n;
        if (!tem_f || !ler_config_novas(pasta_saida + "/config_novas.csv", cfgs_novas) ||
            !nvn.montar(b, rec, grafo, pasta_dados, vf.f, erro_n)) {
            std::cerr << "Sem config_novas.csv ou sem a F (" << erro_n << "). Rode ./recomendador ajustar_novas.\n";
            return false;
        }
        for (const ConfigN& cfg : cfgs_novas) rodar_versao(cfg.nome, nvn.notas(cfg), &nvn.f->popularidade());
    }
    const int principais = (int)res.size();
    // a D antiga so pontuava as vizinhas das arestas. fica so pra comparar com a D na categoria inteira
    rodar_versao("D (só lista curta)", de_lista(b, [&](const std::vector<int>& l, const std::string& cat) { return vd.completar(l, cat, tudo); }));
    if (tem_v4) {
        rodar_versao("D nova (só lista curta)", de_lista(b, [&](const std::vector<int>& l, const std::string& cat) { return vn.completar(l, cat, tudo); }));
    }

    // com a F a tabela vira a final. a corrigida antiga fica como estava
    // as tabelas de antes ficam como estavam: com --novas o arquivo e outro
    const std::string nome_arq = novas  ? (validacao ? "validacao_final_2" : "tabela_final_2")
                                 : tem_f ? (validacao ? "validacao_final" : "tabela_final")
                                         : (validacao ? "validacao_corrigida" : "tabela_corrigida");
    std::ofstream csv(pasta_saida + "/" + nome_arq + ".csv");
    csv << "versao,metrica,media,min,max\n";
    for (int m = 0; m < 4; m++) {
        double e = esperado_aleatorio(b, c, METRICAS[m]);
        csv << "Aleatória (esperado)," << NOMES_METRICAS[m] << ',' << e << ',' << e << ',' << e << '\n';
    }
    for (const Posicoes& p : res) {
        for (int m = 0; m < 4; m++) {
            Resumo r = resumir(p, METRICAS[m]);
            csv << p.nome << ',' << NOMES_METRICAS[m] << ',' << r.media << ',' << r.min << ',' << r.max << '\n';
        }
    }

    // cada versao contra popularidade (indice 1) e contra C (indice 4)
    const int i_pop = 1;
    const int i_c = 4;
    std::vector<std::pair<int, int>> pares;
    for (int v = 0; v < principais; v++) {
        if (v != i_pop) pares.push_back({v, i_pop});
    }
    for (int v = 0; v < principais; v++) {
        if (v != i_c && v != i_pop) pares.push_back({v, i_c});
    }
    if (tem_f) pares.push_back({i_c_pop + 1, i_c_pop});  // F x C + popularidade
    for (int k = 0; k < (int)cfgs_novas.size(); k++) pares.push_back({i_c_pop + 2 + k, i_c_pop + 1});  // nova x F
    std::ofstream dif(pasta_saida + "/" + nome_arq + "_diferencas.csv");
    dif << "x,y,metrica,diferenca,ic95_min,ic95_max,conclusao\n";
    std::vector<std::vector<Intervalo>> ints;
    for (const auto& par : pares) {
        std::vector<Intervalo> linha;
        for (int m = 0; m < 4; m++) {
            Intervalo it = bootstrap(por_consulta(res[par.first], METRICAS[m]), por_consulta(res[par.second], METRICAS[m]));
            linha.push_back(it);
            dif << res[par.first].nome << ',' << res[par.second].nome << ',' << NOMES_METRICAS[m] << ',' << it.dif << ','
                << it.min << ',' << it.max << ',' << conclusao(res[par.first].nome, res[par.second].nome, it) << '\n';
        }
        ints.push_back(linha);
    }

    int com_pergunta = 0;
    double cand_media = 0.0;
    for (int q = 0; q < (int)c.alvo.size(); q++) {
        com_pergunta += !c.erradas[q].empty();
        cand_media += res[0].tam[q];
    }
    if (!c.alvo.empty()) cand_media /= c.alvo.size();

    std::ofstream md(pasta_saida + "/" + nome_arq + ".md");
    const std::string titulo = tem_f ? (validacao ? "Validação com a versão F" : "Tabela final (teste)")
                                     : (validacao ? "Validação com a avaliação corrigida" : "Tabela corrigida (teste)");
    md << "# " << titulo << "\n\n"
       << "Gerado por `./recomendador avaliar_justo" << (validacao ? " --validacao" : "") << "`. "
       << (validacao ? "Looks de validação (semente 7). Eles saem do grafo, do bônus e da popularidade. O teste não é usado.\n\n"
                     : "Looks de teste, uma peça escondida por look com `sortear_escondidas`. Conferência: " + conferencia + ".\n\n")
       << "- Consultas: " << num(c.alvo.size(), 0) << ". Com pergunta de FITB: " << num(com_pergunta, 0)
       << ". Candidatas por consulta: " << num(cand_media, 0) << " em média.\n"
       << "- Cada versão dá nota a todas as peças da categoria da peça escondida. Só as peças do próprio look saem. "
          "Peça sem nota numa versão (B, C e as listas curtas da D) fica com nota 0 e entra no empate.\n"
       << "- Posição da escondida = peças com nota maior + peças com a mesma nota que vêm antes dela na ordem sorteada.\n"
       << "- 10 ordens, sementes 1 a 10. Cada número é a média das 10 ordens, com o menor e o maior valor entre parênteses.\n"
       << "- FITB: a escondida contra 3 erradas da mesma categoria (as mesmas perguntas do `avaliar_fitb`). "
          "Ela acerta se tem a maior nota das 4. No empate vale a mesma ordem sorteada. O chute acerta 25%.\n"
       << "- D: config de `config_d.csv` com `pecas_v2.csv`. D nova: mesma config com `pecas_v4.csv` e só os grupos de `vetor_d.csv`. "
          "As duas agora dão nota a todas as peças da categoria. As linhas \"só lista curta\" mostram a D antiga, que só pontua as vizinhas das arestas.\n"
       << (tem_f ? "- F e C + popularidade: pesos de `config_f.csv`, escolhidos só na validação pelo `ajustar_f` (ver `escolha_final.md`). "
                   "Cada termo vai de 0 a 1 dentro da consulta. As duas desempatam pela popularidade antes da ordem sorteada.\n\n"
                 : "\n")
       << "## Por que sortear a ordem é o mesmo que embaralhar as linhas\n\n"
       << "Todas as versões desempatam pelo índice da peça, que é a linha dela no catálogo. "
          "Embaralhar as linhas troca esses índices por uma ordem sorteada. "
          "A nota de cada peça não muda, só a posição de quem empata. "
          "Então ordenar pela nota e, no empate, pela ordem sorteada dá a mesma lista que embaralhar o arquivo e rodar de novo. "
          "A diferença é que aqui a nota é calculada uma vez por consulta e só o desempate muda. "
          "Isso não vale para a lista curta da D, porque a busca kNN também desempata pela linha. "
          "Por isso a D principal agora dá nota à categoria inteira.\n\n"
       << "## Resultado\n\n"
       << "| Versão | Hit@1 | Hit@5 | Hit@10 | FITB | Empatadas com a escondida |\n| --- | --- | --- | --- | --- | --- |\n";
    md << "| Aleatória (esperado) | " << pct(esperado_aleatorio(b, c, 1), 1) << " | " << pct(esperado_aleatorio(b, c, 5), 5)
       << " | " << pct(esperado_aleatorio(b, c, 10), 10) << " | 25,00% | todas |\n";
    for (const Posicoes& p : res) {
        md << "| " << p.nome;
        for (int m = 0; m < 4; m++) {
            Resumo r = resumir(p, METRICAS[m]);
            if (METRICAS[m] == 0 && com_pergunta == 0) {
                md << " | sem perguntas";
                continue;
            }
            md << " | " << pct(r.media, METRICAS[m]) << " (" << pct(r.min, METRICAS[m]) << " a " << pct(r.max, METRICAS[m]) << ")";
        }
        double emp = 0.0;
        for (int e : p.empatadas) emp += e;
        md << " | " << num(p.empatadas.empty() ? 0.0 : emp / p.empatadas.size(), 0) << " |\n";
    }
    md << "\nA última coluna é a média de outras peças com exatamente a mesma nota da escondida.\n\n"
       << "## Diferenças com intervalo de 95% (bootstrap)\n\n"
       << "Para cada consulta, o acerto é a média das 10 ordens. Os looks são sorteados com reposição 1.000 vezes "
          "(semente " << SEMENTE_BOOTSTRAP << "). O intervalo vai do percentil 2,5 ao 97,5 da diferença. "
          "Se o intervalo não passa pelo 0, a diferença é real. Se passa, pode ser sorte. pp = pontos percentuais.\n\n"
       << "| Comparação | Métrica | Diferença | Intervalo de 95% | Conclusão |\n| --- | --- | --- | --- | --- |\n";
    for (std::size_t i = 0; i < pares.size(); i++) {
        for (int m = 0; m < 4; m++) {
            const Intervalo& it = ints[i][m];
            int casas = METRICAS[m] == 0 ? 2 : 3;
            md << "| " << res[pares[i].first].nome << " x " << res[pares[i].second].nome << " | "
               << (METRICAS[m] == 0 ? std::string("FITB") : "Hit@" + std::to_string(METRICAS[m])) << " | "
               << num(100 * it.dif, casas) << " pp | " << num(100 * it.min, casas) << " a " << num(100 * it.max, casas)
               << " pp | " << conclusao(res[pares[i].first].nome, res[pares[i].second].nome, it) << " |\n";
        }
    }
    if (novas) {
        // peca conhecida = a escondida aparece em algum look de treino de b
        std::vector<bool> conhecida(c.alvo.size());
        for (int q = 0; q < (int)c.alvo.size(); q++) conhecida[q] = grafo.num_looks(c.alvo[q]) > 0;
        const int ks[3] = {1, 10, 0};
        const int i_f = i_c_pop + 1;
        md << "\n## Peça escondida conhecida e peça nova\n\n"
           << "Conhecida: a peça escondida aparece em algum look de treino. Nova: não aparece em nenhum. "
              "\"x aleatório\" é quantas vezes a versão acerta mais que o aleatório esperado no mesmo grupo de consultas "
              "(no FITB, o aleatório acerta 25%). Os intervalos são de bootstrap nas consultas do grupo.\n";
        for (int g = 0; g < 2; g++) {
            std::vector<bool> dentro(c.alvo.size());
            int n = 0;
            for (std::size_t q = 0; q < dentro.size(); q++) {
                dentro[q] = conhecida[q] == (g == 0);
                n += dentro[q];
            }
            std::array<double, 3> esp{};
            for (int j = 0; j < 2; j++) {
                for (std::size_t q = 0; q < dentro.size(); q++) {
                    if (!dentro[q]) continue;
                    double m = (double)candidatas(b, c, (int)q).size();
                    esp[j] += std::min((double)ks[j], m) / m;
                }
                esp[j] = n > 0 ? esp[j] / n : 0.0;
            }
            esp[2] = 0.25;
            md << "\n### " << (g == 0 ? "Peça conhecida" : "Peça nova") << " (" << num(n, 0) << " consultas, "
               << pct((double)n / std::max<std::size_t>(1, c.alvo.size()), 0) << ")\n\n"
               << "| Versão | Hit@1 | x aleatório | Hit@10 | x aleatório | FITB | x aleatório |\n"
               << "| --- | --- | --- | --- | --- | --- | --- |\n"
               << "| Aleatória (esperado) | " << pct(esp[0], 1) << " | 1,0 | " << pct(esp[1], 10) << " | 1,0 | 25,00% | 1,0 |\n";
            for (int v = 0; v < principais; v++) {
                md << "| " << res[v].nome;
                for (int j = 0; j < 3; j++) {
                    double t = 0.0, lo = 2.0, hi = -1.0;
                    for (int s = 0; s < N_ORDENS; s++) {
                        double x = taxa_em(res[v], ks[j], s, dentro);
                        t += x / N_ORDENS;
                        lo = std::min(lo, x);
                        hi = std::max(hi, x);
                    }
                    md << " | " << pct(t, ks[j]) << " (" << pct(lo, ks[j]) << " a " << pct(hi, ks[j]) << ") | "
                       << (esp[j] > 0 ? num(t / esp[j], 1) : "") ;
                }
                md << " |\n";
            }
            md << "\n| Comparação | Métrica | Diferença | Intervalo de 95% | Conclusão |\n| --- | --- | --- | --- | --- |\n";
            std::vector<std::pair<int, int>> pg;
            for (int v = 0; v < principais; v++) {
                if (v != i_pop) pg.push_back({v, i_pop});
            }
            if (tem_f) {
                for (int v = 0; v < principais; v++) {
                    if (v != i_f && v != i_pop) pg.push_back({v, i_f});
                }
            }
            for (const auto& par : pg) {
                for (int j = 0; j < 3; j++) {
                    Intervalo it = bootstrap(filtrar(por_consulta(res[par.first], ks[j]), dentro),
                                             filtrar(por_consulta(res[par.second], ks[j]), dentro));
                    int casas = ks[j] == 0 ? 2 : 3;
                    md << "| " << res[par.first].nome << " x " << res[par.second].nome << " | "
                       << (ks[j] == 0 ? std::string("FITB") : "Hit@" + std::to_string(ks[j])) << " | "
                       << num(100 * it.dif, casas) << " pp | " << num(100 * it.min, casas) << " a " << num(100 * it.max, casas)
                       << " pp | " << conclusao(res[par.first].nome, res[par.second].nome, it) << " |\n";
                }
            }
        }
    }
    std::cout << "Tabela em " << pasta_saida << "/" << nome_arq << ".md" << std::endl;
    return true;
}
