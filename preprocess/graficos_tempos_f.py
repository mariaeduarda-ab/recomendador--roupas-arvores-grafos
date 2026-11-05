#!/usr/bin/env python3
# Graficos de tempo da versao F, a partir de resultados/tempos_f/*.csv.
# Uso: python3 preprocess/graficos_tempos_f.py

import csv
import math

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

PASTA = "resultados/tempos_f"
CORES = ["#2a78d6", "#eb6834", "#1baf7a", "#eda100", "#e87ba4"]

# curvas teoricas. cada uma e ajustada para passar pelo primeiro ponto da serie
TEORIA = {
    "1": lambda n: 1.0,
    "log n": lambda n: math.log2(n),
    "n": lambda n: n,
    "n log n": lambda n: n * math.log2(n),
    "n²": lambda n: n * n,
}
TRACO = {"1": (0, (1, 2)), "log n": (0, (2, 2)), "n": (0, (5, 3)), "n log n": (0, (8, 2, 2, 2)), "n²": (0, (1, 1))}

GRAFICOS = [
    ("kdtree.csv", "kdtree.png", "KD-tree e kNN (k = 10)", [
        ("construir_kdtree", "Construir a KD-tree", "n log n"),
        ("knn_kdtree", "kNN na KD-tree (por consulta)", "log n"),
        ("knn_forca_bruta", "kNN na força bruta (por consulta)", "n"),
    ]),
    ("union_find.csv", "union_find.png", "Union-Find: n − 1 uniões e n buscas", [
        ("uf_sem_tecnicas", "Sem as duas técnicas", "n²"),
        ("uf_so_tamanho", "Só união por tamanho", "n log n"),
        ("uf_so_compressao", "Só compressão de caminho", "n"),
        ("uf_tamanho_e_compressao", "Tamanho e compressão", "n"),
    ]),
    ("agm.csv", "agm.png", "Árvore geradora mínima no grafo kNN (k = 10)", [
        ("kruskal_ordenar_arestas", "Kruskal: ordenar as arestas", "n log n"),
        ("kruskal_uniao", "Kruskal: laço com Union-Find", "n"),
        ("prim_heap", "Prim com heap", "n log n"),
        ("montar_grafo_knn", "Montar o grafo kNN", "n log n"),
    ]),
    ("bfs.csv", "bfs.png", "BFS de 2 passos no grafo peça–look (por consulta)", [
        ("bfs_2_passos", "BFS de 2 passos", "1"),
    ]),
]


def ler(nome):
    with open(f"{PASTA}/{nome}", encoding="utf-8") as arquivo:
        return list(csv.DictReader(arquivo))


def desenhar(arquivo, saida, titulo, series):
    linhas = ler(arquivo)
    fig, ax = plt.subplots(figsize=(7, 4.6))
    for cor, (medida, rotulo, teoria) in zip(CORES, series):
        pontos = sorted((int(l["n"]), float(l["media_ms"]), float(l["desvio_ms"]))
                        for l in linhas if l["medida"] == medida)
        if not pontos:
            continue
        ns = [p[0] for p in pontos]
        ts = [p[1] for p in pontos]
        ax.errorbar(ns, ts, yerr=[p[2] for p in pontos], color=cor, lw=2, marker="o", ms=7,
                    capsize=3, label=rotulo, zorder=3)
        f = TEORIA[teoria]
        base = ts[0] / f(ns[0])
        ax.plot(ns, [base * f(n) for n in ns], color=cor, lw=1.2, ls=TRACO[teoria], alpha=0.8,
                label=f"{rotulo}: curva {teoria}", zorder=2)
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlabel("Peças (n)")
    ax.set_ylabel("Tempo médio (ms), escala log")
    ax.set_title(titulo, fontsize=11)
    ax.grid(True, which="major", color="#e4e3df", lw=0.8)
    for lado in ("top", "right"):
        ax.spines[lado].set_visible(False)
    ax.legend(fontsize=8, frameon=False, loc="upper left")
    fig.tight_layout()
    fig.savefig(f"{PASTA}/{saida}", dpi=150)
    plt.close(fig)


if __name__ == "__main__":
    for arquivo, saida, titulo, series in GRAFICOS:
        desenhar(arquivo, saida, titulo, series)
    print(f"Graficos em {PASTA}/")
