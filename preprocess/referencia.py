#!/usr/bin/env python3
# Confere os lotes rotulados olhando as imagens e junta em referencia_300.csv e referencia_extra_100.csv.
# Uso: python3 preprocess/referencia.py validar dados/referencia/lote_1.json
#      python3 preprocess/referencia.py juntar

import csv
import json
import sys
from pathlib import Path

from aplicacao import aplica

PASTA = Path("dados/referencia")
NA = "não se aplica"

VALORES = {
    "estampa": ["lisa", "listrada", "xadrez", "floral", "poá", "animal print", "outra estampa", NA, "incerto"],
    "comprimento": ["curto", "médio", "longo", NA, "incerto"],
    "modelagem": ["justa", "reta", "ampla", "cropped", "oversized", NA, "incerto"],
    "estilo": ["casual", "clássico", "esportivo", "romântico", "moderno", "incerto"],
    "ocasiao": ["dia a dia", "trabalho", "festa", "esporte", "praia", "incerto"],
    "formalidade": ["baixa", "média", "alta", "incerto"],
}
COLUNAS = ["id", "categoria"] + list(VALORES)


def lote_esperado(n):
    with open(PASTA / "lotes.csv", encoding="utf-8") as arq:
        return {int(l["id"]): l["categoria"] for l in csv.DictReader(arq) if int(l["lote"]) == n}


def validar(caminho):
    n = int(Path(caminho).stem.split("_")[1])
    esperado = lote_esperado(n)
    pecas = json.loads(Path(caminho).read_text(encoding="utf-8"))
    erros = []
    vistos = set()
    for p in pecas:
        i = p.get("id")
        if i not in esperado:
            erros.append(f"id {i} nao e do lote {n}")
            continue
        if i in vistos:
            erros.append(f"id {i} repetido")
        vistos.add(i)
        if p.get("categoria") != esperado[i]:
            erros.append(f"id {i}: categoria {p.get('categoria')} deveria ser {esperado[i]}")
        for atr, ok in VALORES.items():
            v = p.get(atr)
            if v not in ok:
                erros.append(f"id {i}: {atr} = {v!r} nao e permitido")
            elif (v == NA) != (not aplica(atr, esperado[i])):
                erros.append(f"id {i}: {atr} = {v!r} nao combina com a categoria {esperado[i]}")
    for i in sorted(set(esperado) - vistos):
        erros.append(f"falta o id {i}")
    return erros


def juntar(lotes, nome):
    linhas = []
    for n in lotes:
        caminho = PASTA / f"lote_{n}.json"
        erros = validar(caminho)
        if erros:
            sys.exit(f"{caminho}: {len(erros)} erros. Rode validar primeiro.")
        linhas += json.loads(caminho.read_text(encoding="utf-8"))
    linhas.sort(key=lambda p: p["id"])
    with open(PASTA / nome, "w", newline="", encoding="utf-8") as arq:
        esc = csv.DictWriter(arq, COLUNAS, extrasaction="ignore", lineterminator="\n")
        esc.writeheader()
        esc.writerows(linhas)
    print(f"{len(linhas)} pecas em {PASTA / nome}.")


def main():
    if len(sys.argv) == 3 and sys.argv[1] == "validar":
        erros = validar(sys.argv[2])
        print("\n".join(erros) if erros else "OK: 50 pecas, so valores permitidos.")
        sys.exit(1 if erros else 0)
    if len(sys.argv) == 2 and sys.argv[1] == "juntar":
        juntar(range(1, 7), "referencia_300.csv")
        # lotes 7 e 8 sao as 100 roupas extras, sorteadas de outro jeito, entao ficam num arquivo separado
        juntar(range(7, 9), "referencia_extra_100.csv")
        return
    sys.exit(__doc__ or "uso: referencia.py validar <lote.json> | referencia.py juntar")


if __name__ == "__main__":
    main()
