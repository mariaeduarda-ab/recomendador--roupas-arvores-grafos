#!/usr/bin/env python3
"""Prepara a base FashionVerse-MME e grava os CSVs usados pelo C++. Roda uma vez.

Uso:
    python3 preprocess/preprocessar.py --entrada <arquivo.json> --saida dados

ANTES DE RODAR: preencha os TODO das funções de leitura de campos.
O formato do JSON ainda não foi conferido. Abra uma amostra da base primeiro.
"""

import argparse
import csv
import json
import math
import random
from pathlib import Path

import numpy as np

# Constantes fáceis de mudar.
SEMENTE = 42
FRACAO_TREINO = 0.8
TAMANHOS_SUBCONJUNTOS = [1_000, 10_000, 100_000]

# Peso de cada atributo. O valor normalizado é multiplicado por raiz(peso).
PESOS = {"L": 1.0, "a": 1.0, "b": 1.0, "variacao_cor": 1.0, "formalidade": 0.5}

# Listas feitas pela dupla. Ajustem depois de ver as descrições reais.
PALAVRAS_FORMAIS = {"formal", "elegant", "tailored", "blazer", "suit", "silk", "dress shirt"}
PALAVRAS_CASUAIS = {"casual", "sporty", "denim", "sneaker", "hoodie", "t-shirt", "relaxed"}


# ---------------------------------------------------------------------------
# Campos do JSON. Só estas funções dependem do formato da base.
# ---------------------------------------------------------------------------

def listar_pecas_brutas(dados):
    """Devolve a lista de peças do JSON."""
    # TODO: preencher depois de abrir uma amostra da base.
    raise NotImplementedError("Preencha listar_pecas_brutas com o formato real do JSON.")


def campo_id(peca):
    """Devolve o identificador da peça."""
    # TODO: preencher com o nome real do campo.
    raise NotImplementedError("Preencha campo_id.")


def campo_categoria(peca):
    """Devolve a categoria da peça, ou None se não houver."""
    # TODO: preencher com o nome real do campo.
    raise NotImplementedError("Preencha campo_categoria.")


def campo_cor_dominante(peca):
    """Devolve a cor dominante como [r, g, b] de 0 a 255, ou None se não houver."""
    # TODO: preencher com o nome real do campo e conferir a escala.
    raise NotImplementedError("Preencha campo_cor_dominante.")


def campo_paleta(peca):
    """Devolve a paleta como lista de [r, g, b] de 0 a 255, ou None se não houver."""
    # TODO: preencher com o nome real do campo e conferir a escala.
    raise NotImplementedError("Preencha campo_paleta.")


def campo_descricao(peca):
    """Devolve o texto da descrição usada na formalidade."""
    # TODO: escolher a descrição (BLIP ou LLaMA) e preencher com o nome real do campo.
    raise NotImplementedError("Preencha campo_descricao.")


def campo_looks(peca):
    """Devolve a lista de identificadores dos looks em que a peça aparece."""
    # TODO: preencher com o nome real do campo. Os looks podem estar em outra parte do JSON.
    raise NotImplementedError("Preencha campo_looks.")


# ---------------------------------------------------------------------------
# Cor
# ---------------------------------------------------------------------------

def rgb_para_lab(rgb):
    """Converte cores sRGB (0 a 255) para Lab, passando por XYZ. Iluminante D65."""
    srgb = np.asarray(rgb, dtype=float) / 255.0
    linear = np.where(srgb <= 0.04045, srgb / 12.92, ((srgb + 0.055) / 1.055) ** 2.4)

    matriz = np.array([
        [0.4124564, 0.3575761, 0.1804375],
        [0.2126729, 0.7151522, 0.0721750],
        [0.0193339, 0.1191920, 0.9503041],
    ])
    xyz = linear @ matriz.T
    branco = np.array([0.95047, 1.0, 1.08883])
    t = xyz / branco

    delta = 6.0 / 29.0
    f = np.where(t > delta ** 3, np.cbrt(t), t / (3 * delta ** 2) + 4.0 / 29.0)
    L = 116.0 * f[..., 1] - 16.0
    a = 500.0 * (f[..., 0] - f[..., 1])
    b = 200.0 * (f[..., 1] - f[..., 2])
    return np.stack([L, a, b], axis=-1)


def normalizar_lab(lab):
    """Leva L de 0 a 100 e a, b de -128 a 127 para o intervalo de 0 a 1."""
    L, a, b = lab
    return [
        float(np.clip(L / 100.0, 0.0, 1.0)),
        float(np.clip((a + 128.0) / 255.0, 0.0, 1.0)),
        float(np.clip((b + 128.0) / 255.0, 0.0, 1.0)),
    ]


def variacao_bruta(cor_lab, paleta_lab):
    """Média da distância entre as cores da paleta e a cor dominante, em Lab."""
    return float(np.mean(np.linalg.norm(paleta_lab - cor_lab, axis=1)))


def normalizar_min_max(valores):
    """Leva uma lista de números para o intervalo de 0 a 1."""
    menor = min(valores)
    maior = max(valores)
    if maior == menor:
        return [0.0 for _ in valores]
    return [(valor - menor) / (maior - menor) for valor in valores]


# ---------------------------------------------------------------------------
# Formalidade
# ---------------------------------------------------------------------------

def calcular_formalidade(texto):
    """Conta palavras formais e casuais. Devolve 0,5 quando nenhuma aparece."""
    texto = (texto or "").lower()
    formais = sum(texto.count(palavra) for palavra in PALAVRAS_FORMAIS)
    casuais = sum(texto.count(palavra) for palavra in PALAVRAS_CASUAIS)
    if formais + casuais == 0:
        return 0.5
    return formais / (formais + casuais)


# ---------------------------------------------------------------------------
# Etapas
# ---------------------------------------------------------------------------

def carregar_pecas_validas(caminho):
    """Lê o JSON e descarta peças sem cor, sem categoria ou sem look."""
    with open(caminho, encoding="utf-8") as arquivo:
        dados = json.load(arquivo)

    pecas = []
    descartadas = 0
    for bruta in listar_pecas_brutas(dados):
        categoria = campo_categoria(bruta)
        cor = campo_cor_dominante(bruta)
        paleta = campo_paleta(bruta)
        looks = campo_looks(bruta)
        if not categoria or cor is None or not paleta or not looks:
            descartadas += 1
            continue
        pecas.append({
            "origem": campo_id(bruta),
            # A vírgula separa colunas no CSV, então não pode aparecer no texto.
            "categoria": str(categoria).replace(",", " ").strip(),
            "cor": cor,
            "paleta": paleta,
            "descricao": campo_descricao(bruta),
            "looks": looks,
        })
    print(f"Peças válidas: {len(pecas)}. Descartadas: {descartadas}.")
    return pecas


def calcular_vetores(pecas):
    """Calcula os 5 números de cada peça, normaliza e aplica raiz(peso)."""
    cores = rgb_para_lab(np.array([peca["cor"] for peca in pecas], dtype=float))
    variacoes = normalizar_min_max([
        variacao_bruta(cor, rgb_para_lab(np.array(peca["paleta"], dtype=float)))
        for cor, peca in zip(cores, pecas)
    ])

    raizes = {nome: math.sqrt(peso) for nome, peso in PESOS.items()}
    for indice, (peca, cor) in enumerate(zip(pecas, cores)):
        L, a, b = normalizar_lab(cor)
        peca["id"] = indice
        peca["vetor"] = [
            L * raizes["L"],
            a * raizes["a"],
            b * raizes["b"],
            variacoes[indice] * raizes["variacao_cor"],
            calcular_formalidade(peca["descricao"]) * raizes["formalidade"],
        ]


def dividir_looks(pecas):
    """Sorteia os looks com semente fixa. Devolve {look original: (id novo, conjunto)}."""
    originais = sorted({look for peca in pecas for look in peca["looks"]}, key=str)
    random.Random(SEMENTE).shuffle(originais)
    corte = int(len(originais) * FRACAO_TREINO)
    return {
        look: (indice, "treino" if indice < corte else "teste")
        for indice, look in enumerate(originais)
    }


def gravar(pecas, divisao, caminho_pecas, caminho_looks):
    """Grava pecas.csv e looks.csv."""
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
    """Grava pecas_N.csv e looks_N.csv para os testes de tempo."""
    sorteio = random.Random(SEMENTE)
    for tamanho in TAMANHOS_SUBCONJUNTOS:
        if tamanho > len(pecas):
            print(f"Subconjunto de {tamanho} ignorado. A base tem {len(pecas)} peças.")
            continue
        amostra = sorted(sorteio.sample(pecas, tamanho), key=lambda peca: peca["id"])
        gravar(amostra, divisao, pasta / f"pecas_{tamanho}.csv", pasta / f"looks_{tamanho}.csv")


def main():
    parser = argparse.ArgumentParser(description="Prepara a base e grava os CSVs.")
    parser.add_argument("--entrada", required=True, help="arquivo JSON da FashionVerse-MME")
    parser.add_argument("--saida", default="dados", help="pasta dos CSVs (padrão: dados)")
    argumentos = parser.parse_args()

    pasta = Path(argumentos.saida)
    pasta.mkdir(parents=True, exist_ok=True)

    pecas = carregar_pecas_validas(argumentos.entrada)
    calcular_vetores(pecas)
    divisao = dividir_looks(pecas)
    gravar(pecas, divisao, pasta / "pecas.csv", pasta / "looks.csv")
    gravar_subconjuntos(pecas, divisao, pasta)
    print(f"Arquivos gravados em {pasta}.")


if __name__ == "__main__":
    main()
