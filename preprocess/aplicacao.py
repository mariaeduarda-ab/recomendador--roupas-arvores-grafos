# Em quais categorias cada atributo faz sentido. Fora delas o valor e "nao se aplica".
# estilo, ocasiao e formalidade valem para todas as 20 categorias.

ROUPAS = {"top", "outerwear", "pants", "dress", "skirt", "jumpsuit"}

APLICA = {
    # joia, oculos e relogio quase nunca tem estampa de tecido
    "estampa": ROUPAS | {"bag", "shoes", "neckwear", "hats", "legwear"},
    # top fica fora: blusa curta e modelagem cropped, nao comprimento
    "comprimento": {"skirt", "dress", "pants", "outerwear", "jumpsuit"},
    "modelagem": ROUPAS,
}


def aplica(atributo, categoria):
    return categoria in APLICA.get(atributo, {categoria})
