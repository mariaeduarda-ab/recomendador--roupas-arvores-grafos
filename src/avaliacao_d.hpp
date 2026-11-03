#pragma once

#include <fstream>
#include <functional>
#include <string>
#include <vector>

#include "avaliacao.hpp"
#include "leitura.hpp"
#include "recomendador.hpp"
#include "versao_d.hpp"

struct Linha {
    std::string nome;
    std::vector<int> pos;  // posicao da escondida na lista completa, -1 se nao apareceu
    double candidatas = 0.0;
};

using Lista = std::function<std::vector<Candidata>(const std::vector<int>&, const std::string&)>;

int conta_hit(const std::vector<int>& pos, int k);
int conta_cobertura(const std::vector<int>& pos);
std::vector<int> sem_escondida(const Escondida& e, const Base& base);
Linha rodar(const std::string& nome, const Base& base, const std::vector<Escondida>& testes, const Lista& gerar);
void gravar_linha(std::ofstream& csv, const Linha& l);
void gravar_diferenca(std::ofstream& csv, const Linha& x, const Linha& y);
std::vector<int> sortear_validacao(const Base& base);

// escolhe a config na validacao, roda o teste uma vez e grava os csv em pasta_saida
bool avaliar_d(const Base& base, const std::string& pasta_dados, const std::string& pasta_saida);

// false se o arquivo nao existe ou esta com formato errado. nesse caso cfg nao muda
bool ler_config_d(const std::string& caminho, ConfigD& cfg);

// roda popularidade, B, C e D com uma config pronta. nao escolhe nada.
// serve pra ver quanto o resultado muda com a ordem das linhas do catalogo
bool reavaliar(const Base& base, const std::string& pasta_dados, const ConfigD& cfg, const std::string& pasta_saida);
