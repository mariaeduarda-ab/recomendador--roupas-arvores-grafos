#pragma once

#include <string>

#include "leitura.hpp"
#include "versao_e.hpp"

// escolhe a config na validacao, roda o teste e grava os csv em pasta_saida
bool avaliar_e(const Base& base, const std::string& pasta_dados, const std::string& tabela_cores,
               const std::string& pasta_saida);

bool ler_config_e(const std::string& caminho, ConfigE& cfg);
