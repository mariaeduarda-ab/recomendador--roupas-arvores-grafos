#!/usr/bin/env python3
# Prepara a FashionVerse-MME e grava os CSVs que o C++ le. Roda uma vez.
# Uso: python3 preprocess/preprocessar.py --entrada Combined_Metadata.json --saida dados

import argparse
import csv
import json
import math
import random
import re
from pathlib import Path

import numpy as np

SEMENTE = 42
FRACAO_TREINO = 0.8
TAMANHOS_SUBCONJUNTOS = [1_000, 10_000, 100_000]

PESOS = {"L": 1.0, "a": 1.0, "b": 1.0, "variacao_cor": 1.0, "formalidade": 0.5}

PALAVRAS_FORMAIS = {"formal", "elegant", "tailored", "blazer", "suit", "silk", "sophisticated", "office"}
PALAVRAS_CASUAIS = {"casual", "sporty", "denim", "sneaker", "sneakers", "hoodie", "t-shirt", "relaxed", "laid-back"}

# a base tem o mesmo tipo com dois nomes. juntando da as 20 categorias do artigo deles
NOME_CATEGORIA = {"outwear": "outerwear", "watches": "watch"}

# 200266907_1.jpg = peca 1 do look 200266907. uns 277 arquivos fogem disso e ficam sem look
NOME_ARQUIVO = re.compile(r"^(\d+)_\d+\.jpe?g$")


def rgb_para_lab(rgb):
    srgb = np.asarray(rgb, dtype=float) / 255.0
    linear = np.where(srgb <= 0.04045, srgb / 12.92, ((srgb + 0.055) / 1.055) ** 2.4)

    matriz = np.array([
        [0.4124564, 0.3575761, 0.1804375],
        [0.2126729, 0.7151522, 0.0721750],
        [0.0193339, 0.1191920, 0.9503041],
    ])
    xyz = linear @ matriz.T
    t = xyz / np.array([0.95047, 1.0, 1.08883])  # branco D65

    delta = 6.0 / 29.0
    f = np.where(t > delta ** 3, np.cbrt(t), t / (3 * delta ** 2) + 4.0 / 29.0)
    L = 116.0 * f[..., 1] - 16.0
    a = 500.0 * (f[..., 0] - f[..., 1])
    b = 200.0 * (f[..., 1] - f[..., 2])
    return np.stack([L, a, b], axis=-1)


def normalizar_lab(lab):
    L, a, b = lab
    return [
        float(np.clip(L / 100.0, 0.0, 1.0)),
        float(np.clip((a + 128.0) / 255.0, 0.0, 1.0)),
        float(np.clip((b + 128.0) / 255.0, 0.0, 1.0)),
    ]


def normalizar_min_max(valores):
    menor = min(valores)
    maior = max(valores)
    if maior == menor:
        return [0.0 for _ in valores]
    return [(v - menor) / (maior - menor) for v in valores]


def calcular_formalidade(texto):
    # palavra inteira: com texto.count("suit") o "suitable" contava como formal,
    # e "suitable" esta em 60% das descricoes
    palavras = re.findall(r"[a-z]+(?:-[a-z]+)*", texto.lower())
    formais = sum(p in PALAVRAS_FORMAIS for p in palavras)
    casuais = sum(p in PALAVRAS_CASUAIS for p in palavras)
    if formais + casuais == 0:
        return 0.5
    return formais / (formais + casuais)


def carregar_pecas(caminho):
    with open(caminho, encoding="utf-8") as arquivo:
        dados = json.load(arquivo)

    # o nome do arquivo e unico por look, entao cada peca apareceria em um look so e a BFS
    # nunca sairia do look. a mesma imagem da mesma categoria tem o mesmo rgb e a mesma legenda
    # do BLIP (a legenda refinada muda), entao essa chave junta a peca que se repete
    pecas = {}
    descartadas = 0
    for item in dados:
        look = NOME_ARQUIVO.match(item["image_name"])
        rgb = item["dominant_rgb_top3"]
        if not look or not item["category"] or not rgb:
            descartadas += 1
            continue
        categoria = NOME_CATEGORIA.get(item["category"], item["category"])
        chave = (categoria, json.dumps(rgb), item["base_caption"])
        if chave not in pecas:
            pecas[chave] = {
                "categoria": categoria,
                "rgb": rgb,
                "descricao": item["refined_caption"],
                "legenda": item["base_caption"],
                "cor_nome": item["dominant_color"],
                # pasta e nome dentro do zip. a pasta nem sempre e igual ao campo category
                "imagem": item["image_path"].split("/")[-2] + "/" + item["image_name"],
                "looks": set(),
            }
        pecas[chave]["looks"].add(int(look.group(1)))

    lista = list(pecas.values())
    print(f"Registros descartados: {descartadas}. Pecas depois de juntar repetidas: {len(lista)}.")
    return lista


def calcular_vetores(pecas):
    labs = [rgb_para_lab(np.array(p["rgb"], dtype=float)) for p in pecas]
    # o primeiro rgb e a cor dominante. peca lisa fica com as outras perto dele
    variacoes = normalizar_min_max([float(np.mean(np.linalg.norm(lab - lab[0], axis=1))) for lab in labs])

    raizes = {nome: math.sqrt(peso) for nome, peso in PESOS.items()}
    for i, peca in enumerate(pecas):
        L, a, b = normalizar_lab(labs[i][0])
        peca["id"] = i
        peca["vetor"] = [
            L * raizes["L"],
            a * raizes["a"],
            b * raizes["b"],
            variacoes[i] * raizes["variacao_cor"],
            calcular_formalidade(peca["descricao"]) * raizes["formalidade"],
        ]


def dividir_looks(pecas):
    originais = sorted({look for peca in pecas for look in peca["looks"]})
    random.Random(SEMENTE).shuffle(originais)
    corte = int(len(originais) * FRACAO_TREINO)
    return {look: (i, "treino" if i < corte else "teste") for i, look in enumerate(originais)}


def gravar(pecas, divisao, caminho_pecas, caminho_looks):
    with open(caminho_pecas, "w", newline="", encoding="utf-8") as arquivo:
        escritor = csv.writer(arquivo, lineterminator="\n")
        escritor.writerow(["id", "categoria", "L", "a", "b", "variacao_cor", "formalidade"])
        for peca in pecas:
            escritor.writerow([peca["id"], peca["categoria"]] + [f"{x:.6f}" for x in peca["vetor"]])

    linhas = []
    for peca in pecas:
        for look in peca["looks"]:
            look_id, conjunto = divisao[look]
            linhas.append((look_id, peca["id"], conjunto))
    linhas.sort()
    with open(caminho_looks, "w", newline="", encoding="utf-8") as arquivo:
        escritor = csv.writer(arquivo, lineterminator="\n")
        escritor.writerow(["look_id", "peca_id", "conjunto"])
        escritor.writerows(linhas)


def gravar_subconjuntos(pecas, divisao, pasta):
    sorteio = random.Random(SEMENTE)
    for tamanho in TAMANHOS_SUBCONJUNTOS:
        if tamanho > len(pecas):
            print(f"Subconjunto de {tamanho} ignorado. A base tem {len(pecas)} pecas.")
            continue
        amostra = sorted(sorteio.sample(pecas, tamanho), key=lambda p: p["id"])
        gravar(amostra, divisao, pasta / f"pecas_{tamanho}.csv", pasta / f"looks_{tamanho}.csv")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--entrada", required=True)
    parser.add_argument("--saida", default="dados")
    args = parser.parse_args()

    pasta = Path(args.saida)
    pasta.mkdir(parents=True, exist_ok=True)

    pecas = carregar_pecas(args.entrada)
    calcular_vetores(pecas)
    divisao = dividir_looks(pecas)
    gravar(pecas, divisao, pasta / "pecas.csv", pasta / "looks.csv")
    gravar_subconjuntos(pecas, divisao, pasta)
    print(f"Arquivos gravados em {pasta}.")


if __name__ == "__main__":
    main()
