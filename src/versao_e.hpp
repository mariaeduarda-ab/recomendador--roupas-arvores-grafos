#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

#include "atributos.hpp"
#include "grafo_looks.hpp"
#include "kdtree.hpp"
#include "recomendador.hpp"
#include "versao_d.hpp"

struct AtribE {
    int cor = 0;
    double atencao = 0.5;
};

// cores cromaticas da tabela. o que nao esta nela (preto, branco, prata...) combina com tudo
class TabelaCores {
public:
    bool ler(const std::string& caminho, std::string& erro);
    void adicionar(const std::string& cor, const std::vector<std::string>& combina);

    int id(const std::string& cor);
    bool cromatica(int cor) const;
    bool lista_combina(int a, int b) const;
    int total() const { return (int)nomes_.size(); }

private:
    std::unordered_map<std::string, int> nomes_;
    std::vector<char> cromatica_;
    std::vector<std::vector<char>> combina_;
    void crescer();
};

bool ler_atributos_e(const std::string& caminho, const Base& base, TabelaCores& cores, std::vector<AtribE>& attr,
                     std::string& erro);

enum class Tipo { CIMA, BAIXO, INTEIRA, COMPLEMENTO };
Tipo tipo_da_categoria(const std::string& categoria);

// a mesma cor em cima e embaixo pesa. a mesma cor no sapato ou no acessorio repete o detalhe
double regra_cor_e(const TabelaCores& cores, int cor_p, Tipo tipo_p, int cor_q, Tipo tipo_q);
// 1 com 0 da 1 (uma aparece, a outra acalma). 1 com 1 da -1 (as duas brigam). 0 com 0 da 0
double regra_atencao(double p, double q);

struct ConfigE {
    double a = 1.0;
    double b = 1.0;
    double c = 1.0;
    double d = 1.0;
    int k = 20;
    bool filtrar_cor = true;
    bool categoria_inteira = false;
};

struct Partes {
    int indice = -1;
    std::array<double, 4> soma{};  // proximidade, cor, atencao, bonus

    double nota(double a, double b, double c, double d) const {
        return a * soma[0] + b * soma[1] + c * soma[2] + d * soma[3];
    }
};

// uma KD-tree por categoria e cor. a tabela escolhe em quais arvores a busca entra
class VersaoE {
public:
    VersaoE(const Base& base, const std::vector<Atrib>& attr, const std::vector<AtribE>& attr_e,
            const TabelaCores& cores, const GrafoLooks& grafo);

    // a nota de cada parte fica separada pra testar varios pesos sem refazer a busca
    std::vector<Partes> partes(const std::vector<int>& look, const std::string& categoria, const ConfigE& cfg) const;
    std::vector<Candidata> completar(const std::vector<int>& look, const std::string& categoria, const ConfigE& cfg,
                                     int n) const;

    double ms_arvores() const { return ms_arvores_; }

private:
    std::vector<int> candidatas(const std::vector<int>& look, const std::string& categoria, const ConfigE& cfg) const;

    const Base& base_;
    const std::vector<Atrib>& attr_;
    const std::vector<AtribE>& attr_e_;
    const TabelaCores& cores_;
    const GrafoLooks& grafo_;
    TabelaGrupos grupos_;

    std::unordered_map<std::string, std::vector<std::vector<Peca>>> pecas_;  // categoria -> cor -> pecas
    std::unordered_map<std::string, std::vector<KDTree>> arvores_;
    std::unordered_map<std::string, std::vector<int>> da_categoria_;
    double ms_arvores_ = 0.0;
};
