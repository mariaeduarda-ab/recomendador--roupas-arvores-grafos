#include "kdtree.hpp"

#include <algorithm>
#include <numeric>

namespace {

void guardar(HeapMax& heap, int k, double d2, int i) {
    if ((int)heap.size() < k) {
        heap.push({d2, i});
    } else if (d2 < heap.top().first) {
        heap.pop();
        heap.push({d2, i});
    }
}

std::vector<Vizinho> esvaziar(HeapMax& heap) {
    std::vector<Vizinho> res(heap.size());
    for (int i = (int)res.size() - 1; i >= 0; i--) {
        res[i].dist2 = heap.top().first;
        res[i].indice = heap.top().second;
        heap.pop();
    }
    return res;
}

}  // namespace

KDTree::KDTree(int dim) : dim_(dim) {}

void KDTree::construir(const std::vector<Peca>& pecas) {
    pecas_ = &pecas;
    nos_.clear();
    nos_.reserve(pecas.size());
    std::vector<int> ids(pecas.size());
    std::iota(ids.begin(), ids.end(), 0);
    raiz_ = montar(ids, 0, (int)ids.size(), 0);
}

int KDTree::montar(std::vector<int>& ids, int ini, int fim, int prof) {
    if (ini >= fim) return -1;
    int eixo = prof % dim_;
    int meio = (ini + fim) / 2;
    const std::vector<Peca>& pecas = *pecas_;
    // desempata pelo indice pra arvore sair igual em toda execucao
    std::nth_element(ids.begin() + ini, ids.begin() + meio, ids.begin() + fim, [&](int a, int b) {
        if (pecas[a].v[eixo] != pecas[b].v[eixo]) return pecas[a].v[eixo] < pecas[b].v[eixo];
        return a < b;
    });

    int atual = (int)nos_.size();
    nos_.push_back(No{});
    nos_[atual].indice = ids[meio];
    nos_[atual].eixo = eixo;
    int esq = montar(ids, ini, meio, prof + 1);
    int dir = montar(ids, meio + 1, fim, prof + 1);
    nos_[atual].esquerda = esq;
    nos_[atual].direita = dir;
    return atual;
}

void KDTree::pre(int no, std::vector<int>& res) const {
    if (no < 0) return;
    res.push_back(nos_[no].indice);
    pre(nos_[no].esquerda, res);
    pre(nos_[no].direita, res);
}

std::vector<int> KDTree::preordem() const {
    std::vector<int> res;
    pre(raiz_, res);
    return res;
}

bool KDTree::passa_filtro(int i, const std::string* categoria, int excluir) const {
    if (i == excluir) return false;
    return categoria == nullptr || (*pecas_)[i].categoria == *categoria;
}

void KDTree::buscar(int no, Busca& b) const {
    if (no < 0) return;
    b.visitados++;
    const No& n = nos_[no];
    const std::vector<double>& v = (*pecas_)[n.indice].v;

    if (passa_filtro(n.indice, b.categoria, b.excluir)) {
        guardar(b.heap, b.k, distancia2(b.q, v), n.indice);
    }

    double dif = b.q[n.eixo] - v[n.eixo];
    buscar(dif < 0 ? n.esquerda : n.direita, b);
    // o outro lado esta a pelo menos |dif|. com heap cheio da pra comparar com o pior
    if ((int)b.heap.size() < b.k || dif * dif < b.heap.top().first) {
        buscar(dif < 0 ? n.direita : n.esquerda, b);
    }
}

Vizinho KDTree::mais_proximo(const std::vector<double>& q, int& visitados) const {
    Busca b{q, 1, nullptr, -1, HeapMax(), 0};
    buscar(raiz_, b);
    visitados = b.visitados;
    std::vector<Vizinho> res = esvaziar(b.heap);
    return res.empty() ? Vizinho{} : res[0];
}

std::vector<Vizinho> KDTree::knn(const std::vector<double>& q, int k, const std::string* categoria,
                                 int excluir) const {
    if (k <= 0) return {};
    Busca b{q, k, categoria, excluir, HeapMax(), 0};
    buscar(raiz_, b);
    return esvaziar(b.heap);
}

std::vector<Vizinho> KDTree::forca_bruta(const std::vector<double>& q, int k,
                                         const std::string* categoria, int excluir) const {
    if (k <= 0) return {};
    HeapMax heap;
    for (int i = 0; i < (int)pecas_->size(); i++) {
        if (passa_filtro(i, categoria, excluir)) guardar(heap, k, distancia2(q, (*pecas_)[i].v), i);
    }
    return esvaziar(heap);
}
