#!/usr/bin/env python3
# Grava pecas_v3.csv com a cor e a nota "quanto chama atencao", de 0 a 1. Nao muda os outros CSVs.
# Uso: python3 preprocess/atributos_e.py --entrada Combined_Metadata.json --saida dados

import argparse
import csv
from pathlib import Path

import numpy as np

import regras_moda as rm
from atributos import ACHA_ESTAMPA, achar, atributo_visual, ids_do_csv, limpar
from preprocessar import TAMANHOS_SUBCONJUNTOS, carregar_pecas, rgb_para_lab

ACHA_CALMA = achar(rm.CALMAS)
ACHA_CHAMATIVA = achar(rm.CHAMATIVAS)
ACHA_CORINGA = {cat: achar(lista) for cat, lista in rm.CORINGAS.items()}


def variacao(peca):
    lab = np.array([rgb_para_lab(np.array(c, dtype=float)) for c in peca["rgb"]])
    return float(np.mean(np.linalg.norm(lab - lab[0], axis=1)))


def nota_atencao(peca, limite_variacao):
    cat = peca["categoria"]
    legenda = limpar(peca["legenda"], cat)
    desc = limpar(peca["descricao"], cat)
    texto = legenda + " " + desc

    nota = 0.5
    nota += rm.PASSO_PALAVRA * len(set(ACHA_CHAMATIVA.findall(texto)))
    nota -= rm.PASSO_PALAVRA * len(set(ACHA_CALMA.findall(texto)))

    estampa = atributo_visual(ACHA_ESTAMPA, legenda, desc)
    if estampa == "lisa":
        nota -= rm.PASSO_ESTAMPA
    elif estampa != "desconhecido":
        nota += rm.PASSO_ESTAMPA

    if cat in ACHA_CORINGA and ACHA_CORINGA[cat].search(texto):
        nota -= rm.PASSO_CORINGA
    if variacao(peca) > limite_variacao:
        nota += rm.PASSO_VARIACAO
    return min(1.0, max(0.0, nota))


def gravar(pecas, notas, ids, caminho):
    with open(caminho, "w", newline="", encoding="utf-8") as arq:
        esc = csv.writer(arq, lineterminator="\n")
        esc.writerow(["id", "cor", "atencao"])
        for i in ids:
            esc.writerow([i, pecas[i]["cor_nome"], f"{notas[i]:.2f}"])


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--entrada", required=True)
    parser.add_argument("--saida", default="dados")
    args = parser.parse_args()

    pasta = Path(args.saida)
    pecas = carregar_pecas(args.entrada)
    limite = float(np.percentile([variacao(p) for p in pecas], rm.PERCENTIL_VARIACAO))
    notas = [nota_atencao(p, limite) for p in pecas]
    gravar(pecas, notas, range(len(pecas)), pasta / "pecas_v3.csv")
    for tam in TAMANHOS_SUBCONJUNTOS:
        antigo = pasta / f"pecas_{tam}.csv"
        if antigo.exists():
            gravar(pecas, notas, ids_do_csv(antigo), pasta / f"pecas_v3_{tam}.csv")

    n = np.array(notas)
    print(f"Pecas: {len(n)}. Media {n.mean():.2f}.")
    print(f"Calmas (< 0.5): {100 * (n < 0.5).mean():.1f}%. No meio (0.5): {100 * (n == 0.5).mean():.1f}%. Chamativas (> 0.5): {100 * (n > 0.5).mean():.1f}%.")


if __name__ == "__main__":
    main()
