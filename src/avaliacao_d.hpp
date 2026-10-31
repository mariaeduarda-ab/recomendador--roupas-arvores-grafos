#pragma once

#include <string>

#include "leitura.hpp"
#include "versao_d.hpp"

// escolhe a config na validacao, roda o teste uma vez e grava os csv em pasta_saida
bool avaliar_d(const Base& base, const std::string& pasta_dados, const std::string& pasta_saida);

// false se o arquivo nao existe ou esta com formato errado. nesse caso cfg nao muda
bool ler_config_d(const std::string& caminho, ConfigD& cfg);

// roda popularidade, B, C e D com uma config pronta. nao escolhe nada.
// serve pra ver quanto o resultado muda com a ordem das linhas do catalogo
bool reavaliar(const Base& base, const std::string& pasta_dados, const ConfigD& cfg, const std::string& pasta_saida);
