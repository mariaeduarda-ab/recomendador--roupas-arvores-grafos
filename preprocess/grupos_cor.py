#!/usr/bin/env python3
# Junta as cores Lab de todas as pecas em 12 grupos com k-means e da a cada grupo o nome da cor
# de referencia mais perto. Nao usa os looks, entao nao vaza nada do teste.
# Grava pecas_v5.csv = pecas_v4.csv + coluna grupo_cor (arquivo novo) e resultados/grupos_cor.md.
# Uso: python3 preprocess/grupos_cor.py [--entrada dados/pecas_v4.csv] [--saida dados/pecas_v5.csv]

import argparse
import csv
from collections import Counter
from pathlib import Path

import numpy as np

from preprocessar import rgb_para_lab

K = 12
SEMENTE = 42
MAX_RODADAS = 300

# rgb tipico de cada nome. o Lab sai pela mesma conta do preprocessar.py
REFERENCIAS = {
    "preto": (20, 20, 20),
    "branco": (245, 245, 245),
    "cinza": (128, 128, 128),
    "bege": (215, 195, 160),
    "marrom": (110, 70, 40),
    "jeans": (75, 105, 145),
    "marinho": (25, 35, 70),
    "vermelho": (190, 30, 40),
    "vinho": (110, 25, 45),
    "rosa": (230, 160, 180),
    "verde": (60, 120, 60),
    "amarelo": (235, 205, 60),
    "laranja": (230, 120, 40),
    "roxo": (110, 60, 130),
}


def escala(lab_csv):
    # o csv guarda L/100 e (a+128)/255. com a e b vezes 2,55 a distancia vira delta E / 100
    x = np.array(lab_csv, dtype=float)
    x[:, 1:] *= 2.55
    return x


def de_verdade(x):
    # volta para L, a, b em unidade de verdade
    return np.stack([x[:, 0] * 100.0, x[:, 1] * 100.0 - 128.0, x[:, 2] * 100.0 - 128.0], axis=1)


def kmeans(x, k, semente):
    rng = np.random.default_rng(semente)
    # k-means++: cada centro novo sai com chance proporcional a distancia ao centro mais perto
    centros = [x[rng.integers(len(x))]]
    d2 = ((x - centros[0]) ** 2).sum(axis=1)
    for _ in range(1, k):
        i = rng.choice(len(x), p=d2 / d2.sum())
        centros.append(x[i])
        d2 = np.minimum(d2, ((x - x[i]) ** 2).sum(axis=1))
    centros = np.array(centros)

    grupo = np.full(len(x), -1)
    for rodada in range(MAX_RODADAS):
        dist = ((x[:, None, :] - centros[None, :, :]) ** 2).sum(axis=2)
        novo = dist.argmin(axis=1)
        if (novo == grupo).all():
            break
        grupo = novo
        for g in range(k):
            membros = x[grupo == g]
            # grupo vazio fica com o centro antigo
            if len(membros) > 0:
                centros[g] = membros.mean(axis=0)
    return grupo, centros, rodada + 1


def dar_nomes(centros):
    nomes_ref = list(REFERENCIAS)
    ref = rgb_para_lab(np.array([REFERENCIAS[n] for n in nomes_ref], dtype=float))
    ref = np.stack([ref[:, 0] / 100.0, (ref[:, 1] + 128.0) / 100.0, (ref[:, 2] + 128.0) / 100.0], axis=1)
    perto = [nomes_ref[((ref - c) ** 2).sum(axis=1).argmin()] for c in centros]
    # nome repetido ganha numero, 1 para o grupo mais escuro
    nomes = list(perto)
    for nome, vezes in Counter(perto).items():
        if vezes < 2:
            continue
        gs = sorted((g for g in range(len(centros)) if perto[g] == nome), key=lambda g: centros[g][0])
        for i, g in enumerate(gs):
            nomes[g] = f"{nome} {i + 1}"
    return nomes


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--entrada", default="dados/pecas_v4.csv")
    parser.add_argument("--saida", default="dados/pecas_v5.csv")
    parser.add_argument("--resumo", default="resultados/grupos_cor.md")
    args = parser.parse_args()

    saida = Path(args.saida)
    if saida.exists():
        raise SystemExit(f"{saida} ja existe. Nao sobrescrevo arquivo de dados.")

    with open(args.entrada, encoding="utf-8", newline="") as arq:
        leitor = csv.DictReader(arq)
        colunas = leitor.fieldnames
        linhas = list(leitor)
    x = escala([[float(l["L"]), float(l["a"]), float(l["b"])] for l in linhas])

    grupo, centros, rodadas = kmeans(x, K, SEMENTE)
    nomes = dar_nomes(centros)

    with open(saida, "w", encoding="utf-8", newline="") as arq:
        escritor = csv.DictWriter(arq, fieldnames=colunas + ["grupo_cor"], lineterminator="\n")
        escritor.writeheader()
        for l, g in zip(linhas, grupo):
            escritor.writerow({**l, "grupo_cor": nomes[g]})

    tam = np.bincount(grupo, minlength=K)
    lab = de_verdade(centros)
    # distancia media ao centro em delta E, pra mostrar quanto cada grupo e espalhado
    raio = [100.0 * np.sqrt(((x[grupo == g] - centros[g]) ** 2).sum(axis=1)).mean() if tam[g] else 0.0
            for g in range(K)]
    s = ["# Grupos de cor (k-means)", "",
         f"k-means com {K} grupos na cor Lab das {len(linhas)} peças de {args.entrada}, semente {SEMENTE}, "
         f"início k-means++, parou em {rodadas} rodadas. Os looks não entram nesta conta.",
         "",
         "Escala: L/100 e a/100, b/100 (o a e o b do CSV vezes 2,55). Assim a distância é o delta E dividido por 100. "
         "O nome é a cor de referência mais perto do centro. Nome repetido ganha número, 1 para o grupo mais escuro.",
         "",
         "| grupo | peças | % | L | a | b | distância média ao centro (delta E) |",
         "| --- | --- | --- | --- | --- | --- | --- |"]
    for g in sorted(range(K), key=lambda g: -tam[g]):
        s.append(f"| {nomes[g]} | {tam[g]} | {100 * tam[g] / len(linhas):.1f} | {lab[g][0]:.1f} | "
                 f"{lab[g][1]:.1f} | {lab[g][2]:.1f} | {raio[g]:.1f} |".replace(".", ","))
    s += ["", "Cores de referência (RGB):", ""]
    s += [f"- {n}: {r}" for n, r in REFERENCIAS.items()]
    Path(args.resumo).write_text("\n".join(s) + "\n", encoding="utf-8")
    print(f"{saida}: {len(linhas)} peças, {rodadas} rodadas")
    for g in sorted(range(K), key=lambda g: -tam[g]):
        print(nomes[g], tam[g], np.round(lab[g], 1))


if __name__ == "__main__":
    main()
