#pragma once

#include <string>
#include <vector>

#include "grafo_similaridade.hpp"
#include "leitura.hpp"

// exporta so uma amostra, com a base inteira o desenho no gephi fica ilegivel
bool exportar_gephi(const Base& base, const std::vector<int>& grupos,
                    const std::vector<std::vector<Aresta>>& adjacencia, const std::string& pasta,
                    int limite = 3000);
