#pragma once

#include <string>
#include <vector>

#include "grafo_similaridade.hpp"
#include "leitura.hpp"

// Grava nodes.csv (Id, Label, Categoria, Grupo) e edges.csv (Source, Target, Weight, Type).
// Exporta uma amostra de até limite peças para o desenho ficar legível.
// Uma aresta só entra se as duas pontas estão na amostra.
bool exportar_gephi(const Base& base, const std::vector<int>& grupos,
                    const std::vector<std::vector<Aresta>>& adjacencia, const std::string& pasta,
                    int limite = 3000);
