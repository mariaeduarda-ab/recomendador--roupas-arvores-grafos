#!/usr/bin/env python3
# Junta pecas gemeas: mesma categoria, mesma legenda do BLIP e as 3 cores arredondadas.
# O preprocessar ja junta as que tem as cores exatas. Aqui a cor pode variar um pouco
# (mesma peca recortada de outro jeito em outro look). O JSON nao tem codigo de produto.
# Uso: python3 preprocess/gemeas.py --entrada Combined_Metadata.json [--passo 16] [--sem-legenda --nome gemeas_p8]
# So mede e grava dados/<nome>.csv (id,classe) e os pares para conferir. Nao muda os outros CSVs.

import argparse
import csv
import random
from collections import Counter, defaultdict
from pathlib import Path

from preprocessar import carregar_pecas

SEMENTE_PARES = 7
N_PARES = 50


class UnionFind:
    def __init__(self, n):
        self.pai = list(range(n))
        self.tam = [1] * n

    def achar(self, x):
        while self.pai[x] != x:
            self.pai[x] = self.pai[self.pai[x]]
            x = self.pai[x]
        return x

    def unir(self, x, y):
        x, y = self.achar(x), self.achar(y)
        if x == y:
            return
        if self.tam[x] < self.tam[y]:
            x, y = y, x
        self.pai[y] = x
        self.tam[x] += self.tam[y]


def digital(peca, passo, legenda=True):
    # cada canal vira a faixa dele: com passo 16, 0..15 vira 0 e 16..31 vira 1
    cores = tuple(tuple(c // passo for c in rgb) for rgb in peca["rgb"])
    return (peca["categoria"], cores, peca["legenda"] if legenda else "")


def ler_looks(caminho):
    looks = defaultdict(list)
    conjunto = {}
    with open(caminho, encoding="utf-8") as arq:
        for l in csv.DictReader(arq):
            looks[int(l["look_id"])].append(int(l["peca_id"]))
            conjunto[int(l["look_id"])] = l["conjunto"]
    return looks, conjunto


def cobertura(consultas, classe, looks, conjunto, fora=frozenset()):
    # a peca "aparece no treino" se alguma peca da classe dela esta num look de treino
    no_treino = set()
    for look, pecas in looks.items():
        if conjunto[look] == "treino" and look not in fora:
            no_treino.update(classe[p] for p in pecas)
    return sum(classe[p] in no_treino for _, p in consultas)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--entrada", required=True)
    parser.add_argument("--dados", default="dados")
    parser.add_argument("--passo", type=int, default=16)
    parser.add_argument("--resultados", default="resultados")
    parser.add_argument("--sem-legenda", action="store_true", help="junta so por categoria e cores")
    parser.add_argument("--nome", default="gemeas", help="prefixo dos arquivos gravados")
    args = parser.parse_args()

    pecas = carregar_pecas(args.entrada)
    n = len(pecas)
    with open(Path(args.dados) / "pecas.csv", encoding="utf-8") as arq:
        cats = [l["categoria"] for l in csv.DictReader(arq)]
    if len(cats) != n or any(cats[i] != pecas[i]["categoria"] for i in range(n)):
        raise SystemExit("A ordem das pecas nao bate com dados/pecas.csv.")

    uf = UnionFind(n)
    primeira = {}
    for i, p in enumerate(pecas):
        d = digital(p, args.passo, not args.sem_legenda)
        if d in primeira:
            uf.unir(primeira[d], i)
        else:
            primeira[d] = i
    classe = [uf.achar(i) for i in range(n)]
    tam = Counter(classe)
    grandes = [c for c, t in tam.items() if t > 1]

    # so treino e validacao: o teste nao entra em nenhuma conta que orienta escolha
    looks, conjunto = ler_looks(Path(args.dados) / "looks.csv")
    with open(Path(args.dados) / "looks_validacao.csv", encoding="utf-8") as arq:
        validacao = frozenset(int(l["look_id"]) for l in csv.DictReader(arq))
    # toda peca de look de validacao conta, como se cada uma fosse a escondida
    pecas_val = [(l, p) for l in sorted(validacao) for p in looks[l]]
    proprio = list(range(n))
    antes = cobertura(pecas_val, proprio, looks, conjunto, fora=validacao)
    depois = cobertura(pecas_val, classe, looks, conjunto, fora=validacao)

    with open(Path(args.dados) / f"{args.nome}.csv", "w", newline="", encoding="utf-8") as arq:
        esc_csv = csv.writer(arq, lineterminator="\n")
        esc_csv.writerow(["id", "classe"])
        for i in range(n):
            esc_csv.writerow([i, classe[i]])

    # 50 pares para conferir pela imagem: duas pecas de classes diferentes sorteadas
    rnd = random.Random(SEMENTE_PARES)
    membros = defaultdict(list)
    for i, c in enumerate(classe):
        membros[c].append(i)
    sorteio = rnd.sample(sorted(grandes), min(N_PARES, len(grandes)))
    pares = [tuple(rnd.sample(membros[c], 2)) for c in sorteio]
    with open(Path(args.resultados) / f"{args.nome}_pares.csv", "w", newline="", encoding="utf-8") as arq:
        w = csv.writer(arq, lineterminator="\n")
        w.writerow(["par", "id1", "id2", "categoria", "legenda", "rgb1", "rgb2", "mesma_peca"])
        for k, (a, b) in enumerate(pares):
            w.writerow([k, a, b, pecas[a]["categoria"], pecas[a]["legenda"], pecas[a]["rgb"], pecas[b]["rgb"], ""])
    with open(Path(args.resultados) / f"{args.nome}_ids.csv", "w", newline="", encoding="utf-8") as arq:
        w = csv.writer(arq, lineterminator="\n")
        w.writerow(["id"])
        for a, b in pares:
            w.writerow([a])
            w.writerow([b])

    juntadas = sum(t for t in tam.values() if t > 1)
    print(f"passo {args.passo}: {n} pecas, {len(tam)} classes, {len(grandes)} com mais de 1 peca "
          f"({juntadas} pecas), maior classe {max(tam.values())}")
    print(f"pecas dos looks de validacao com peca da classe no treino (sem a validacao): antes {antes} de "
          f"{len(pecas_val)} ({100 * antes / len(pecas_val):.1f}%), depois {depois} ({100 * depois / len(pecas_val):.1f}%)")


if __name__ == "__main__":
    main()
