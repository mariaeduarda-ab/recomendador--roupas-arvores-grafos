#pragma once

#include <string>

#include "leitura.hpp"

// escolhe a config na validacao, roda o teste uma vez e grava os csv em pasta_saida
bool avaliar_d(const Base& base, const std::string& pasta_dados, const std::string& pasta_saida);
