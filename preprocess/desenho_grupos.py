#!/usr/bin/env python3
# Desenha o grafo entre grupos (limiar na mediana) com as articulacoes em destaque.
# Serve so ate a imagem do Gephi ficar pronta: o layout e um Fruchterman-Reingold simples feito aqui.
# Uso: python3 preprocess/desenho_grupos.py   (grava artigo/figuras/grupos_articulacoes.png)

import csv

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

ENTRADA = "resultados/gephi_articulacoes/q2"
SAIDA = "artigo/figuras/grupos_articulacoes.png"
PASSOS = 300


def main():
    nos = list(csv.DictReader(open(ENTRADA + "_nodes.csv", encoding="utf-8")))
    arestas = [(int(r["Source"]), int(r["Target"])) for r in csv.DictReader(open(ENTRADA + "_edges.csv", encoding="utf-8"))]
    # so os grupos que tem aresta: os soltos ficariam espalhados sem informacao
    usados = sorted({u for a in arestas for u in a})
    idx = {u: i for i, u in enumerate(usados)}
    n = len(usados)
    e = np.array([(idx[u], idx[v]) for u, v in arestas])

    rng = np.random.default_rng(7)
    pos = rng.uniform(-1, 1, (n, 2))
    k = np.sqrt(4.0 / n)
    temp = 0.1
    for _ in range(PASSOS):
        d = pos[:, None, :] - pos[None, :, :]
        dist = np.linalg.norm(d, axis=2) + 1e-9
        desloc = (d / dist[..., None] * (k * k / dist)[..., None]).sum(axis=1)
        delta = pos[e[:, 0]] - pos[e[:, 1]]
        dl = np.linalg.norm(delta, axis=1) + 1e-9
        f = (delta / dl[:, None]) * (dl * dl / k)[:, None]
        np.add.at(desloc, e[:, 0], -f)
        np.add.at(desloc, e[:, 1], f)
        tam = np.linalg.norm(desloc, axis=1) + 1e-9
        pos += desloc / tam[:, None] * np.minimum(tam, temp)[:, None]
        temp *= 0.985

    art = np.array([int(nos[u]["articulacao"]) for u in usados])
    dest = np.array([int(nos[u]["destaque"]) for u in usados])
    pecas = np.array([int(nos[u]["Pecas"]) for u in usados])
    fig, ax = plt.subplots(figsize=(6, 6))
    for a, b in e:
        ax.plot(pos[[a, b], 0], pos[[a, b], 1], color="#bbbbbb", lw=0.2, zorder=1)
    tam = 4 + 30 * np.sqrt(pecas / pecas.max())
    ax.scatter(*pos[art == 0].T, s=tam[art == 0], color="#5b7fa3", zorder=2, label="grupo")
    ax.scatter(*pos[(art == 1) & (dest == 0)].T, s=tam[(art == 1) & (dest == 0)], color="#e0a030", zorder=3,
               label="articulação (solta 1 ou 2 grupos)")
    ax.scatter(*pos[dest == 1].T, s=tam[dest == 1] * 2, color="#c0392b", zorder=4, label="articulação (solta 3 ou mais)")
    ax.legend(loc="lower left", fontsize=7, frameon=False)
    ax.set_axis_off()
    fig.tight_layout()
    fig.savefig(SAIDA, dpi=200)
    print(f"{n} grupos, {len(e)} arestas em {SAIDA}")


if __name__ == "__main__":
    main()
