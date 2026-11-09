#!/usr/bin/env python3
# Monta a pasta zenodo/ com os dados que o projeto gerou, cada peca com o id original da FashionVerse-MME.
# Nao copia imagens nem legendas: as imagens vem do Polyvore, que e de terceiros.
# Uso: python3 preprocess/zenodo.py --entrada Combined_Metadata.json
#   antes: ./recomendador exportar_grupos (grava zenodo/grupos.csv)

import argparse
import csv
import json
import shutil
from collections import defaultdict
from pathlib import Path

from preprocessar import NOME_ARQUIVO, NOME_CATEGORIA, carregar_pecas, dividir_looks


def ler(caminho):
    with open(caminho, encoding="utf-8") as arq:
        return list(csv.DictReader(arq))


def gravar(caminho, cabecalho, linhas):
    with open(caminho, "w", newline="", encoding="utf-8") as arq:
        w = csv.writer(arq, lineterminator="\n")
        w.writerow(cabecalho)
        w.writerows(linhas)
    print(f"{caminho}: {len(linhas)} linhas")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--entrada", required=True)
    parser.add_argument("--dados", default="dados")
    parser.add_argument("--resultados", default="resultados")
    parser.add_argument("--saida", default="zenodo")
    args = parser.parse_args()
    dados, res, saida = Path(args.dados), Path(args.resultados), Path(args.saida)
    saida.mkdir(exist_ok=True)
    if not (saida / "grupos.csv").exists():
        raise SystemExit("Falta zenodo/grupos.csv. Rode ./recomendador exportar_grupos antes.")

    pecas = carregar_pecas(args.entrada)
    catalogo = ler(dados / "pecas.csv")
    if len(catalogo) != len(pecas) or any(catalogo[i]["categoria"] != pecas[i]["categoria"] for i in range(len(pecas))):
        raise SystemExit("A ordem das pecas nao bate com dados/pecas.csv.")

    # todos os registros originais de cada peca. a chave e a mesma do preprocessar
    indice = {(p["categoria"], json.dumps(p["rgb"]), p["legenda"]): i for i, p in enumerate(pecas)}
    registros = []
    for item in json.load(open(args.entrada, encoding="utf-8")):
        if not NOME_ARQUIVO.match(item["image_name"]) or not item["category"] or not item["dominant_rgb_top3"]:
            continue
        cat = NOME_CATEGORIA.get(item["category"], item["category"])
        i = indice[(cat, json.dumps(item["dominant_rgb_top3"]), item["base_caption"])]
        registros.append((i, item["image_name"], item["image_path"].split("/")[-2]))
    registros.sort()
    gravar(saida / "registros_originais.csv", ["id", "image_name", "pasta_original"], registros)

    n_reg = defaultdict(int)
    for i, _, _ in registros:
        n_reg[i] += 1
    gravar(saida / "pecas.csv", ["id", "categoria", "imagem_original", "registros"],
           [(i, p["categoria"], p["imagem"], n_reg[i]) for i, p in enumerate(pecas)])

    # atributos: vetor do catalogo + atributos do texto + grupo de cor. modelagem saiu do projeto
    v5 = ler(dados / "pecas_v5.csv")
    cols = [c for c in v5[0].keys() if c not in ("id", "categoria", "modelagem")]
    linhas = []
    for i, (c, a) in enumerate(zip(catalogo, v5)):
        if int(a["id"]) != i:
            raise SystemExit(f"pecas_v5.csv fora de ordem na linha {i}")
        linhas.append([i, c["categoria"], c["variacao_cor"], c["formalidade"]] + [a[k] for k in cols])
    gravar(saida / "atributos.csv", ["id", "categoria", "variacao_cor", "formalidade_vetor"] + cols, linhas)

    # divisao dos looks: treino, validacao e teste, com o numero original do conjunto do Polyvore
    original = {novo: look for look, (novo, _) in dividir_looks(pecas).items()}
    validacao = {int(l["look_id"]) for l in ler(dados / "looks_validacao.csv")}
    looks = []
    for l in ler(dados / "looks.csv"):
        lid = int(l["look_id"])
        conjunto = "validacao" if lid in validacao else l["conjunto"]
        looks.append((lid, original[lid], int(l["peca_id"]), conjunto))
    gravar(saida / "looks.csv", ["look_id", "look_original", "peca_id", "conjunto"], looks)

    shutil.copy(res / "escondidas.csv", saida / "escondidas_teste.csv")
    destino = saida / "tabelas_pontos"
    destino.mkdir(exist_ok=True)
    for f in sorted((res / "tabelas_pontos").glob("*.csv")):
        shutil.copy(f, destino / f.name)
    print(f"Tabelas de pontos copiadas para {destino}")


if __name__ == "__main__":
    main()
