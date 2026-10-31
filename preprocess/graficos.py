#!/usr/bin/env python3
# Gera os graficos do artigo a partir dos csv de resultados.
# Uso: python3 preprocess/graficos.py

import csv

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

PASTA = "resultados"


def ler(nome):
    with open(f"{PASTA}/{nome}", encoding="utf-8") as arquivo:
        return list(csv.DictReader(arquivo))


def barras():
    linhas = [l for l in ler("hit_d.csv") if l["versao"] != "Aleatoria (sorteio)"]
    nomes = [l["versao"] for l in linhas]
    y = range(len(nomes))
    fig, ax = plt.subplots(figsize=(8, 5))
    for i, (coluna, rotulo) in enumerate([("hit1", "Hit@1"), ("hit5", "Hit@5"), ("hit10", "Hit@10")]):
        ax.barh([p + i * 0.27 for p in y], [100 * float(l[coluna]) for l in linhas], height=0.27, label=rotulo)
    ax.set_yticks([p + 0.27 for p in y])
    ax.set_yticklabels(nomes)
    ax.invert_yaxis()
    ax.set_xlabel("Acerto (%)")
    ax.legend()
    fig.tight_layout()
    fig.savefig(f"{PASTA}/acertos.png", dpi=150)


def linhas_tempo():
    velhos = ler("tempos.csv")
    novos = ler("tempos_d.csv")
    series = [
        ("KD-tree unica", velhos, "construir_kdtree_ms"),
        ("KD-trees por categoria", novos, "arvores_ms"),
        ("Arestas da versao D", novos, "arestas_ms"),
        ("Recomendacao C", velhos, "recomendacao_c_ms"),
        ("Recomendacao D", novos, "recomendacao_ms"),
    ]
    fig, ax = plt.subplots(figsize=(8, 5))
    for rotulo, dados, coluna in series:
        ax.plot([int(l["pecas"]) for l in dados], [float(l[coluna]) for l in dados], marker="o", label=rotulo)
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlabel("Pecas")
    ax.set_ylabel("Tempo (ms)")
    ax.legend()
    fig.tight_layout()
    fig.savefig(f"{PASTA}/tempos.png", dpi=150)


if __name__ == "__main__":
    barras()
    linhas_tempo()
