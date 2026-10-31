#!/usr/bin/env python3
# Embaralha as linhas do catalogo e roda ./recomendador reavaliar em cada ordem.
# Mede quanto o resultado depende da ordem. Nao escolhe nem ajusta nada.
# Uso: python3 preprocess/ordem.py <pasta temporaria>

import csv
import random
import subprocess
import sys
from pathlib import Path

SEMENTES = [1, 2, 3, 4, 5]
VERSOES = ["Popularidade", "B", "C", "D"]


def preparar(pasta, semente):
    pasta.mkdir(parents=True, exist_ok=True)
    for nome in ["pecas.csv", "pecas_v2.csv"]:
        linhas = Path("dados", nome).read_text(encoding="utf-8").splitlines(keepends=True)
        corpo = linhas[1:]
        random.Random(semente).shuffle(corpo)
        (pasta / nome).write_text(linhas[0] + "".join(corpo), encoding="utf-8")
    alvo = pasta / "looks.csv"
    if not alvo.exists():
        alvo.symlink_to(Path("dados/looks.csv").resolve())


def main():
    base = Path(sys.argv[1])
    rodadas = {"original": base / "ordem0"}
    for s in SEMENTES:
        rodadas[f"semente {s}"] = base / f"ordem{s}"
        preparar(rodadas[f"semente {s}"], s)

    # a saida vai pra resultados/ordemN. a original tambem usa pasta propria pra nao tocar nas tabelas
    (base / "ordem0").mkdir(parents=True, exist_ok=True)
    for nome in ["pecas.csv", "pecas_v2.csv", "looks.csv"]:
        alvo = base / "ordem0" / nome
        if not alvo.exists():
            alvo.symlink_to(Path("dados", nome).resolve())

    processos = [subprocess.Popen(["./recomendador", "reavaliar", "--dados", str(p)], stdout=subprocess.DEVNULL)
                 for p in rodadas.values()]
    if any(p.wait() != 0 for p in processos):
        sys.exit("Alguma rodada falhou.")

    esperadas = sorted(Path("resultados/escondidas.csv").read_text().splitlines())
    with open("resultados/ordem_d.csv", "w", newline="", encoding="utf-8") as arquivo:
        saida = csv.writer(arquivo, lineterminator="\n")
        saida.writerow(["ordem"] + [f"{v}_acertos10" for v in VERSOES] + ["mesmas_escondidas"])
        for nome, p in rodadas.items():
            pasta = Path("resultados", p.name)
            iguais = sorted((pasta / "escondidas.csv").read_text().splitlines()) == esperadas
            with open(pasta / "reavaliar.csv", encoding="utf-8") as f:
                acertos = {l["versao"]: l["acertos10"] for l in csv.DictReader(f)}
            saida.writerow([nome] + [acertos[v] for v in VERSOES] + [int(iguais)])


if __name__ == "__main__":
    main()
