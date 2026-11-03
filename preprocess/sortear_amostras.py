#!/usr/bin/env python3
# Sorteia as pecas da referencia (15 por categoria, 300 no total), as 500 do teste de velocidade do CLIP
# e 100 extras so de roupas para comprimento e modelagem.
# Uso: python3 preprocess/sortear_amostras.py --dados dados

import argparse
import csv
import random
from collections import defaultdict
from pathlib import Path

from aplicacao import APLICA

SEMENTE = 42
POR_CATEGORIA = 15
TAMANHO_CLIP = 500
TAMANHO_EXTRA = 100


def gravar(caminho, linhas):
    caminho.parent.mkdir(parents=True, exist_ok=True)
    with open(caminho, "w", newline="", encoding="utf-8") as arq:
        esc = csv.writer(arq, lineterminator="\n")
        esc.writerow(["id", "categoria"])
        esc.writerows(linhas)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dados", default="dados")
    args = parser.parse_args()
    pasta = Path(args.dados)

    with open(pasta / "pecas.csv", encoding="utf-8") as arq:
        pecas = [(int(l["id"]), l["categoria"]) for l in csv.DictReader(arq)]
    por_cat = defaultdict(list)
    for i, cat in pecas:
        por_cat[cat].append(i)

    gerador = random.Random(SEMENTE)
    referencia = []
    for cat in sorted(por_cat):
        for i in sorted(gerador.sample(por_cat[cat], POR_CATEGORIA)):
            referencia.append((i, cat))
    gravar(pasta / "referencia" / "amostra_300.csv", referencia)

    # o CLIP roda nessas 500 e tambem nas 300, que entram na comparacao
    usadas = {i for i, _ in referencia}
    resto = [p for p in pecas if p[0] not in usadas]
    clip = sorted(gerador.sample(resto, TAMANHO_CLIP))
    gravar(pasta / "amostra_clip_500.csv", clip)

    # comprimento e modelagem tinham so 75 e 90 pecas na referencia. as 100 extras sao so de roupas
    roupas = sorted(APLICA["comprimento"] | APLICA["modelagem"])
    usadas |= {i for i, _ in clip}
    extra = []
    for k, cat in enumerate(roupas):
        quantas = TAMANHO_EXTRA // len(roupas) + (k < TAMANHO_EXTRA % len(roupas))
        livres = [i for i in por_cat[cat] if i not in usadas]
        extra += [(i, cat) for i in sorted(gerador.sample(livres, quantas))]
    gravar(pasta / "referencia" / "amostra_extra_100.csv", extra)
    print(f"{len(referencia)} pecas na referencia, {len(clip)} no teste do CLIP, {len(extra)} extras.")


if __name__ == "__main__":
    main()
