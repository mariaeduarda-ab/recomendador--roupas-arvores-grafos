#!/usr/bin/env python3
# Baixa so as imagens pedidas de dentro do zip da FashionVerse, sem baixar os 3,6 GB.
# O zip fica no Mendeley e aceita pedidos por faixa de bytes, entao cada imagem e um pedido.
# Uso: python3 preprocess/imagens.py --entrada Combined_Metadata.json --ids lista.csv --saida dados/imagens
#   lista.csv precisa de uma coluna id. Grava tambem dados/imagens.csv com id,imagem de todas as pecas.

import argparse
import csv
import io
import json
import struct
import subprocess
import zipfile
import zlib
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from preprocessar import carregar_pecas

URL = "https://data.mendeley.com/public-files/datasets/6sx3pbsfvz/files/2ae1ddea-cd02-4c2b-b22b-d88c84d30b56/file_downloaded"
TAMANHO_ZIP = 3610669167
PASTA_NO_ZIP = "FashionVerse-MME/"
PEDIDOS_JUNTOS = 8


def endereco_final():
    # o link do Mendeley redireciona para um endereco que expira, entao ele e pedido de novo quando falha
    r = subprocess.run(["curl", "-s", "-L", "-r", "0-0", "-o", "/dev/null", "-w", "%{url_effective}", URL],
                       check=True, capture_output=True, text=True)
    return r.stdout


def faixa(url, inicio, fim):
    for _ in range(3):
        r = subprocess.run(["curl", "-s", "--fail", "-r", f"{inicio}-{fim}", url], capture_output=True)
        if r.returncode == 0 and len(r.stdout) == fim - inicio + 1:
            return r.stdout, url
        url = endereco_final()
    raise RuntimeError(f"falhou a faixa {inicio}-{fim}")


class ZipRemoto(io.RawIOBase):
    def __init__(self, url):
        self.url, self.pos = url, 0

    def seekable(self):
        return True

    def readable(self):
        return True

    def tell(self):
        return self.pos

    def seek(self, desvio, de_onde=0):
        base = (0, self.pos, TAMANHO_ZIP)[de_onde]
        self.pos = base + desvio
        return self.pos

    def readinto(self, b):
        fim = min(TAMANHO_ZIP, self.pos + len(b)) - 1
        if fim < self.pos:
            return 0
        dados, self.url = faixa(self.url, self.pos, fim)
        b[:len(dados)] = dados
        self.pos += len(dados)
        return len(dados)


def indice(url, cache):
    # ler o diretorio do zip leva alguns minutos, entao ele fica guardado em cache
    if cache.exists():
        return json.loads(cache.read_text())
    z = zipfile.ZipFile(io.BufferedReader(ZipRemoto(url), buffer_size=1 << 20))
    tabela = {i.filename[len(PASTA_NO_ZIP):]: [i.header_offset, i.compress_size, i.compress_type]
              for i in z.infolist() if i.filename.startswith(PASTA_NO_ZIP) and i.filename.endswith(".jpg")}
    cache.write_text(json.dumps(tabela))
    return tabela


def baixar(url, entrada):
    inicio, tamanho, tipo = entrada
    # cabecalho local: 30 bytes fixos, depois nome e extra com tamanhos proprios
    cab, url = faixa(url, inicio, inicio + 29)
    n_nome, n_extra = struct.unpack("<HH", cab[26:30])
    dados_ini = inicio + 30 + n_nome + n_extra
    dados, _ = faixa(url, dados_ini, dados_ini + tamanho - 1)
    if tipo == zipfile.ZIP_DEFLATED:
        return zlib.decompress(dados, -15)
    if tipo != zipfile.ZIP_STORED:
        raise RuntimeError(f"compressao {tipo} nao suportada")
    return dados


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--entrada", required=True)
    parser.add_argument("--ids", required=True)
    parser.add_argument("--saida", default="dados/imagens")
    parser.add_argument("--cache", default="dados/indice_zip.json")
    args = parser.parse_args()

    pecas = carregar_pecas(args.entrada)
    with open(Path(args.saida).parent / "imagens.csv", "w", newline="", encoding="utf-8") as arq:
        esc = csv.writer(arq, lineterminator="\n")
        esc.writerow(["id", "imagem"])
        for i, p in enumerate(pecas):
            esc.writerow([i, p["imagem"]])

    with open(args.ids, encoding="utf-8") as arq:
        ids = sorted({int(linha["id"]) for linha in csv.DictReader(arq)})
    saida = Path(args.saida)
    saida.mkdir(parents=True, exist_ok=True)
    faltam = [i for i in ids if not (saida / f"{i}.jpg").exists()]
    print(f"{len(ids)} ids, {len(ids) - len(faltam)} ja baixadas.")
    if not faltam:
        return

    url = endereco_final()
    tabela = indice(url, Path(args.cache))

    def uma(i):
        dados = baixar(url, tabela[pecas[i]["imagem"]])
        (saida / f"{i}.jpg").write_bytes(dados)

    with ThreadPoolExecutor(PEDIDOS_JUNTOS) as fila:
        list(fila.map(uma, faltam))
    print(f"{len(faltam)} imagens gravadas em {saida}.")


if __name__ == "__main__":
    main()
