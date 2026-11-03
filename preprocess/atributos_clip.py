#!/usr/bin/env python3
# Tira os atributos das imagens com o CLIP ViT-B/32, sem treino (zero-shot).
# Para cada atributo, compara a imagem com uma frase por valor e fica com a mais parecida.
# A confianca e a probabilidade do vencedor no softmax.
# Uso: python3 preprocess/atributos_clip.py --imagens dados/imagens --pecas dados/pecas.csv --ids lista.csv
#   Precisa do requirements_clip.txt. O C++ nao depende disto, so le o CSV gerado.

import argparse
import csv
import os
import time
from collections import Counter
from pathlib import Path

import open_clip
import torch
from PIL import Image

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
    "estilo": {
        "casual": ["casual {}", "relaxed everyday {}"],
        "clássico": ["classic elegant {}", "timeless {}"],
        "esportivo": ["sporty {}", "athletic {}"],
        "romântico": ["romantic feminine {}", "delicate {} with lace or ruffles"],
        "moderno": ["modern trendy {}", "edgy fashion forward {}"],
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
    ("formalidade", "watch"): {
        "baixa": ["sporty digital watch"],
        "média": ["casual leather strap watch"],
        "alta": ["elegant dress watch"],
    },
}

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


def conferir():
    for atr, tabela in FRASES.items():
        certos = [v for v in VALORES[atr] if v not in (NA, "incerto")]
        assert list(tabela) == certos, f"{atr}: {list(tabela)} != {certos}"
    for (atr, _), tabela in TROCAS.items():
        assert list(tabela) == list(FRASES[atr]), f"troca de {atr} com valores errados"


def textos(modelo, tokenizer, categorias, aparelho):
    # um vetor por valor: media das frases e dos modelos de frase, normalizada
    saida = {}
    with torch.no_grad():
        for cat in categorias:
            for atr in FRASES:
                if not aplica(atr, cat):
                    continue
                tabela = frases(atr, cat)
                vetores = []
                for lista in tabela.values():
                    todas = [m.format(f) for f in lista for m in MODELOS_FRASE]
                    t = modelo.encode_text(tokenizer(todas).to(aparelho))
                    t = t / t.norm(dim=-1, keepdim=True)
                    t = t.mean(dim=0)
                    vetores.append(t / t.norm())
                saida[(cat, atr)] = (list(tabela), torch.stack(vetores).cpu())
    return saida


class Imagens(torch.utils.data.Dataset):
    def __init__(self, ids, pasta, preparo):
        self.ids, self.pasta, self.preparo = ids, pasta, preparo

    def __len__(self):
        return len(self.ids)

    def __getitem__(self, k):
        try:
            with Image.open(self.pasta / f"{self.ids[k]}.jpg") as img:
                return self.preparo(img.convert("RGB")), True
        except (OSError, ValueError):
            # o lote precisa de um tensor do mesmo tamanho, entao vai um vazio marcado como falho
            return torch.zeros(3, 224, 224), False


def ler_categorias(caminho):
    with open(caminho, encoding="utf-8") as arq:
        return {int(l["id"]): l["categoria"] for l in csv.DictReader(arq)}


def ler_ids(args):
    if args.ids:
        with open(args.ids, encoding="utf-8") as arq:
            return sorted({int(l["id"]) for l in csv.DictReader(arq)})
    return sorted(int(p.stem) for p in Path(args.imagens).glob("*.jpg") if p.stem.isdigit())


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--imagens", required=True)
    parser.add_argument("--pecas", default="dados/pecas.csv")
    parser.add_argument("--ids")
    parser.add_argument("--saida", default="dados/atributos_clip.csv")
    parser.add_argument("--lote", type=int, default=64)
    parser.add_argument("--leitores", type=int, default=2)
    args = parser.parse_args()

    conferir()
    aparelho = "cuda" if torch.cuda.is_available() else "cpu"
    if aparelho == "cpu":
        torch.set_num_threads(os.cpu_count() or 1)

    categorias = ler_categorias(args.pecas)
    pasta = Path(args.imagens)
    ids = []
    for i in ler_ids(args):
        if i not in categorias:
            print(f"id {i} nao esta em {args.pecas}, ignorado.")
        elif not (pasta / f"{i}.jpg").exists():
            print(f"id {i} sem imagem em {pasta}, ignorado.")
        else:
            ids.append(i)

    modelo, _, preparo = open_clip.create_model_and_transforms(MODELO, pretrained=PESOS, device=aparelho)
    modelo.eval()
    tokenizer = open_clip.get_tokenizer(MODELO)
    escala = modelo.logit_scale.exp().item()
    vetores = textos(modelo, tokenizer, sorted({categorias[i] for i in ids}), aparelho)

    leitor = torch.utils.data.DataLoader(Imagens(ids, pasta, preparo), batch_size=args.lote,
                                         num_workers=args.leitores)
    Path(args.saida).parent.mkdir(parents=True, exist_ok=True)
    falhas = 0
    contagem = {atr: Counter() for atr in FRASES}
    inicio = time.time()
    feitas = 0
    with open(args.saida, "w", newline="", encoding="utf-8") as arq, torch.no_grad():
        esc = csv.writer(arq, lineterminator="\n")
        esc.writerow(COLUNAS)
        for fotos, ok in leitor:
            img = modelo.encode_image(fotos.to(aparelho))
            img = (img / img.norm(dim=-1, keepdim=True)).cpu()
            for k in range(len(ok)):
                i = ids[feitas + k]
                if not ok[k]:
                    print(f"id {i}: imagem ilegivel, ignorado.")
                    falhas += 1
                    continue
                cat = categorias[i]
                linha = [i, cat]
                for atr in FRASES:
                    if (cat, atr) not in vetores:
                        linha += [NA, ""]
                        contagem[atr][NA] += 1
                        continue
                    valores, txt = vetores[(cat, atr)]
                    prob = (escala * img[k] @ txt.T).softmax(dim=-1)
                    melhor = int(prob.argmax())
                    linha += [valores[melhor], f"{prob[melhor]:.3f}"]
                    contagem[atr][valores[melhor]] += 1
                esc.writerow(linha)
            feitas += len(ok)
            gasto = time.time() - inicio
            print(f"{feitas}/{len(ids)} imagens, {feitas / gasto:.1f} por segundo", end="\r", flush=True)

    gasto = time.time() - inicio
    print(f"\n{feitas - falhas} pecas em {args.saida}. {falhas} ilegiveis. "
          f"{gasto:.0f} s, {feitas / max(gasto, 1e-9):.1f} imagens por segundo.")
    for atr, c in contagem.items():
        print(f"{atr}: {dict(c.most_common())}")


if __name__ == "__main__":
    main()
