#!/usr/bin/env python3
# Monta um pecas_v2.csv com a fonte escolhida para cada atributo (ver resultados/comparacao_fontes.md).
# O formato e o mesmo que a versao D le (src/atributos.cpp), entao o C++ nao muda.
# Uso: python3 preprocess/atributos_finais.py --texto <pasta com pecas_v2.csv> --clip dados/atributos_clip.csv
#        --ids lista.csv --saida <pasta>   [--fonte estilo=texto ...]

import argparse
import csv
from collections import Counter
from pathlib import Path

from aplicacao import aplica
from atributos import COLUNAS
from referencia import NA

# fora = valor neutro que a D ja trata. so a estampa passou dos 70% no teste
FONTE = {
    "estampa": "lisa",
    "comprimento": "fora",
    "modelagem": "fora",
    "estilo": "fora",
    "ocasiao": "fora",
    "formalidade": "fora",
}
OPCOES = {"texto", "clip", "comb", "lisa", "fora"}
DESC = "desconhecido"

# valores do CLIP no vocabulario do texto, que e o que a D le
DO_CLIP = {"poá": "poa", "animal print": "animal", "outra estampa": "outra", "médio": "medio",
           "clássico": "classico", "romântico": "romantico", "dia a dia": "dia"}
FORMAL = {"baixa": 0.0, "média": 0.5, "alta": 1.0}
ESTILOS = [c[7:] for c in COLUNAS if c.startswith("estilo_")]
OCASIOES = [c[8:] for c in COLUNAS if c.startswith("ocasiao_")]


def ler_csv(caminho, quero=None):
    with open(caminho, encoding="utf-8-sig", newline="") as arq:
        return {int(l["id"]): l for l in csv.DictReader(arq) if quero is None or int(l["id"]) in quero}


def valor_clip(atr, linha):
    if linha is None:
        return None
    v = linha.get(atr, "")
    if v in ("", NA, "incerto"):
        return None
    return DO_CLIP.get(v, v)


def texto_achou(atr, t):
    if atr == "estilo":
        return any(t[f"estilo_{v}"] == "1" for v in ESTILOS)
    if atr == "ocasiao":
        return any(t[f"ocasiao_{v}"] == "1" for v in OCASIOES)
    if atr == "formalidade":
        # sem o JSON nao da para separar empate de falta de palavra. 0,5 fica como esta nos dois casos
        return float(t["formalidade"]) != 0.5
    return t[atr] != DESC


def neutro(atr, saida):
    if atr == "estilo":
        saida.update({f"estilo_{v}": "0" for v in ESTILOS})
    elif atr == "ocasiao":
        saida.update({f"ocasiao_{v}": "0" for v in OCASIOES})
    elif atr == "formalidade":
        saida["formalidade"] = "0.500000"
    else:
        saida[atr] = DESC


def copiar_texto(atr, t, saida):
    if atr == "estilo":
        saida.update({f"estilo_{v}": t[f"estilo_{v}"] for v in ESTILOS})
    elif atr == "ocasiao":
        saida.update({f"ocasiao_{v}": t[f"ocasiao_{v}"] for v in OCASIOES})
    else:
        saida[atr] = t[atr]


def por_clip(atr, v, saida):
    if atr == "estilo":
        saida.update({f"estilo_{x}": str(int(x == v)) for x in ESTILOS})
    elif atr == "ocasiao":
        saida.update({f"ocasiao_{x}": str(int(x == v)) for x in OCASIOES})
    elif atr == "formalidade":
        saida["formalidade"] = f"{FORMAL[v]:.6f}"
    else:
        saida[atr] = v


def montar(t, c, fontes, conta):
    cat = t["categoria"]
    saida = {k: t[k] for k in ("id", "categoria", "L", "a", "b", "cor_neutra")}
    for atr, fonte in fontes.items():
        # onde o atributo nao se aplica (estampa de joia, comprimento de blusa) fica o neutro
        if fonte == "fora" or not aplica(atr, cat):
            neutro(atr, saida)
            conta[atr]["neutro"] += 1
            continue
        achou = texto_achou(atr, t)
        vc = valor_clip(atr, c)
        if fonte == "texto" or (fonte == "comb" and achou):
            copiar_texto(atr, t, saida)
            conta[atr]["texto"] += 1
        elif fonte == "lisa":
            saida[atr] = t[atr] if achou else "lisa"
            conta[atr]["texto" if achou else "lisa"] += 1
        elif vc is not None:
            por_clip(atr, vc, saida)
            conta[atr]["clip"] += 1
        else:
            # sem linha do CLIP, ou CLIP sem valor (formalidade de joia): fica o texto
            copiar_texto(atr, t, saida)
            conta[atr]["texto sem clip" if c is None else "texto, clip sem valor"] += 1
    return saida


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--texto", required=True)
    parser.add_argument("--clip", default="dados/atributos_clip.csv")
    parser.add_argument("--ids", required=True)
    parser.add_argument("--saida", required=True)
    parser.add_argument("--fonte", action="append", default=[], help="atributo=fonte, troca a escolha padrao")
    args = parser.parse_args()

    fontes = dict(FONTE)
    for item in args.fonte:
        atr, _, f = item.partition("=")
        if atr not in fontes or f not in OPCOES or (f == "lisa" and atr != "estampa"):
            raise SystemExit(f"--fonte {item}: use atributo=fonte com fonte em {sorted(OPCOES)} (lisa só na estampa)")
        fontes[atr] = f

    ids = ler_csv(args.ids)
    if not ids:
        raise SystemExit(f"{args.ids} sem ids.")
    quero = set(ids)
    texto = ler_csv(Path(args.texto) / "pecas_v2.csv", quero)
    clip = ler_csv(args.clip, quero) if Path(args.clip).exists() else {}
    if not clip and any(f in ("clip", "comb") for f in fontes.values()):
        print(f"{args.clip} vazio ou ausente. Toda fonte de CLIP cai no texto.")

    faltam = sorted(quero - set(texto))
    if faltam:
        print(f"{len(faltam)} ids sem linha no texto, ficam fora do arquivo (a D vai reclamar se estiverem na base): "
              f"{faltam[:10]}")
    for i in sorted(set(texto) & set(clip)):
        if texto[i]["categoria"] != clip[i]["categoria"]:
            raise SystemExit(f"id {i}: categoria {texto[i]['categoria']} no texto e {clip[i]['categoria']} no CLIP")

    conta = {atr: Counter() for atr in fontes}
    sem_clip = 0
    pasta = Path(args.saida)
    pasta.mkdir(parents=True, exist_ok=True)
    with open(pasta / "pecas_v2.csv", "w", newline="", encoding="utf-8") as arq:
        esc = csv.writer(arq, lineterminator="\n")
        esc.writerow(COLUNAS)
        for i in sorted(texto):
            c = clip.get(i)
            sem_clip += c is None
            linha = montar(texto[i], c, fontes, conta)
            esc.writerow([linha[k] for k in COLUNAS])

    print(f"{len(texto)} peças em {pasta / 'pecas_v2.csv'}. Sem linha do CLIP: {sem_clip}.")
    for atr, f in fontes.items():
        print(f"{atr} ({f}): {dict(conta[atr].most_common())}")


if __name__ == "__main__":
    main()
