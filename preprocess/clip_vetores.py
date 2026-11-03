#!/usr/bin/env python3
# Baixa as imagens em lotes, guarda o vetor do CLIP de cada uma e apaga o lote.
# Com os vetores salvos, trocar as frases do atributos_clip.py nao exige baixar nada de novo.
# Uso: python3 preprocess/clip_vetores.py --entrada Combined_Metadata.json --ids dados/catalogo.csv --saida dados/clip_vetores
#   Sem --entrada, usa so as imagens que ja estao em --imagens e nao apaga nada.

import argparse
import csv
import os
import shutil
import time
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

import numpy as np
import open_clip
import torch
from PIL import Image

from imagens import PEDIDOS_JUNTOS, baixar, endereco_final, indice
from preprocessar import carregar_pecas

MODELO = "ViT-B-32-quickgelu"
PESOS = "openai"
TAMANHO_LOTE = 2000


def codificar(modelo, preparo, arquivos, aparelho, por_vez=64):
    vetores, ok = [], []
    for k in range(0, len(arquivos), por_vez):
        fotos = []
        for arq in arquivos[k:k + por_vez]:
            try:
                with Image.open(arq) as img:
                    fotos.append(preparo(img.convert("RGB")))
                ok.append(True)
            except (OSError, ValueError):
                ok.append(False)
        if not fotos:
            continue
        with torch.no_grad():
            v = modelo.encode_image(torch.stack(fotos).to(aparelho))
        vetores.append((v / v.norm(dim=-1, keepdim=True)).cpu().numpy().astype(np.float16))
    return (np.concatenate(vetores) if vetores else np.zeros((0, 512), np.float16)), ok


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--ids", required=True)
    parser.add_argument("--saida", required=True)
    parser.add_argument("--entrada")
    parser.add_argument("--imagens", default="dados/imagens")
    parser.add_argument("--temporaria", default="dados/imagens_lote")
    parser.add_argument("--cache", default="dados/indice_zip.json")
    args = parser.parse_args()

    with open(args.ids, encoding="utf-8") as arq:
        ids = sorted({int(l["id"]) for l in csv.DictReader(arq)})
    saida = Path(args.saida)
    saida.mkdir(parents=True, exist_ok=True)

    aparelho = "cuda" if torch.cuda.is_available() else "cpu"
    if aparelho == "cpu":
        torch.set_num_threads(os.cpu_count() or 1)
    modelo, _, preparo = open_clip.create_model_and_transforms(MODELO, pretrained=PESOS, device=aparelho)
    modelo.eval()

    pecas = url = tabela = None
    if args.entrada:
        pecas = carregar_pecas(args.entrada)
        url = endereco_final()
        tabela = indice(url, Path(args.cache))
    fixas = Path(args.imagens)
    temp = Path(args.temporaria)

    inicio = time.time()
    lotes = [ids[k:k + TAMANHO_LOTE] for k in range(0, len(ids), TAMANHO_LOTE)]
    for n, lote in enumerate(lotes):
        destino = saida / f"lote_{n:03d}.npz"
        # cada lote pronto fica em disco, entao uma parada no meio continua de onde estava
        if destino.exists():
            continue
        faltam = [i for i in lote if not (fixas / f"{i}.jpg").exists()]
        if faltam and pecas is None:
            raise SystemExit(f"{len(faltam)} imagens faltando em {fixas}. Passe --entrada para baixar.")
        temp.mkdir(parents=True, exist_ok=True)

        def uma(i):
            (temp / f"{i}.jpg").write_bytes(baixar(url, tabela[pecas[i]["imagem"]]))

        with ThreadPoolExecutor(PEDIDOS_JUNTOS) as fila:
            list(fila.map(uma, faltam))
        arquivos = [(fixas if (fixas / f"{i}.jpg").exists() else temp) / f"{i}.jpg" for i in lote]
        vetores, ok = codificar(modelo, preparo, arquivos, aparelho)
        bons = [i for i, certo in zip(lote, ok) if certo]
        np.savez(destino, ids=np.array(bons, dtype=np.int32), vetores=vetores)
        shutil.rmtree(temp, ignore_errors=True)
        gasto = time.time() - inicio
        print(f"lote {n + 1}/{len(lotes)}: {len(bons)} vetores, {len(lote) - len(bons)} ilegiveis, {gasto / 60:.1f} min", flush=True)

    print(f"Pronto em {(time.time() - inicio) / 60:.1f} min.")


if __name__ == "__main__":
    main()
