#!/usr/bin/env python3
# Monta dados/demo: um pedaco pequeno da base real para o make demo, sem baixar nada.
# Pega os looks das 60 pecas mais usadas, com todas as pecas desses looks, e numera de novo a partir de 0.
# Com looks sorteados ao acaso quase nenhuma peca se repetiria e o grafo ficaria vazio.
# Uso: python3 preprocess/demo.py   (le dados/*.csv da base completa)

import csv
from collections import Counter, defaultdict
from pathlib import Path

N_POPULARES = 60
ARQUIVOS = ["pecas.csv", "pecas_v2.csv", "pecas_v4.csv", "pecas_v5.csv"]


def ler(caminho):
    with open(caminho, encoding="utf-8") as arq:
        leitor = csv.reader(arq)
        cab = next(leitor)
        return cab, list(leitor)


def main():
    dados, saida = Path("dados"), Path("dados/demo")
    saida.mkdir(exist_ok=True)
    cab_l, linhas = ler(dados / "looks.csv")
    looks = defaultdict(list)
    conjunto = {}
    for look, peca, conj in linhas:
        looks[int(look)].append(int(peca))
        conjunto[int(look)] = conj
    uso = Counter(p for ps in looks.values() for p in ps)
    top = {p for p, _ in uso.most_common(N_POPULARES)}
    escolhidos = sorted(l for l, ps in looks.items() if top & set(ps))
    pecas = sorted({p for l in escolhidos for p in looks[l]})
    novo = {p: i for i, p in enumerate(pecas)}
    novo_look = {l: i for i, l in enumerate(escolhidos)}

    with open(saida / "looks.csv", "w", newline="", encoding="utf-8") as arq:
        w = csv.writer(arq, lineterminator="\n")
        w.writerow(cab_l)
        for l in escolhidos:
            for p in sorted(looks[l]):
                w.writerow([novo_look[l], novo[p], conjunto[l]])
    for nome in ARQUIVOS:
        cab, linhas = ler(dados / nome)
        with open(saida / nome, "w", newline="", encoding="utf-8") as arq:
            w = csv.writer(arq, lineterminator="\n")
            w.writerow(cab)
            for linha in linhas:
                i = int(linha[0])
                if i in novo:
                    w.writerow([novo[i]] + linha[1:])
    with open(saida / "ids_originais.csv", "w", newline="", encoding="utf-8") as arq:
        w = csv.writer(arq, lineterminator="\n")
        w.writerow(["id_demo", "id"])
        w.writerows((novo[p], p) for p in pecas)
    print(f"{len(escolhidos)} looks e {len(pecas)} pecas em {saida}")


if __name__ == "__main__":
    main()
