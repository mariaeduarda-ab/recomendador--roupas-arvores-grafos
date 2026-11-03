#!/usr/bin/env python3
# Monta o catalogo de avaliacao: todas as pecas dos looks de teste e de validacao.
# As escondidas continuam as mesmas, e as pecas dos outros looks fazem o papel de concorrentes.
# Uso: ./recomendador validacao > dados/looks_validacao.csv
#      python3 preprocess/catalogo.py --dados dados

import argparse
import csv
from pathlib import Path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dados", default="dados")
    args = parser.parse_args()
    pasta = Path(args.dados)

    with open(pasta / "looks_validacao.csv", encoding="utf-8") as arq:
        validacao = {int(l["look_id"]) for l in csv.DictReader(arq)}
    if not validacao:
        raise SystemExit("looks_validacao.csv vazio. Rode ./recomendador validacao antes.")

    escolhidas = set()
    with open(pasta / "looks.csv", encoding="utf-8") as arq:
        for l in csv.DictReader(arq):
            if l["conjunto"] == "teste" or int(l["look_id"]) in validacao:
                escolhidas.add(int(l["peca_id"]))

    with open(pasta / "pecas.csv", encoding="utf-8") as arq:
        linhas = [(l["id"], l["categoria"]) for l in csv.DictReader(arq) if int(l["id"]) in escolhidas]
    with open(pasta / "catalogo.csv", "w", newline="", encoding="utf-8") as arq:
        esc = csv.writer(arq, lineterminator="\n")
        esc.writerow(["id", "categoria"])
        esc.writerows(linhas)
    print(f"{len(linhas)} pecas no catalogo ({len(validacao)} looks de validacao mais os de teste).")


if __name__ == "__main__":
    main()
