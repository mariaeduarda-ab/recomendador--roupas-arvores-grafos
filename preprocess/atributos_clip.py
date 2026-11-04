#!/usr/bin/env python3
# Tira os atributos das pecas com o CLIP ViT-B/32, sem treino (zero-shot).
# Le os vetores de imagem ja salvos pelo clip_vetores.py. Aqui so entram as frases.
# Para cada atributo, compara o vetor da peca com as frases de cada valor e fica com a mais parecida.
# Antes da escolha, cada frase perde a sua semelhanca media com fotos sem rotulo (ver VIES_GERAL).
# A confianca e a probabilidade do vencedor no softmax.
# Uso: python3 preprocess/atributos_clip.py --vetores dados/clip_vetores --pecas dados/pecas.csv [--ids lista.csv]
#   As fotos sem rotulo saem de --neutras: a amostra de 500 e a metade de ajuste da referencia.
#   A metade de teste nunca entra, para a avaliacao continuar limpa.
#   Precisa do requirements_clip.txt. O C++ nao depende disto, so le o CSV gerado.

import argparse
import csv
import time
from collections import Counter
from pathlib import Path

import numpy as np
import open_clip
import torch

from aplicacao import aplica
from referencia import NA, VALORES

# os pesos da OpenAI foram treinados com QuickGELU. sem isso o modelo fica errado sem dar erro
MODELO = "ViT-B-32-quickgelu"
PESOS = "openai"

# as fotos da base sao de produto, quase todas em fundo branco
MODELOS_FRASE = ["a photo of {}.", "a product photo of {}.", "{} on a white background."]

NOMES = {
    "top": "top", "outerwear": "jacket", "pants": "pants", "dress": "dress", "skirt": "skirt",
    "jumpsuit": "jumpsuit", "bag": "bag", "shoes": "shoes", "neckwear": "scarf", "hats": "hat",
    "legwear": "tights", "bracelet": "bracelet", "brooch": "brooch", "earrings": "earrings",
    "eyewear": "sunglasses", "gloves": "gloves", "hairwear": "hair accessory", "necklace": "necklace",
    "rings": "ring", "watch": "watch",
}
PLURAL = {"pants", "shoes", "legwear", "earrings", "eyewear", "gloves"}

# {} vira o nome da peca. cada valor pode ter mais de uma frase, e a media delas entra na conta
FRASES = {
    "estampa": {
        "lisa": ["plain {}", "solid color {}"],
        "listrada": ["striped {}"],
        "xadrez": ["plaid {}", "checkered {}"],
        "floral": ["floral print {}"],
        "poá": ["polka dot {}"],
        "animal print": ["animal print {}", "leopard print {}"],
        "outra estampa": ["{} with a graphic print", "patterned {}"],
    },
    "comprimento": {
        "curto": ["short {}", "mini {}"],
        "médio": ["knee length {}", "midi {}"],
        "longo": ["long {}", "floor length {}"],
    },
    "modelagem": {
        "justa": ["tight fitted {}", "slim fit {}"],
        "reta": ["straight cut {}"],
        "ampla": ["loose flowy {}", "wide {}"],
        "cropped": ["cropped {}"],
        "oversized": ["oversized {}", "baggy oversized {}"],
    },
    # "trendy" e "fashion forward" puxavam quase toda peca para moderno
    "estilo": {
        "casual": ["casual {}", "basic everyday {}", "simple casual {}"],
        "clássico": ["classic elegant {}", "timeless {}", "classic tailored {}"],
        "esportivo": ["sporty {}", "athletic {}", "sportswear {}"],
        "romântico": ["romantic feminine {}", "delicate {} with lace or ruffles"],
        "moderno": ["modern {} with a bold design", "contemporary designer {}"],
    },
    "ocasiao": {
        "dia a dia": ["{} for everyday wear"],
        "trabalho": ["{} for the office", "{} for work"],
        "festa": ["{} for a party", "{} for an evening event"],
        "esporte": ["{} for sports", "{} for the gym"],
        "praia": ["{} for the beach"],
    },
    "formalidade": {
        "baixa": ["casual {}"],
        "média": ["smart casual {}"],
        "alta": ["formal {}", "elegant formal {}"],
    },
}

# frases que trocam a generica naquela categoria. o nome sozinho nao separa bem esses casos
TROCAS = {
    ("comprimento", "pants"): {
        "curto": ["shorts"],
        "médio": ["capri pants", "knee length bermuda shorts"],
        "longo": ["full length pants"],
    },
    ("comprimento", "outerwear"): {
        "curto": ["cropped jacket", "short jacket ending at the waist"],
        "médio": ["hip length jacket"],
        "longo": ["long coat", "knee length coat"],
    },
    ("comprimento", "jumpsuit"): {
        "curto": ["short romper", "playsuit with shorts"],
        "médio": ["cropped jumpsuit"],
        "longo": ["full length jumpsuit"],
    },
    ("modelagem", "pants"): {
        "justa": ["skinny pants", "leggings"],
        "reta": ["straight leg pants"],
        "ampla": ["wide leg pants", "palazzo pants"],
        "cropped": ["cropped pants"],
        "oversized": ["baggy oversized pants"],
    },
    ("modelagem", "dress"): {
        "justa": ["bodycon dress", "tight fitted dress"],
        "reta": ["straight shift dress"],
        "ampla": ["flowy a-line dress", "loose flowing dress"],
        "cropped": ["dress with a cropped top"],
        "oversized": ["oversized dress"],
    },
    ("modelagem", "skirt"): {
        "justa": ["pencil skirt", "tight fitted skirt"],
        "reta": ["straight skirt"],
        "ampla": ["flared a-line skirt", "full pleated skirt"],
        "cropped": ["cropped skirt"],
        "oversized": ["oversized skirt"],
    },
    ("modelagem", "top"): {
        "justa": ["tight fitted top"],
        "reta": ["straight cut top"],
        "ampla": ["loose flowy top"],
        "cropped": ["crop top"],
        "oversized": ["oversized top"],
    },
    ("estilo", "shoes"): {
        "casual": ["casual shoes"],
        "clássico": ["classic leather pumps", "classic loafers"],
        "esportivo": ["sneakers", "athletic running shoes"],
        "romântico": ["delicate shoes with bows or lace"],
        "moderno": ["modern statement shoes", "chunky platform shoes"],
    },
    ("formalidade", "shoes"): {
        "baixa": ["casual sneakers", "flip flops"],
        "média": ["loafers", "smart casual shoes"],
        "alta": ["formal dress shoes", "elegant high heels"],
    },
    ("formalidade", "top"): {
        "baixa": ["casual t-shirt", "hoodie"],
        "média": ["smart casual blouse"],
        "alta": ["formal silk blouse", "formal dress shirt"],
    },
    ("formalidade", "outerwear"): {
        "baixa": ["casual denim jacket", "hoodie"],
        "média": ["smart casual jacket"],
        "alta": ["formal blazer", "tailored wool coat"],
    },
    ("formalidade", "pants"): {
        "baixa": ["jeans", "sweatpants"],
        "média": ["chinos", "smart casual pants"],
        "alta": ["tailored dress trousers"],
    },
    ("formalidade", "dress"): {
        "baixa": ["casual sundress", "t-shirt dress"],
        "média": ["smart day dress"],
        "alta": ["formal evening gown", "elegant cocktail dress"],
    },
    ("formalidade", "bag"): {
        "baixa": ["casual backpack", "canvas tote bag"],
        "média": ["leather handbag"],
        "alta": ["formal evening clutch"],
    },
}

# algumas frases combinam com quase toda foto e ganham sempre. por isso cada frase perde a sua
# semelhanca media com fotos sem rotulo. no estilo a media e sobre todas as fotos, porque o estilo
# muda com a categoria. nos outros tres a media e so da mesma categoria, porque a frase fala daquela peca.
# estampa e ocasiao ficam sem correcao: nelas um valor e o normal (lisa, dia a dia) e a correcao o apagava
VIES_GERAL = {"estilo"}
VIES_CATEGORIA = {"comprimento", "modelagem", "formalidade"}

# nessas o CLIP acertou a formalidade em menos da metade das pecas de ajuste, quase o mesmo que
# chutar sempre o valor mais comum. entao a joia e o acessorio ficam sem formalidade
SEM_FORMALIDADE = {"earrings", "necklace", "bracelet", "rings", "brooch", "eyewear", "watch", "hairwear", "gloves"}

COLUNAS = ["id", "categoria"]
for _atr in FRASES:
    COLUNAS += [_atr, f"{_atr}_conf"]


def com_artigo(texto, plural):
    if plural:
        return texto
    return ("an " if texto[0] in "aeiou" else "a ") + texto


def frases(atributo, categoria):
    tabela = TROCAS.get((atributo, categoria), FRASES[atributo])
    nome = NOMES[categoria]
    # a troca ja traz o nome da peca. a generica recebe o nome no {}
    plural = categoria in PLURAL
    return {v: [com_artigo(f.format(nome), plural) for f in lista] for v, lista in tabela.items()}


def usa(atributo, categoria):
    if atributo == "formalidade" and categoria in SEM_FORMALIDADE:
        return False
    return aplica(atributo, categoria)


def conferir():
    for atr, tabela in FRASES.items():
        certos = [v for v in VALORES[atr] if v not in (NA, "incerto")]
        assert list(tabela) == certos, f"{atr}: {list(tabela)} != {certos}"
    for (atr, _), tabela in TROCAS.items():
        assert list(tabela) == list(FRASES[atr]), f"troca de {atr} com valores errados"


def codificar(modelo, tokenizer, todas):
    # o texto do CLIP tem mascara causal: o enchimento depois do fim da frase nao muda o vetor.
    # cortar o enchimento da o mesmo resultado e roda varias vezes mais rapido
    tokens = tokenizer(todas)
    n = int((tokens != 0).sum(dim=1).max())
    with torch.no_grad():
        x = modelo.token_embedding(tokens[:, :n]) + modelo.positional_embedding[:n]
        x = modelo.ln_final(modelo.transformer(x, attn_mask=modelo.attn_mask[:n, :n]))
        x = x[torch.arange(len(tokens)), tokens.argmax(dim=-1)] @ modelo.text_projection
    return dict(zip(todas, x / x.norm(dim=-1, keepdim=True)))


def textos(modelo, tokenizer, categorias):
    # um vetor por valor: media das frases e dos modelos de frase, normalizada
    tabelas = {(cat, atr): frases(atr, cat) for cat in categorias for atr in FRASES if usa(atr, cat)}
    todas = {m.format(f) for t in tabelas.values() for lista in t.values() for f in lista for m in MODELOS_FRASE}
    vet = codificar(modelo, tokenizer, sorted(todas))
    saida = {}
    for chave, tabela in tabelas.items():
        linhas = []
        for lista in tabela.values():
            t = torch.stack([vet[m.format(f)] for f in lista for m in MODELOS_FRASE]).mean(dim=0)
            linhas.append(t / t.norm())
        saida[chave] = (list(tabela), torch.stack(linhas).numpy())
    return saida


def ler_categorias(caminho):
    with open(caminho, encoding="utf-8") as arq:
        return {int(l["id"]): l["categoria"] for l in csv.DictReader(arq)}


def ler_vetores(pasta):
    ids, vetores = [], []
    for arq in sorted(Path(pasta).glob("*.npz")):
        with np.load(arq) as d:
            ids.append(d["ids"].astype(np.int64))
            vetores.append(d["vetores"].astype(np.float32))
    if not ids:
        return np.zeros(0, np.int64), np.zeros((0, 512), np.float32)
    return np.concatenate(ids), np.concatenate(vetores)


def ler_neutras(pasta, amostra, metades, categorias):
    for caminho in (amostra, metades):
        if not Path(caminho).exists():
            print(f"{caminho} nao existe. Tudo fica sem correcao.")
            return np.zeros((0, 512), np.float32), np.array([])
    with open(amostra, encoding="utf-8") as arq:
        quero = {int(l["id"]) for l in csv.DictReader(arq)}
    with open(metades, encoding="utf-8") as arq:
        # peca com categoria errada na base atrapalharia a media da categoria
        quero |= {int(l["id"]) for l in csv.DictReader(arq)
                  if l["metade"] == "ajuste" and l["categoria_errada"] == "nao"}
    ids, img = ler_vetores(pasta)
    _, primeiro = np.unique(ids, return_index=True)
    ids, img = ids[primeiro], img[primeiro]
    ok = np.array([int(i) in quero and int(i) in categorias for i in ids], bool)
    return img[ok], np.array([categorias[int(i)] for i in ids[ok]])


def vieses(vetores, neutras, cats_neutras):
    saida = {}
    if len(neutras) == 0:
        return saida
    for (cat, atr), (_, txt) in vetores.items():
        if atr in VIES_GERAL:
            fotos = neutras
        elif atr in VIES_CATEGORIA:
            fotos = neutras[cats_neutras == cat]
        else:
            continue
        if len(fotos):
            saida[(cat, atr)] = (fotos @ txt.T).mean(axis=0)
        else:
            print(f"sem fotos neutras para {cat}, {atr} fica sem correcao.")
    return saida


def classificar(img, cat, vetores, vies, escala):
    # img: matriz (n, 512) das pecas da mesma categoria. devolve {atr: (valores, indice, prob)}
    saida = {}
    for atr in FRASES:
        if (cat, atr) not in vetores:
            continue
        valores, txt = vetores[(cat, atr)]
        nota = img @ txt.T - vies.get((cat, atr), 0)
        prob = torch.from_numpy(escala * nota).softmax(dim=-1).numpy()
        saida[atr] = (valores, prob.argmax(axis=1), prob.max(axis=1))
    return saida


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--vetores", required=True)
    parser.add_argument("--pecas", default="dados/pecas.csv")
    parser.add_argument("--ids")
    parser.add_argument("--saida", default="dados/atributos_clip.csv")
    parser.add_argument("--neutras", default="dados/clip_vetores_amostras")
    parser.add_argument("--amostra", default="dados/amostra_clip_500.csv")
    parser.add_argument("--metades", default="dados/referencia/metades.csv")
    args = parser.parse_args()

    conferir()
    inicio = time.time()
    categorias = ler_categorias(args.pecas)
    ids, img = ler_vetores(args.vetores)
    if len(ids) == 0:
        raise SystemExit(f"nenhum vetor em {args.vetores}.")
    # o mesmo id pode aparecer em dois lotes. fica a primeira vez
    _, primeiro = np.unique(ids, return_index=True)
    manter = np.zeros(len(ids), bool)
    manter[primeiro] = True
    if args.ids:
        with open(args.ids, encoding="utf-8") as arq:
            pedidos = {int(l["id"]) for l in csv.DictReader(arq)}
        tem = set(ids.tolist())
        for i in sorted(pedidos - tem):
            print(f"id {i} sem vetor em {args.vetores}, ignorado.")
        manter &= np.isin(ids, list(pedidos))
    for k in np.flatnonzero(manter):
        if int(ids[k]) not in categorias:
            print(f"id {ids[k]} nao esta em {args.pecas}, ignorado.")
            manter[k] = False
    ids, img = ids[manter], img[manter]
    ordem = np.argsort(ids)
    ids, img = ids[ordem], img[ordem]
    cats = np.array([categorias[int(i)] for i in ids])

    modelo, _, _ = open_clip.create_model_and_transforms(MODELO, pretrained=PESOS)
    modelo.eval()
    tokenizer = open_clip.get_tokenizer(MODELO)
    escala = modelo.logit_scale.exp().item()
    vetores = textos(modelo, tokenizer, sorted(set(cats.tolist())))
    neutras, cats_neutras = ler_neutras(args.neutras, args.amostra, args.metades, categorias)
    print(f"{len(neutras)} fotos neutras para a correcao.")
    vies = vieses(vetores, neutras, cats_neutras)

    linhas = {}
    contagem = {atr: Counter() for atr in FRASES}
    for cat in sorted(set(cats.tolist())):
        pos = np.flatnonzero(cats == cat)
        res = classificar(img[pos], cat, vetores, vies, escala)
        for j, k in enumerate(pos):
            linha = [int(ids[k]), cat]
            for atr in FRASES:
                if atr not in res:
                    linha += [NA, ""]
                    contagem[atr][NA] += 1
                    continue
                valores, melhor, prob = res[atr]
                v = valores[melhor[j]]
                linha += [v, f"{prob[j]:.3f}"]
                contagem[atr][v] += 1
            linhas[int(ids[k])] = linha

    Path(args.saida).parent.mkdir(parents=True, exist_ok=True)
    with open(args.saida, "w", newline="", encoding="utf-8") as arq:
        esc = csv.writer(arq, lineterminator="\n")
        esc.writerow(COLUNAS)
        for i in sorted(linhas):
            esc.writerow(linhas[i])
    print(f"{len(linhas)} pecas em {args.saida}, {time.time() - inicio:.1f} s.")
    for atr, c in contagem.items():
        print(f"{atr}: {dict(c.most_common())}")


if __name__ == "__main__":
    main()
