#!/usr/bin/env python3
# Tira atributos do texto das pecas e grava pecas_v2.csv. Nao muda o pecas.csv antigo.
# Uso: python3 preprocess/atributos.py --entrada Combined_Metadata.json --saida dados

import argparse
import csv
import random
import re
from collections import Counter
from pathlib import Path

import numpy as np

import palavras_chave as pc
from preprocessar import TAMANHOS_SUBCONJUNTOS, calcular_formalidade, carregar_pecas, normalizar_lab, rgb_para_lab

COLUNAS = (["id", "categoria"]
           + [f"estilo_{v}" for v in pc.ESTILO]
           + [f"ocasiao_{v}" for v in pc.OCASIAO]
           + ["formalidade", "L", "a", "b", "cor_neutra", "estampa", "comprimento", "modelagem"])


def juntar(palavras):
    # maiores primeiro senao "polka" ganharia de "polka dot"
    return "|".join(re.escape(p) for p in sorted(palavras, key=len, reverse=True))


def achar(lista):
    return re.compile(r"(?<![a-z])(?:" + juntar(lista) + r")(?![a-z])")


ACHA_ESTILO = {v: achar(l) for v, l in pc.ESTILO.items()}
ACHA_OCASIAO = {v: achar(l) for v, l in pc.OCASIAO.items()}
ACHA_ESTAMPA = {v: achar(l) for v, l in pc.ESTAMPA.items()}
ACHA_COMPRIMENTO = {v: achar(l) for v, l in pc.COMPRIMENTO.items()}
ACHA_MODELAGEM = {v: achar(l) for v, l in pc.MODELAGEM.items()}

# "pair of" e a peca em si. "pairs well with" ja fala de outra, ate o fim da frase
CORTE_PAR = re.compile(r"\b(?:pairs?(?! of)|paired|pairing|complements?|team(?:ed)? it|style it|wear (?:it|them) with)\b[^.]*")
CORTE_NOTA = re.compile(r"\(note.*", re.S)
# manga comprida nao e peca comprida
MANGA = re.compile(r"\b(?:short|long)[- ](?:sleeves?|sleeved)\b|\b(?:short|long) hair\b")

ADJETIVOS = set()
for grupo in (pc.ESTILO, pc.ESTAMPA, pc.COMPRIMENTO, pc.MODELAGEM):
    for lista in grupo.values():
        ADJETIVOS.update(lista)
ADJETIVOS.update(["tailored", "elegant", "formal", "distressed", "silk"])

CITACOES = {}


def citacao(categoria):
    if categoria not in CITACOES:
        nomes = [n for c, l in pc.NOMES_PECA.items() if c != categoria for n in l]
        # "dress watch" e "dress shoes" nao citam um vestido
        CITACOES[categoria] = re.compile(
            r"(?:(?<![a-z])(?:" + juntar(ADJETIVOS) + r")[ ,]+(?:[a-z-]+ )?)?"
            r"(?<![a-z-])(?:" + juntar(nomes) + r")(?![a-z-])(?! (?:watch|shoes|shirt|pants))")
    return CITACOES[categoria]


def limpar(texto, categoria):
    t = CORTE_NOTA.sub(" ", texto.lower())
    t = CORTE_PAR.sub(" ", t)
    t = citacao(categoria).sub(" ", t)
    return MANGA.sub(" ", t)


def primeiro(achadores, texto):
    for valor, regex in achadores.items():
        if regex.search(texto):
            return valor
    return None


def atributo_visual(achadores, legenda, descricao):
    # a legenda do BLIP descreve a foto. a descricao refinada so entra se a legenda nao disser nada
    return primeiro(achadores, legenda) or primeiro(achadores, descricao) or "desconhecido"


def extrair(peca):
    cat = peca["categoria"]
    legenda = limpar(peca["legenda"], cat)
    desc = limpar(peca["descricao"], cat)
    tem_corte = cat in pc.CATEGORIAS_COM_CORTE

    L, a, b = normalizar_lab(rgb_para_lab(np.array(peca["rgb"][0], dtype=float)))
    linha = {"categoria": cat}
    for v, r in ACHA_ESTILO.items():
        linha[f"estilo_{v}"] = int(bool(r.search(desc)))
    for v, r in ACHA_OCASIAO.items():
        linha[f"ocasiao_{v}"] = int(bool(r.search(desc)))
    linha["formalidade"] = calcular_formalidade(desc)
    linha.update(L=L, a=a, b=b)
    linha["cor_neutra"] = int(peca["cor_nome"] in pc.CORES_NEUTRAS)
    linha["estampa"] = atributo_visual(ACHA_ESTAMPA, legenda, desc)
    linha["comprimento"] = atributo_visual(ACHA_COMPRIMENTO, legenda, desc) if tem_corte else "desconhecido"
    linha["modelagem"] = atributo_visual(ACHA_MODELAGEM, legenda, desc) if tem_corte else "desconhecido"
    return linha


def gravar(linhas, ids, caminho):
    with open(caminho, "w", newline="", encoding="utf-8") as arq:
        esc = csv.writer(arq, lineterminator="\n")
        esc.writerow(COLUNAS)
        for i in ids:
            l = linhas[i]
            esc.writerow([i, l["categoria"]] + [
                f"{l[c]:.6f}" if isinstance(l[c], float) else l[c] for c in COLUNAS[2:]])


def ids_do_csv(caminho):
    with open(caminho, encoding="utf-8") as arq:
        return [int(l["id"]) for l in csv.DictReader(arq)]


def pct(n, total):
    return f"{100 * n / total:.1f}%"


def relatorio(pecas, linhas, caminho):
    n = len(linhas)
    com_corte = [l for l in linhas if l["categoria"] in pc.CATEGORIAS_COM_CORTE]
    estilos = [f"estilo_{v}" for v in pc.ESTILO]
    ocasioes = [f"ocasiao_{v}" for v in pc.OCASIAO]

    tab = [
        ("estilo (todas as colunas em 0)", sum(not any(l[c] for c in estilos) for l in linhas), n),
        ("ocasiao (todas as colunas em 0)", sum(not any(l[c] for c in ocasioes) for l in linhas), n),
        ("formalidade (sem palavra, fica 0.5)", sum(l["formalidade"] == 0.5 for l in linhas), n),
        ("estampa", sum(l["estampa"] == "desconhecido" for l in linhas), n),
        ("comprimento (so roupas)", sum(l["comprimento"] == "desconhecido" for l in com_corte), len(com_corte)),
        ("modelagem (so roupas)", sum(l["modelagem"] == "desconhecido" for l in com_corte), len(com_corte)),
    ]

    with open(caminho, "w", encoding="utf-8") as s:
        s.write("# Amostra dos atributos\n\nConferir a mao se o atributo bate com a descricao.\n\n")
        s.write("## Desconhecido por atributo\n\n| Atributo | Desconhecido |\n| --- | --- |\n")
        for nome, q, t in tab:
            s.write(f"| {nome} | {pct(q, t)} |\n")
        s.write("\n## Distribuicao\n\n")
        for c in ["estampa", "comprimento", "modelagem"]:
            s.write(f"- {c}: {dict(Counter(l[c] for l in linhas).most_common())}\n")
        for c in estilos + ocasioes + ["cor_neutra"]:
            s.write(f"- {c}: {pct(sum(l[c] for l in linhas), n)}\n")

        s.write("\n## 50 pecas sorteadas\n\n")
        for i in random.Random(42).sample(range(n), 50):
            l = linhas[i]
            est = [c[7:] for c in estilos if l[c]]
            oca = [c[8:] for c in ocasioes if l[c]]
            s.write(f"### Peca {i} ({l['categoria']}, cor {pecas[i]['cor_nome']})\n\n")
            s.write(f"- Legenda: {pecas[i]['legenda']}\n")
            s.write(f"- Descricao: {pecas[i]['descricao'].strip()}\n")
            s.write(f"- estilo {est}, ocasiao {oca}, formalidade {l['formalidade']:.2f}, "
                    f"estampa {l['estampa']}, comprimento {l['comprimento']}, modelagem {l['modelagem']}\n\n")
    return tab


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--entrada", required=True)
    parser.add_argument("--saida", default="dados")
    parser.add_argument("--amostra", default="resultados/amostra_atributos.md")
    args = parser.parse_args()

    pasta = Path(args.saida)
    pecas = carregar_pecas(args.entrada)
    linhas = [extrair(p) for p in pecas]
    gravar(linhas, range(len(linhas)), pasta / "pecas_v2.csv")
    for tam in TAMANHOS_SUBCONJUNTOS:
        antigo = pasta / f"pecas_{tam}.csv"
        if antigo.exists():
            gravar(linhas, ids_do_csv(antigo), pasta / f"pecas_v2_{tam}.csv")
    for nome, q, t in relatorio(pecas, linhas, args.amostra):
        print(f"{nome}: {pct(q, t)}")


if __name__ == "__main__":
    main()
