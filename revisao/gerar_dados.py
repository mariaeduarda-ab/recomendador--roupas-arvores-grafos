#!/usr/bin/env python3
# Sorteia 50 das 300 pecas e grava revisao/dados.js para a pagina revisar.html.
# Uso: python3 revisao/gerar_dados.py [--teste]

import argparse
import csv
import json
import random
import sys
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(RAIZ / "preprocess"))

from aplicacao import aplica  # noqa: E402
from preprocessar import carregar_pecas  # noqa: E402
from referencia import NA, VALORES  # noqa: E402

PASTA = RAIZ / "dados" / "referencia"
SAIDA = RAIZ / "revisao"
METADADOS = Path.home() / "fashionverse" / "Combined_Metadata.json"
SEMENTE = 42
TOTAL = 50


def sortear(amostra):
    por_cat = {}
    for p in amostra:
        por_cat.setdefault(p["categoria"], []).append(p["id"])
    cats = sorted(por_cat)
    rng = random.Random(SEMENTE)
    # 50 nao divide por 20: umas categorias levam uma peca a mais, sorteadas tambem
    base, sobra = divmod(TOTAL, len(cats))
    extras = set(rng.sample(cats, sobra))
    escolhidas = []
    for c in cats:
        ids = sorted(por_cat[c])
        n = min(len(ids), base + (c in extras))
        escolhidas += [(i, c) for i in sorted(rng.sample(ids, n))]
    return escolhidas


def ler_rotulos():
    final = PASTA / "referencia_300.csv"
    if final.exists():
        with open(final, encoding="utf-8") as arq:
            return {int(l["id"]): l for l in csv.DictReader(arq)}, final.name
    rotulos = {}
    lidos = []
    for caminho in sorted(PASTA.glob("lote_*.json")):
        try:
            pecas = json.loads(caminho.read_text(encoding="utf-8"))
        except (OSError, ValueError) as erro:
            # outro agente pode estar no meio da escrita do lote
            print(f"aviso: {caminho.name} ignorado ({erro})")
            continue
        for p in pecas:
            if isinstance(p, dict) and "id" in p:
                rotulos[int(p["id"])] = p
        lidos.append(caminho.name)
    return rotulos, ", ".join(lidos) or "nenhum lote"


def main():
    args = argparse.ArgumentParser()
    args.add_argument("--teste", action="store_true", help="preenche rotulos que faltam com valores de exemplo")
    teste = args.parse_args().teste

    with open(PASTA / "amostra_300.csv", encoding="utf-8") as arq:
        amostra = [{"id": int(l["id"]), "categoria": l["categoria"]} for l in csv.DictReader(arq)]
    escolhidas = sortear(amostra)
    rotulos, origem = ler_rotulos()
    pecas = carregar_pecas(METADADOS)

    saida = []
    faltam = []
    rng = random.Random(SEMENTE)
    for i, cat in escolhidas:
        texto = pecas[i] if i < len(pecas) else {}
        if texto and texto.get("categoria") != cat:
            print(f"aviso: id {i} e {texto.get('categoria')} nos metadados e {cat} na amostra")
        rot = rotulos.get(i, {})
        valores = {}
        for atr, ok in VALORES.items():
            if not aplica(atr, cat):
                valores[atr] = NA
                continue
            v = rot.get(atr)
            if v not in ok or v == NA:
                if not teste:
                    faltam.append(i)
                    v = ""
                else:
                    v = rng.choice([x for x in ok if x not in (NA, "incerto")])
            valores[atr] = v
        saida.append({
            "id": i,
            "categoria": cat,
            "imagem": f"../dados/imagens/{i}.jpg",
            "legenda": texto.get("legenda", ""),
            "descricao": texto.get("descricao", ""),
            "rotulos": valores,
        })

    dados = {
        "valores": VALORES,
        "na": NA,
        "aplica": {atr: sorted({p["categoria"] for p in amostra if aplica(atr, p["categoria"])}) for atr in VALORES},
        "teste": teste,
        "pecas": saida,
    }
    with open(SAIDA / "dados.js", "w", encoding="utf-8") as arq:
        arq.write("const DADOS = " + json.dumps(dados, ensure_ascii=False, indent=1) + ";\n")
    with open(SAIDA / "amostra_50.csv", "w", newline="", encoding="utf-8") as arq:
        esc = csv.writer(arq, lineterminator="\n")
        esc.writerow(["id", "categoria"])
        esc.writerows(escolhidas)

    print(f"{len(saida)} pecas. Rotulos de: {origem}.")
    if faltam:
        print(f"aviso: {len(set(faltam))} pecas sem rotulo completo ficam em branco na pagina")
    if teste:
        print("modo --teste: valores de exemplo, nao sao os rotulos de verdade")


if __name__ == "__main__":
    main()
