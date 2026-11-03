# Listas montadas a partir de uma amostra de 200 descricoes e da contagem na base inteira.
# Para mudar um atributo, basta mexer aqui e rodar atributos.py de novo.
# A ordem importa em estampa e modelagem: vale o primeiro valor com palavra encontrada.

ESTILO = {
    "casual": ["casual", "relaxed", "laid-back"],
    "classico": ["classic", "timeless", "traditional", "preppy", "vintage", "retro"],
    "esportivo": ["sporty", "athletic", "sportswear", "athleisure"],
    "romantico": ["romantic", "feminine", "whimsical", "bohemian", "boho"],
    "moderno": ["modern", "contemporary", "edgy", "minimalist", "streetwear", "urban"],
}

OCASIAO = {
    "dia": ["daily", "everyday", "day-to-day"],
    "trabalho": ["office", "work", "professional", "business"],
    "festa": ["evening", "party", "cocktail", "festive", "wedding", "night out"],
    "esporte": ["gym", "workout", "athletic", "sporty", "running"],
    "praia": ["beach", "vacation", "resort", "poolside"],
}

ESTAMPA = {
    "listrada": ["striped", "stripes", "stripe", "pinstripe", "pinstriped"],
    "xadrez": ["plaid", "checkered", "tartan", "gingham", "houndstooth"],
    # "flower" no singular fica de fora: quase sempre e um enfeite so, no salto ou na bolsa
    "floral": ["floral", "flower print", "flower pattern", "flowered", "flowers", "flowery", "roses"],
    "poa": ["polka", "polka dot", "dots", "dot", "dotted"],
    "animal": ["leopard", "zebra", "snake", "snakeskin", "cheetah", "tiger", "animal print", "python", "giraffe"],
    "outra": ["pattern", "patterned", "print", "printed", "graphic", "paisley", "camouflage", "camo", "tie-dye", "tribal", "aztec",
              "tie dye", "tie-dyed", "geometric", "abstract", "argyle", "chevron", "ombre", "patchwork", "monogram",
              "color block", "color-block", "color-blocked", "colorblock", "two-tone", "motif", "motifs",
              "skull", "skulls", "stars", "hearts", "slogan", "lettering", "letters", "words"],
    "lisa": ["solid", "plain", "solid-colored", "solid-color", "solid color", "monochrome"],
}

COMPRIMENTO = {
    "curto": ["mini", "short", "shorts", "miniskirt", "above the knee", "above-the-knee"],
    "medio": ["midi", "knee-length", "knee length", "capri", "tea-length", "mid-length", "below the knee", "bermuda", "culottes"],
    # calca sem outra pista e comprida. "shorts" e "capri" vem antes e ganham.
    # "gown" ficou de fora: nas fotos, 3 de 4 vestidos chamados de gown eram curtos
    "longo": ["maxi", "long", "floor-length", "ankle-length", "full-length", "floor length", "ankle length",
              "floor-sweeping", "jeans", "trousers", "pants", "sweatpants", "leggings", "chinos"],
}

MODELAGEM = {
    "cropped": ["cropped", "crop"],
    "oversized": ["oversized", "oversize", "boxy", "slouchy"],
    "justa": ["fitted", "tight", "skinny", "slim", "slim-fit", "bodycon", "form-fitting", "figure-hugging",
              "body-hugging", "close-fitting", "pencil", "sheath", "leggings", "bodysuit", "mermaid"],
    "ampla": ["loose", "wide", "wide-leg", "flowy", "flared", "baggy", "a-line", "flowing", "billowy", "relaxed fit",
              "relaxed-fit", "fit-and-flare", "skater", "palazzo", "culottes"],
    "reta": ["straight", "straight-leg", "shift"],
}

# so roupa tem comprimento e modelagem. "long necklace" nao diz nada aqui
CATEGORIAS_COM_CORTE = {"top", "dress", "skirt", "pants", "outerwear", "jumpsuit"}

# nomes que dominant_color usa na base. navy entra no lugar de jeans, que nao existe la
CORES_NEUTRAS = {"black", "white", "grey", "cream", "beige", "tan", "navy"}

# usados para apagar a citacao de outra peca: "flowy maxi skirts" numa blusa nao descreve a blusa
NOMES_PECA = {
    "top": ["top", "tops", "blouse", "blouses", "shirt", "shirts", "t-shirt", "t-shirts", "tee", "tees", "sweater", "sweaters", "tank", "camisole"],
    "dress": ["dress", "dresses", "gown", "gowns", "sundress", "sundresses"],
    "skirt": ["skirt", "skirts"],
    "pants": ["pants", "trousers", "jeans", "shorts", "leggings", "denim", "chinos"],
    "outerwear": ["jacket", "jackets", "coat", "coats", "blazer", "blazers", "suit", "suits", "cardigan", "cardigans"],
    "jumpsuit": ["jumpsuit", "jumpsuits", "romper", "rompers"],
    "shoes": ["shoes", "heels", "sneakers", "boots", "sandals", "loafers", "flats"],
    "bag": ["bag", "bags", "purse", "purses", "clutch", "clutches", "tote", "totes"],
}
