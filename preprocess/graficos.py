#!/usr/bin/env python3
# Gera os gráficos do artigo a partir dos csv de resultados.
# Uso: python3 preprocess/graficos.py

import csv

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

PASTA = "resultados"


# os nomes no csv ficam sem acento
ACENTOS = {
    "Aleatoria (esperado)": "Aleatória (esperado)",
    "D sem bonus de looks": "D sem bônus de looks",
    "D com kNN mutuo": "D com kNN mútuo",
}


def ler(nome):
    with open(f"{PASTA}/{nome}", encoding="utf-8") as arquivo:
        return list(csv.DictReader(arquivo))


def barras():
    linhas = [l for l in ler("hit_d.csv") if l["versao"] != "Aleatoria (sorteio)"]
    nomes = [ACENTOS.get(l["versao"], l["versao"]) for l in linhas]
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
        # C fica de fora: a medida dela usa look de 1 peça e a de D usa looks reais
        ("KD-tree única", velhos, "construir_kdtree_ms"),
        ("KD-trees por categoria", novos, "arvores_ms"),
        ("Arestas da versão D", novos, "arestas_ms"),
        ("Recomendação D", novos, "recomendacao_ms"),
    ]
    fig, ax = plt.subplots(figsize=(8, 5))
    for rotulo, dados, coluna in series:
        ax.plot([int(l["pecas"]) for l in dados], [float(l[coluna]) for l in dados], marker="o", label=rotulo)
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlabel("Peças")
    ax.set_ylabel("Tempo (ms)")
    ax.legend()
    fig.tight_layout()
    fig.savefig(f"{PASTA}/tempos.png", dpi=150)


if __name__ == "__main__":
    barras()
    linhas_tempo()
