#include "grafo_looks.hpp"

void GrafoLooks::construir(const Base& base) {
    // TODO: dimensionar as listas e percorrer base.looks.
    // TODO: ignorar looks de teste. Se entrarem, o resultado da avaliação fica falso.
    (void)base;
}

std::unordered_map<int, double> GrafoLooks::bfs_2_passos(int peca, bool ajuste_popularidade) const {
    // TODO: passo 1. Visitar looks_da_peca[peca].
    // TODO: passo 2. Visitar pecas_do_look[l] e somar 1 em cada outra peça. Parar aqui.
    // TODO: se ajuste_popularidade, dividir cada contagem por sqrt(num_looks(candidata)).
    (void)peca;
    (void)ajuste_popularidade;
    return {};
}

int GrafoLooks::num_looks(int peca) const {
    // TODO: devolver o tamanho de looks_da_peca[peca].
    (void)peca;
    return 0;
}
