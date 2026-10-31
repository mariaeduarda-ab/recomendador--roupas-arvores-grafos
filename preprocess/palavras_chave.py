# Listas montadas a partir de uma amostra de 200 descricoes e da contagem na base inteira.
# Para mudar um atributo, basta mexer aqui e rodar atributos.py de novo.
# A ordem importa em estampa e modelagem: vale o primeiro valor com palavra encontrada.

ESTILO = {
    "casual": ["casual", "relaxed", "laid-back"],
    "classico": ["classic", "timeless", "traditional"],
    "esportivo": ["sporty", "athletic", "sportswear"],
    "romantico": ["romantic", "feminine", "whimsical"],
    "moderno": ["modern", "contemporary", "edgy", "minimalist"],
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
    "floral": ["floral", "flower print", "flower pattern", "flowered"],
    "poa": ["polka", "polka dot", "dots", "dot", "dotted"],
    "animal": ["leopard", "zebra", "snake", "snakeskin", "cheetah", "tiger", "animal print"],
    "outra": ["pattern", "patterned", "print", "printed", "graphic", "paisley", "camouflage", "camo", "tie-dye", "tribal", "aztec"],
    "lisa": ["solid", "plain", "solid-colored", "solid-color", "solid color", "monochrome"],
}

COMPRIMENTO = {
    "curto": ["mini", "short", "shorts"],
    "medio": ["midi", "knee-length", "knee length", "capri"],
    "longo": ["maxi", "long", "floor-length", "ankle-length", "full-length"],
}

MODELAGEM = {
    "cropped": ["cropped", "crop"],
    "oversized": ["oversized", "boxy"],
    "justa": ["fitted", "tight", "skinny", "slim", "slim-fit", "bodycon"],
    "ampla": ["loose", "wide", "wide-leg", "flowy", "flared", "baggy", "a-line"],
    "reta": ["straight", "straight-leg"],
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
