#include "pontos_atributos.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <sstream>

#include "avaliacao_justa.hpp"

const std::array<std::string, N_ATRIB> NOME_ATRIB = {"cor", "estampa", "formalidade", "estilo", "ocasiao",
                                                     "comprimento"};

namespace {

const std::string CABECALHO_V5 =
    "id,categoria,estilo_casual,estilo_classico,estilo_esportivo,estilo_romantico,estilo_moderno,"
    "ocasiao_dia,ocasiao_trabalho,ocasiao_festa,ocasiao_esporte,ocasiao_praia,formalidade,"
    "L,a,b,cor_neutra,estampa,comprimento,modelagem,grupo_cor";
constexpr int N_COLUNAS_V5 = 21;
const std::string DESC = "desconhecido";

const std::vector<std::string> ESTILOS = {"casual", "classico", "esportivo", "romantico", "moderno"};
const std::vector<std::string> OCASIOES = {"dia", "trabalho", "festa", "esporte", "praia"};

constexpr unsigned SEMENTE_DIAGNOSTICO = 17;
constexpr int N_SORTEADAS = 10;  // pecas sorteadas por consulta no diagnostico, pra tirar ruido
constexpr int N_RESUMO = 10;

// nome no texto: com acento e por extenso
std::string mostrar(const std::string& v) {
    static const std::map<std::string, std::string> m = {
        {"poa", "poá"},         {"animal", "animal print"},  {"outra", "outra estampa"},
        {"medio", "médio"},     {"media", "média"},          {"classico", "clássico"},
        {"romantico", "romântico"}, {"dia", "dia a dia"},    {"ocasiao", "ocasião"},
        {"peca unica", "peça única"}, {"calcado", "calçado"}, {"acessorio", "acessório"}};
    auto it = m.find(v);
    return it == m.end() ? v : it->second;
}

// virgula decimal e sinal sempre
std::string com_sinal(double x) {
    std::ostringstream o;
    o << std::fixed << std::setprecision(2) << std::fabs(x);
    std::string s = o.str();
    std::replace(s.begin(), s.end(), '.', ',');
    return (x < 0 ? "-" : "+") + s;
}

std::string num(double x, int casas) {
    std::ostringstream o;
    o << std::fixed << std::setprecision(casas) << x;
    std::string s = o.str();
    std::replace(s.begin(), s.end(), '.', ',');
    // -0,0000 vira 0,0000
    if (!s.empty() && s[0] == '-') s = s.find_first_not_of("-0,") == std::string::npos ? s.substr(1) : "−" + s.substr(1);
    return s;
}

std::string lista_valores(const ValoresAtrib& val, int a, int i) {
    std::string s;
    for (int x : val.val[a][i]) s += (s.empty() ? "" : "/") + mostrar(val.nomes[a][x]);
    return s;
}

bool roupa(const std::string& categoria) {
    std::string g = grupo_categoria(categoria);
    return g == "parte de cima" || g == "parte de baixo" || g == "peca unica";
}

// soma o par (p de A, q de B) na tabela. cada peca reparte peso w entre os valores marcados
void somar(Tabela& t, const std::vector<int>& vp, const std::vector<int>& vq, int nv, double w) {
    if (vp.empty() || vq.empty()) return;
    t.total += w;
    double u = w / (vp.size() * vq.size());
    for (int x : vp) {
        for (int y : vq) {
            t.n[x * nv + y] += u;
            t.soma_a[x] += u;
            t.soma_b[y] += u;
        }
    }
}

int indice(const std::vector<std::string>& v, const std::string& x) {
    auto it = std::lower_bound(v.begin(), v.end(), x);
    return it != v.end() && *it == x ? (int)(it - v.begin()) : -1;
}

}  // namespace

int ValoresAtrib::valor(int a, const std::string& nome) {
    std::vector<std::string>& v = nomes[a];
    auto it = std::find(v.begin(), v.end(), nome);
    if (it != v.end()) return (int)(it - v.begin());
    v.push_back(nome);
    return (int)v.size() - 1;
}

std::string grupo_categoria(const std::string& c) {
    if (c == "top" || c == "outerwear") return "parte de cima";
    // meia-calca vai por baixo da saia ou do vestido, entao fica com a parte de baixo
    if (c == "pants" || c == "skirt" || c == "legwear") return "parte de baixo";
    if (c == "dress" || c == "jumpsuit") return "peca unica";
    if (c == "shoes") return "calcado";
    if (c == "bag") return "bolsa";
    return "acessorio";
}

std::string faixa_formalidade(double f) {
    // sem o texto nao da pra separar o 0,5 de empate do 0,5 sem palavra. quase todo 0,5 e sem palavra
    if (std::fabs(f - 0.5) < 1e-9) return DESC;
    if (f < 1.0 / 3.0 - 1e-6) return "baixa";
    if (f > 2.0 / 3.0 + 1e-6) return "alta";
    return "media";
}

bool ler_valores(const std::string& caminho, const Base& base, ValoresAtrib& val, std::string& erro) {
    std::ifstream arquivo(caminho);
    if (!arquivo) {
        erro = "Não foi possível abrir " + caminho;
        return false;
    }
    std::string linha;
    std::getline(arquivo, linha);
    if (!linha.empty() && linha.back() == '\r') linha.pop_back();
    if (linha != CABECALHO_V5) {
        erro = "Cabeçalho inesperado em " + caminho;
        return false;
    }

    // ordem fixa dos valores, pra tabela sair igual em toda execucao
    val = ValoresAtrib{};
    for (const char* v : {"lisa", "listrada", "xadrez", "floral", "poa", "animal", "outra"}) val.valor(ESTAMPA, v);
    for (const char* v : {"baixa", "media", "alta"}) val.valor(FORMALIDADE, v);
    for (const std::string& v : ESTILOS) val.valor(ESTILO, v);
    for (const std::string& v : OCASIOES) val.valor(OCASIAO, v);
    for (const char* v : {"curto", "medio", "longo"}) val.valor(COMPRIMENTO, v);
    for (int a = 0; a < N_ATRIB; a++) val.val[a].assign(base.pecas.size(), {});

    std::vector<char> lida(base.pecas.size(), 0);
    int numero = 1;
    while (std::getline(arquivo, linha)) {
        numero++;
        if (linha.empty() || linha == "\r") continue;
        std::vector<std::string> col = separar_colunas(linha);
        if ((int)col.size() != N_COLUNAS_V5) {
            erro = "Linha " + std::to_string(numero) + " com formato errado em " + caminho;
            return false;
        }
        int i = -1;
        double f = 0.0;
        try {
            auto it = base.indice_da_peca.find(std::stoi(col[0]));
            if (it == base.indice_da_peca.end()) continue;
            i = it->second;
            f = std::stod(col[12]);
        } catch (const std::exception&) {
            erro = "Linha " + std::to_string(numero) + " com formato errado em " + caminho;
            return false;
        }
        for (int k = 0; k < 5; k++) {
            if (col[2 + k] == "1") val.val[ESTILO][i].push_back(val.valor(ESTILO, ESTILOS[k]));
            if (col[7 + k] == "1") val.val[OCASIAO][i].push_back(val.valor(OCASIAO, OCASIOES[k]));
        }
        std::string faixa = faixa_formalidade(f);
        if (faixa != DESC) val.val[FORMALIDADE][i].push_back(val.valor(FORMALIDADE, faixa));
        if (col[17] != DESC && !col[17].empty()) val.val[ESTAMPA][i].push_back(val.valor(ESTAMPA, col[17]));
        // comprimento de bolsa ou colar nao e o mesmo atributo
        if (col[18] != DESC && !col[18].empty() && roupa(base.pecas[i].categoria)) {
            val.val[COMPRIMENTO][i].push_back(val.valor(COMPRIMENTO, col[18]));
        }
        if (col[20] != DESC && !col[20].empty()) val.val[COR][i].push_back(val.valor(COR, col[20]));
        lida[i] = 1;
    }
    for (int i = 0; i < (int)lida.size(); i++) {
        if (!lida[i]) {
            erro = "Peça " + std::to_string(base.pecas[i].id) + " sem atributos em " + caminho;
            return false;
        }
    }
    return true;
}

double esperado(const Tabela& t, int, int x, int y) {
    if (t.total <= 0) return 0.0;
    // total x (soma_a[x] / total) x (soma_b[y] / total)
    return t.soma_a[x] * t.soma_b[y] / t.total;
}

double pontos_tabela(const Tabela& t, int nv, int x, int y) {
    // o +1 nos dois lados evita log de 0 e puxa pra 0 os pares com pouca conta
    return std::log((t.n[x * nv + y] + 1.0) / (esperado(t, nv, x, y) + 1.0));
}

PontosAtributos::PontosAtributos(const Base& base, const ValoresAtrib& val) : val_(val) {
    peso.fill(1.0);
    for (const Peca& p : base.pecas) cats_.push_back(p.categoria);
    std::sort(cats_.begin(), cats_.end());
    cats_.erase(std::unique(cats_.begin(), cats_.end()), cats_.end());
    for (const std::string& c : cats_) grupos_.push_back(grupo_categoria(c));
    std::sort(grupos_.begin(), grupos_.end());
    grupos_.erase(std::unique(grupos_.begin(), grupos_.end()), grupos_.end());

    const int nc = (int)cats_.size();
    const int ng = (int)grupos_.size();
    for (const std::string& c : cats_) grupo_da_cat_.push_back(indice(grupos_, grupo_categoria(c)));
    cat_.resize(base.pecas.size());
    for (int i = 0; i < (int)base.pecas.size(); i++) cat_[i] = indice(cats_, base.pecas[i].categoria);
    pares_.assign((std::size_t)nc * nc, 0.0);
    for (int a = 0; a < N_ATRIB; a++) {
        int nv = (int)val.nomes[a].size();
        Tabela vazia;
        vazia.n.assign((std::size_t)nv * nv, 0.0);
        vazia.soma_a.assign(nv, 0.0);
        vazia.soma_b.assign(nv, 0.0);
        por_cat_[a].assign((std::size_t)nc * nc, vazia);
        por_grupo_[a].assign((std::size_t)ng * ng, vazia);
    }

    for (const Look& look : base.looks) {
        if (!look.treino) continue;
        for (std::size_t i = 0; i < look.pecas.size(); i++) {
            for (std::size_t j = i + 1; j < look.pecas.size(); j++) {
                int p = look.pecas[i];
                int q = look.pecas[j];
                int A = cat_[p];
                int B = cat_[q];
                // dois brincos no mesmo look nao dizem nada sobre combinar categorias
                if (A == B) continue;
                pares_[(std::size_t)A * nc + B]++;
                pares_[(std::size_t)B * nc + A]++;
                int G = grupo_da_cat_[A];
                int H = grupo_da_cat_[B];
                // blusa com casaco cai na mesma tabela de grupo nos dois sentidos: meio par em cada
                double w = G == H ? 0.5 : 1.0;
                for (int a = 0; a < N_ATRIB; a++) {
                    int nv = (int)val.nomes[a].size();
                    const std::vector<int>& vp = val.val[a][p];
                    const std::vector<int>& vq = val.val[a][q];
                    somar(por_cat_[a][(std::size_t)A * nc + B], vp, vq, nv, 1.0);
                    somar(por_cat_[a][(std::size_t)B * nc + A], vq, vp, nv, 1.0);
                    somar(por_grupo_[a][(std::size_t)G * ng + H], vp, vq, nv, w);
                    somar(por_grupo_[a][(std::size_t)H * ng + G], vq, vp, nv, w);
                }
            }
        }
    }
}

double PontosAtributos::pares(const std::string& cat_a, const std::string& cat_b) const {
    int A = indice(cats_, cat_a);
    int B = indice(cats_, cat_b);
    if (A < 0 || B < 0) return 0.0;
    return pares_[(std::size_t)A * cats_.size() + B];
}

bool PontosAtributos::usa_grupo(const std::string& cat_a, const std::string& cat_b) const {
    return pares(cat_a, cat_b) < MIN_PARES_CATEGORIA;
}

const Tabela* PontosAtributos::tabela_de(int a, bool grupo, const std::string& x, const std::string& y) const {
    const std::vector<std::string>& nomes = grupo ? grupos_ : cats_;
    int X = indice(nomes, x);
    int Y = indice(nomes, y);
    if (X < 0 || Y < 0) return nullptr;
    return &(grupo ? por_grupo_ : por_cat_)[a][(std::size_t)X * nomes.size() + Y];
}

const Tabela* PontosAtributos::tabela(int a, int p, int c) const {
    int A = cat_[p];
    int B = cat_[c];
    if (A == B) return nullptr;
    const std::size_t nc = cats_.size();
    if (pares_[A * nc + B] >= MIN_PARES_CATEGORIA) return &por_cat_[a][A * nc + B];
    return &por_grupo_[a][(std::size_t)grupo_da_cat_[A] * grupos_.size() + grupo_da_cat_[B]];
}

double PontosAtributos::pontos_de(int a, int p, int c) const {
    const Tabela* t = tabela(a, p, c);
    const std::vector<int>& vp = val_.val[a][p];
    const std::vector<int>& vc = val_.val[a][c];
    // desconhecido vale 0
    if (t == nullptr || vp.empty() || vc.empty()) return 0.0;
    int nv = (int)val_.nomes[a].size();
    // varios valores marcados: media dos pares de valores, igual ao peso usado na contagem
    double soma = 0.0;
    for (int x : vp) {
        for (int y : vc) soma += pontos_tabela(*t, nv, x, y);
    }
    return soma / (vp.size() * vc.size());
}

double PontosAtributos::pontos(int p, int c) const {
    double soma = 0.0;
    for (int a = 0; a < N_ATRIB; a++) {
        if (peso[a] != 0.0) soma += peso[a] * pontos_de(a, p, c);
    }
    return soma;
}

std::string PontosAtributos::explicar(int p, int c) const {
    std::vector<std::pair<double, std::string>> partes;
    for (int a = 0; a < N_ATRIB; a++) {
        if (peso[a] == 0.0) continue;
        double v = peso[a] * pontos_de(a, p, c);
        if (std::fabs(v) < 0.005) continue;  // some no arredondamento
        partes.push_back({v, mostrar(NOME_ATRIB[a]) + " " + lista_valores(val_, a, p) + " com " +
                                 lista_valores(val_, a, c) + ": " + com_sinal(v)});
    }
    if (partes.empty()) return "nenhum atributo pesa neste par";
    std::stable_sort(partes.begin(), partes.end(),
                     [](const auto& x, const auto& y) { return std::fabs(x.first) > std::fabs(y.first); });
    std::string s;
    for (const auto& x : partes) s += (s.empty() ? "" : "; ") + x.second;
    return s;
}

namespace {

struct Celula {
    double pontos = 0, n = 0, esp = 0;
    std::string a, b, x, y;
    bool grupo = false;
};

// celulas das tabelas em uso: categorias com pares suficientes (A < B) e grupos (G <= H)
std::vector<Celula> celulas(const PontosAtributos& pa, int a) {
    const ValoresAtrib& val = pa.valores();
    int nv = (int)val.nomes[a].size();
    std::vector<Celula> res;
    auto juntar = [&](bool grupo, const std::string& A, const std::string& B) {
        const Tabela* t = pa.tabela_de(a, grupo, A, B);
        if (t == nullptr || t->total <= 0) return;
        for (int x = 0; x < nv; x++) {
            for (int y = 0; y < nv; y++) {
                if (A == B && y < x) continue;  // mesma tabela nos dois sentidos
                if (t->soma_a[x] <= 0 || t->soma_b[y] <= 0) continue;  // valor que nao aparece nao tem par
                res.push_back({pontos_tabela(*t, nv, x, y), t->n[x * nv + y], esperado(*t, nv, x, y), A, B,
                               val.nomes[a][x], val.nomes[a][y], grupo});
            }
        }
    };
    const std::vector<std::string>& cats = pa.categorias();
    for (std::size_t i = 0; i < cats.size(); i++) {
        for (std::size_t j = i + 1; j < cats.size(); j++) {
            if (!pa.usa_grupo(cats[i], cats[j])) juntar(false, cats[i], cats[j]);
        }
    }
    // tabela de grupo so entra se algum par de categorias usa ela
    std::set<std::pair<std::string, std::string>> usados;
    for (const std::string& x : cats) {
        for (const std::string& y : cats) {
            if (x == y || !pa.usa_grupo(x, y)) continue;
            std::string g = grupo_categoria(x), h = grupo_categoria(y);
            usados.insert({std::min(g, h), std::max(g, h)});
        }
    }
    for (const auto& u : usados) juntar(true, u.first, u.second);
    return res;
}

void gravar_csv(const PontosAtributos& pa, int a, const std::string& caminho) {
    std::ofstream s(caminho);
    s << "tabela,lado_a,lado_b,pares,valor_a,valor_b,n,esperado,pontos\n";
    for (const Celula& c : celulas(pa, a)) {
        const Tabela* t = pa.tabela_de(a, c.grupo, c.a, c.b);
        s << (c.grupo ? "grupo" : "categoria") << ',' << c.a << ',' << c.b << ',' << t->total << ',' << c.x << ','
          << c.y << ',' << c.n << ',' << c.esp << ',' << c.pontos << '\n';
    }
}

std::string linha_celula(const Celula& c) {
    std::string lado = c.grupo ? " (grupo)" : "";
    return "| " + mostrar(c.a) + lado + " | " + mostrar(c.x) + " | " + mostrar(c.b) + lado + " | " + mostrar(c.y) +
           " | " + num(c.n, 1) + " | " + num(c.esp, 1) + " | " + num(c.pontos, 2) + " |\n";
}

struct Diagnostico {
    std::array<double, N_ATRIB + 1> certo{}, sorteado{}, dif{}, dif2{};
    std::array<int, N_ATRIB + 1> ganha{}, perde{};
    std::array<double, N_ATRIB> conhecido{};
    long long pares = 0, pares_grupo = 0;
    int consultas = 0;
};

// pontos do par certo (escondida com as pecas do look) contra pecas sorteadas da mesma categoria.
// por_aparicao = false sorteia no catalogo, como a lista do recomendador. true sorteia entre as aparicoes
// nos looks de treino: peca popular sai mais, como a escondida. ai sobra so o efeito do par.
// tabelas sem os looks de validacao. so diagnostico: nao escolhe nada
Diagnostico diagnosticar(const Base& sem_val, const PontosAtributos& pa, const Consultas& c,
                         const ValoresAtrib& val, bool por_aparicao) {
    const Base& base = sem_val;
    std::unordered_map<std::string, std::vector<int>> aparicoes;
    for (const Look& l : sem_val.looks) {
        if (!l.treino) continue;
        for (int p : l.pecas) aparicoes[sem_val.pecas[p].categoria].push_back(p);
    }
    std::mt19937 gerador(SEMENTE_DIAGNOSTICO);
    Diagnostico d;
    for (int q = 0; q < (int)c.alvo.size(); q++) {
        const int alvo = c.alvo[q];
        auto it = c.por_cat.find(base.pecas[alvo].categoria);
        if (it == c.por_cat.end() || it->second.size() < 2) continue;
        const std::vector<int>* m = &it->second;
        auto ap = aparicoes.find(base.pecas[alvo].categoria);
        // so a escondida aparece: o sorteio nunca sairia dela
        if (por_aparicao && ap != aparicoes.end() &&
            std::any_of(ap->second.begin(), ap->second.end(), [alvo](int x) { return x != alvo; })) {
            m = &ap->second;
        }
        std::array<double, N_ATRIB + 1> sc{}, ss{};
        int np = 0;
        for (int p : c.look[q]) {
            if (base.pecas[p].categoria == base.pecas[alvo].categoria) continue;
            np++;
            if (pa.usa_grupo(base.pecas[p].categoria, base.pecas[alvo].categoria)) d.pares_grupo++;
            for (int a = 0; a < N_ATRIB; a++) {
                sc[a] += pa.pontos_de(a, p, alvo);
                if (!val.val[a][p].empty() && !val.val[a][alvo].empty()) d.conhecido[a]++;
            }
        }
        if (np == 0) continue;
        for (int s = 0; s < N_SORTEADAS; s++) {
            int r = alvo;
            while (r == alvo) r = (*m)[std::uniform_int_distribution<int>(0, (int)m->size() - 1)(gerador)];
            for (int p : c.look[q]) {
                if (base.pecas[p].categoria == base.pecas[alvo].categoria) continue;
                for (int a = 0; a < N_ATRIB; a++) ss[a] += pa.pontos_de(a, p, r) / N_SORTEADAS;
            }
        }
        for (int a = 0; a < N_ATRIB; a++) {
            sc[N_ATRIB] += sc[a];
            ss[N_ATRIB] += ss[a];
        }
        // media por par dentro da consulta. cada consulta e um look, entao as consultas sao independentes
        for (int a = 0; a <= N_ATRIB; a++) {
            double x = sc[a] / np;
            double y = ss[a] / np;
            d.certo[a] += x;
            d.sorteado[a] += y;
            d.dif[a] += x - y;
            d.dif2[a] += (x - y) * (x - y);
            if (sc[a] > ss[a] + 1e-12) d.ganha[a]++;
            if (sc[a] < ss[a] - 1e-12) d.perde[a]++;
        }
        d.pares += np;
        d.consultas++;
    }
    return d;
}

}  // namespace

bool rodar_pontos(const Base& base, const std::string& pasta_dados, const std::string& pasta_saida) {
    namespace fs = std::filesystem;
    ValoresAtrib val;
    std::string erro;
    if (!ler_valores(pasta_dados + "/pecas_v5.csv", base, val, erro)) {
        std::cerr << erro << "\nRode antes: python3 preprocess/grupos_cor.py\n";
        return false;
    }
    PontosAtributos pa(base, val);
    const std::string pasta = pasta_saida + "/tabelas_pontos";
    fs::create_directories(pasta);
    for (int a = 0; a < N_ATRIB; a++) gravar_csv(pa, a, pasta + "/" + NOME_ATRIB[a] + ".csv");
    std::cout << "Tabelas gravadas em " << pasta << std::endl;

    std::ofstream md(pasta_saida + "/tabelas_pontos.md");
    md << "# Tabelas de pontos por atributo\n\n"
          "Gerado por `./recomendador pontos`. As tabelas usam todos os looks de treino (os de validação entram). "
          "Os CSVs estão em `tabelas_pontos/`, um por atributo.\n\n"
          "## Conta\n\n"
          "Para cada atributo e cada par de categorias A e B, com A diferente de B:\n\n"
          "- `n(x, y)`: pares de peças no mesmo look de treino, a de A com valor x e a de B com valor y.\n"
          "- `esperado(x, y) = total × fração de x no lado A × fração de y no lado B`.\n"
          "- `pontos(x, y) = log((n + 1) / (esperado + 1))`.\n\n"
          "O total é o número de pares A–B em que as duas peças têm valor conhecido. A fração de x no lado A é "
          "a parte desses pares em que a peça de A tem valor x. Ela conta aparições em pares, e não peças do "
          "catálogo, para que o esperado some o mesmo que o n. Valor desconhecido fica fora das contas e vale 0. "
          "Duas peças da mesma categoria no mesmo look não entram, e o par vale 0.\n\n"
          "Estilo e ocasião podem ter vários valores marcados. Cada peça reparte o peso 1 entre os seus valores "
          "(dois marcados, metade para cada). Nenhum marcado conta como desconhecido. Os pontos do par de peças "
          "são a média dos pontos dos pares de valores marcados, que é a mesma divisão usada na contagem.\n\n"
          "Formalidade: o número de 0 a 1 vira baixa (abaixo de 1/3), média (de 1/3 a 2/3) e alta (acima de 2/3). "
          "O valor 0,5 conta como desconhecido. Ele quer dizer, quase sempre, que o texto não tem palavra formal "
          "nem casual. Sem o texto original não dá para separar o 0,5 de empate, que é raro "
          "(mesma regra de `comparacao_fontes.md`). Comprimento só vale para roupas (parte de cima, parte de baixo "
          "e peça única). Nas outras categorias fica desconhecido. A cor é o `grupo_cor` de `dados/pecas_v5.csv` "
          "(ver `grupos_cor.md`).\n\n"
          "`pontos_atributos(p, c)` é a soma, nos 6 atributos, de `peso × pontos`. Os pesos começam em 1.\n\n";

    md << "## Categorias e grupos de categoria\n\n"
          "Se um par de categorias tem menos de "
       << (int)MIN_PARES_CATEGORIA
       << " pares de peças nos looks de treino, o par usa a tabela dos grupos de categoria. Na tabela de grupo, "
          "um par dentro do mesmo grupo (blusa com casaco) conta meio par em cada sentido.\n\n"
          "| categoria | peças | grupo |\n| --- | --- | --- |\n";
    std::map<std::string, int> n_cat;
    for (const Peca& p : base.pecas) n_cat[p.categoria]++;
    for (const std::string& c : pa.categorias()) {
        md << "| " << c << " | " << n_cat[c] << " | " << mostrar(grupo_categoria(c)) << " |\n";
    }
    const std::vector<std::string>& cats = pa.categorias();
    int com_pares = 0, fallback = 0;
    std::string lista_fallback;
    for (std::size_t i = 0; i < cats.size(); i++) {
        for (std::size_t j = i + 1; j < cats.size(); j++) {
            double np = pa.pares(cats[i], cats[j]);
            if (np >= MIN_PARES_CATEGORIA) {
                com_pares++;
            } else {
                fallback++;
                if (np > 0) lista_fallback += (lista_fallback.empty() ? "" : ", ") + cats[i] + "–" + cats[j] + " (" +
                                              std::to_string((int)np) + ")";
            }
        }
    }
    md << "\nDos " << cats.size() * (cats.size() - 1) / 2 << " pares de categorias, " << com_pares
       << " têm tabela própria e " << fallback << " usam o grupo. Os que usam o grupo e aparecem pelo menos uma vez: "
       << (lista_fallback.empty() ? "nenhum" : lista_fallback) << ".\n\n";

    md << "## Pares mais positivos e mais negativos\n\n"
          "Entram as tabelas em uso: as de categoria com pares suficientes e as de grupo. Ao lado vêm o n e o "
          "esperado. Com n e esperado pequenos, o número se apoia em poucas contagens. O +1 da conta já puxa esses "
          "casos para perto de 0.\n\n";
    for (int a = 0; a < N_ATRIB; a++) {
        std::vector<Celula> cel = celulas(pa, a);
        std::sort(cel.begin(), cel.end(), [](const Celula& x, const Celula& y) { return x.pontos > y.pontos; });
        md << "### " << mostrar(NOME_ATRIB[a]) << "\n\n";
        const std::string cab = "| lado A | valor A | lado B | valor B | n | esperado | pontos |\n"
                                "| --- | --- | --- | --- | --- | --- | --- |\n";
        md << "Mais positivos:\n\n" << cab;
        for (int i = 0; i < N_RESUMO && i < (int)cel.size(); i++) md << linha_celula(cel[i]);
        md << "\nMais negativos:\n\n" << cab;
        for (int i = 0; i < N_RESUMO && i < (int)cel.size(); i++) md << linha_celula(cel[cel.size() - 1 - i]);
        md << "\n";
    }

    std::cout << "Diagnóstico na validação..." << std::endl;
    const Base sem_val = base_sem_validacao(base);
    const PontosAtributos pv(sem_val, val);
    const Consultas c = consultas_validacao(base);
    md << "## Diagnóstico na validação\n\n"
          "Só diagnóstico, não escolhe nada. As tabelas são refeitas sem os looks de validação. Em cada consulta "
          "de validação (mesmas escondidas da avaliação justa), comparo os pontos da escondida com as peças do "
          "look contra os pontos de "
       << N_SORTEADAS
       << " peças sorteadas da mesma categoria (média das sorteadas). Pesos todos 1. O valor é a média por par "
          "de peças. O erro padrão sai das consultas, que são looks diferentes. Diferença maior que uns 3 erros "
          "padrão não parece sorte.\n\n"
          "Há dois sorteios. No catálogo, toda peça da categoria tem a mesma chance, como na lista que o "
          "recomendador ordena. Nas aparições, a peça sai na proporção dos looks de treino em que aparece, como a "
          "escondida. O segundo tira o efeito de a cor ou o valor ser mais comum nos looks do que no catálogo e "
          "deixa só o efeito do par.\n\n";
    for (int modo = 0; modo < 2; modo++) {
        Diagnostico d = diagnosticar(sem_val, pv, c, val, modo == 1);
        md << "### Sorteio " << (modo == 0 ? "no catálogo" : "nas aparições") << "\n\n"
           << d.consultas << " consultas, " << d.pares << " pares, "
           << num(d.pares ? 100.0 * d.pares_grupo / d.pares : 0.0, 1) << "% deles com a tabela de grupo.\n\n"
           << "| atributo | par conhecido (%) | certo | sorteado | diferença | erro padrão | certo ganha (%) | "
              "certo perde (%) |\n| --- | --- | --- | --- | --- | --- | --- | --- |\n";
        std::cout << (modo == 0 ? "Sorteio no catálogo\n" : "Sorteio nas aparições\n");
        for (int a = 0; a <= N_ATRIB; a++) {
            double q = std::max(d.consultas, 1);
            double media = d.dif[a] / q;
            double var = d.consultas > 1 ? (d.dif2[a] - q * media * media) / (q - 1) : 0.0;
            double ep = std::sqrt(std::max(var, 0.0) / q);
            std::string nome = a < N_ATRIB ? mostrar(NOME_ATRIB[a]) : "soma";
            std::string conhecido = a < N_ATRIB ? num(d.pares ? 100.0 * d.conhecido[a] / d.pares : 0.0, 1) : "";
            md << "| " << nome << " | " << conhecido << " | " << num(d.certo[a] / q, 4) << " | "
               << num(d.sorteado[a] / q, 4) << " | " << num(media, 4) << " | " << num(ep, 4) << " | "
               << num(100.0 * d.ganha[a] / q, 1) << " | " << num(100.0 * d.perde[a] / q, 1) << " |\n";
            std::cout << "  " << nome << ": certo " << d.certo[a] / q << ", sorteado " << d.sorteado[a] / q
                      << ", diferença " << media << " (erro padrão " << ep << ")\n";
        }
        md << "\n";
    }
    md << "Certo ganha e certo perde contam as consultas em que a soma da escondida fica acima ou abaixo da média "
          "das sorteadas. O resto empata, quase sempre porque o valor é desconhecido.\n";
    std::cout << "Resumo em " << pasta_saida << "/tabelas_pontos.md" << std::endl;
    return true;
}
