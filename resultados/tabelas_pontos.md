# Tabelas de pontos por atributo

Gerado por `./recomendador pontos`. As tabelas usam todos os looks de treino (os de validação entram). Os CSVs estão em `tabelas_pontos/`, um por atributo.

## Conta

Para cada atributo e cada par de categorias A e B, com A diferente de B:

- `n(x, y)`: pares de peças no mesmo look de treino, a de A com valor x e a de B com valor y.
- `esperado(x, y) = total × fração de x no lado A × fração de y no lado B`.
- `pontos(x, y) = log((n + 1) / (esperado + 1))`.

O total é o número de pares A–B em que as duas peças têm valor conhecido. A fração de x no lado A é a parte desses pares em que a peça de A tem valor x. Ela conta aparições em pares, e não peças do catálogo, para que o esperado some o mesmo que o n. Valor desconhecido fica fora das contas e vale 0. Duas peças da mesma categoria no mesmo look não entram, e o par vale 0.

Estilo e ocasião podem ter vários valores marcados. Cada peça reparte o peso 1 entre os seus valores (dois marcados, metade para cada). Nenhum marcado conta como desconhecido. Os pontos do par de peças são a média dos pontos dos pares de valores marcados, que é a mesma divisão usada na contagem.

Formalidade: o número de 0 a 1 vira baixa (abaixo de 1/3), média (de 1/3 a 2/3) e alta (acima de 2/3). O valor 0,5 conta como desconhecido. Ele quer dizer, quase sempre, que o texto não tem palavra formal nem casual. Sem o texto original não dá para separar o 0,5 de empate, que é raro (mesma regra de `comparacao_fontes.md`). Comprimento só vale para roupas (parte de cima, parte de baixo e peça única). Nas outras categorias fica desconhecido. A cor é o `grupo_cor` de `dados/pecas_v5.csv` (ver `grupos_cor.md`).

`pontos_atributos(p, c)` é a soma, nos 6 atributos, de `peso × pontos`. Os pesos começam em 1.

## Categorias e grupos de categoria

Se um par de categorias tem menos de 200 pares de peças nos looks de treino, o par usa a tabela dos grupos de categoria. Na tabela de grupo, um par dentro do mesmo grupo (blusa com casaco) conta meio par em cada sentido.

| categoria | peças | grupo |
| --- | --- | --- |
| bag | 17052 | bolsa |
| bracelet | 3700 | acessório |
| brooch | 832 | acessório |
| dress | 6165 | peça única |
| earrings | 4502 | acessório |
| eyewear | 4201 | acessório |
| gloves | 344 | acessório |
| hairwear | 461 | acessório |
| hats | 2322 | acessório |
| jumpsuit | 186 | peça única |
| legwear | 169 | parte de baixo |
| necklace | 4449 | acessório |
| neckwear | 1329 | acessório |
| outerwear | 8116 | parte de cima |
| pants | 6632 | parte de baixo |
| rings | 2885 | acessório |
| shoes | 16033 | calçado |
| skirt | 4567 | parte de baixo |
| top | 16717 | parte de cima |
| watch | 1708 | acessório |

Dos 190 pares de categorias, 109 têm tabela própria e 81 usam o grupo. Os que usam o grupo e aparecem pelo menos uma vez: bag–jumpsuit (99), bag–legwear (60), bracelet–gloves (39), bracelet–hairwear (109), bracelet–jumpsuit (14), bracelet–legwear (15), brooch–earrings (185), brooch–eyewear (146), brooch–gloves (17), brooch–hairwear (71), brooch–hats (59), brooch–jumpsuit (4), brooch–legwear (7), brooch–necklace (183), brooch–neckwear (77), brooch–pants (152), brooch–rings (140), brooch–skirt (129), brooch–watch (38), dress–gloves (75), dress–hairwear (122), dress–jumpsuit (4), dress–legwear (16), dress–neckwear (175), dress–skirt (148), earrings–gloves (60), earrings–hairwear (109), earrings–jumpsuit (19), earrings–legwear (21), eyewear–gloves (82), eyewear–hairwear (82), eyewear–jumpsuit (36), eyewear–legwear (32), gloves–hairwear (15), gloves–hats (76), gloves–legwear (4), gloves–necklace (75), gloves–neckwear (82), gloves–outerwear (193), gloves–pants (74), gloves–rings (26), gloves–skirt (64), gloves–watch (23), hairwear–hats (21), hairwear–jumpsuit (3), hairwear–legwear (10), hairwear–necklace (89), hairwear–neckwear (29), hairwear–outerwear (108), hairwear–pants (108), hairwear–rings (59), hairwear–skirt (95), hairwear–watch (25), hats–jumpsuit (21), hats–legwear (25), hats–neckwear (186), jumpsuit–legwear (1), jumpsuit–necklace (28), jumpsuit–neckwear (6), jumpsuit–outerwear (42), jumpsuit–pants (9), jumpsuit–rings (14), jumpsuit–shoes (105), jumpsuit–skirt (2), jumpsuit–top (50), jumpsuit–watch (12), legwear–necklace (24), legwear–neckwear (7), legwear–outerwear (57), legwear–pants (38), legwear–rings (13), legwear–shoes (71), legwear–skirt (35), legwear–top (114), legwear–watch (11), necklace–neckwear (178), neckwear–rings (174), neckwear–skirt (192), neckwear–watch (100), pants–skirt (65).

## Pares mais positivos e mais negativos

Entram as tabelas em uso: as de categoria com pares suficientes e as de grupo. Ao lado vêm o n e o esperado. Com n e esperado pequenos, o número se apoia em poucas contagens. O +1 da conta já puxa esses casos para perto de 0.

### cor

Mais positivos:

| lado A | valor A | lado B | valor B | n | esperado | pontos |
| --- | --- | --- | --- | --- | --- | --- |
| bag | rosa | shoes | rosa | 47,0 | 3,2 | 2,43 |
| acessório (grupo) | rosa | acessório (grupo) | rosa | 27,0 | 1,5 | 2,43 |
| acessório (grupo) | marinho | acessório (grupo) | marinho | 51,0 | 3,8 | 2,38 |
| acessório (grupo) | rosa | parte de cima (grupo) | rosa | 53,0 | 4,1 | 2,36 |
| bag | verde | shoes | verde | 26,0 | 1,7 | 2,32 |
| bag | rosa | earrings | rosa | 17,0 | 0,8 | 2,30 |
| bag | marinho | shoes | marinho | 60,0 | 5,2 | 2,28 |
| earrings | marinho | shoes | marinho | 24,0 | 1,6 | 2,26 |
| bag | verde | earrings | verde | 20,0 | 1,4 | 2,17 |
| shoes | verde | top | verde | 16,0 | 1,0 | 2,16 |

Mais negativos:

| lado A | valor A | lado B | valor B | n | esperado | pontos |
| --- | --- | --- | --- | --- | --- | --- |
| outerwear | bege | skirt | cinza 1 | 0,0 | 10,0 | −2,40 |
| dress | bege | necklace | cinza 2 | 0,0 | 8,0 | −2,20 |
| bolsa (grupo) | marrom | peça única (grupo) | rosa | 0,0 | 7,1 | −2,09 |
| bag | marrom | dress | rosa | 0,0 | 6,8 | −2,06 |
| bracelet | bege | earrings | verde | 0,0 | 6,8 | −2,05 |
| bag | marinho | rings | preto | 0,0 | 6,5 | −2,02 |
| gloves | preto | shoes | marrom | 0,0 | 6,5 | −2,01 |
| gloves | preto | shoes | vinho | 0,0 | 6,5 | −2,01 |
| bracelet | marinho | top | preto | 0,0 | 5,7 | −1,90 |
| outerwear | preto | watch | marrom | 0,0 | 5,2 | −1,83 |

### estampa

Mais positivos:

| lado A | valor A | lado B | valor B | n | esperado | pontos |
| --- | --- | --- | --- | --- | --- | --- |
| bag | poá | skirt | poá | 4,0 | 0,2 | 1,42 |
| bag | animal print | hats | animal print | 4,0 | 0,2 | 1,40 |
| bag | animal print | neckwear | animal print | 9,0 | 1,5 | 1,40 |
| bolsa (grupo) | poá | parte de baixo (grupo) | poá | 4,0 | 0,3 | 1,37 |
| shoes | animal print | top | animal print | 13,0 | 2,8 | 1,31 |
| bag | animal print | shoes | animal print | 22,0 | 5,6 | 1,25 |
| shoes | poá | top | poá | 3,0 | 0,2 | 1,18 |
| bag | animal print | top | animal print | 12,0 | 3,0 | 1,18 |
| bolsa (grupo) | xadrez | parte de baixo (grupo) | xadrez | 4,0 | 0,6 | 1,14 |
| skirt | poá | top | poá | 4,0 | 0,6 | 1,13 |

Mais negativos:

| lado A | valor A | lado B | valor B | n | esperado | pontos |
| --- | --- | --- | --- | --- | --- | --- |
| bag | floral | shoes | animal print | 0,0 | 6,6 | −2,03 |
| skirt | floral | top | animal print | 0,0 | 5,1 | −1,81 |
| dress | outra estampa | outerwear | floral | 0,0 | 4,6 | −1,73 |
| skirt | floral | top | poá | 0,0 | 4,1 | −1,62 |
| parte de baixo (grupo) | animal print | parte de cima (grupo) | floral | 0,0 | 3,4 | −1,48 |
| parte de baixo (grupo) | xadrez | parte de cima (grupo) | floral | 2,0 | 11,9 | −1,46 |
| bag | animal print | skirt | listrada | 0,0 | 3,3 | −1,46 |
| acessório (grupo) | animal print | parte de cima (grupo) | floral | 0,0 | 3,3 | −1,45 |
| bag | animal print | skirt | xadrez | 0,0 | 3,2 | −1,45 |
| bag | listrada | dress | outra estampa | 0,0 | 3,2 | −1,44 |

### formalidade

Mais positivos:

| lado A | valor A | lado B | valor B | n | esperado | pontos |
| --- | --- | --- | --- | --- | --- | --- |
| earrings | média | shoes | baixa | 9,0 | 3,2 | 0,88 |
| bracelet | baixa | watch | média | 2,0 | 0,3 | 0,86 |
| bag | média | earrings | média | 2,0 | 0,3 | 0,83 |
| dress | baixa | watch | baixa | 3,0 | 0,7 | 0,83 |
| bag | média | brooch | baixa | 3,0 | 0,8 | 0,83 |
| earrings | baixa | rings | baixa | 4,0 | 1,2 | 0,80 |
| earrings | baixa | necklace | baixa | 7,0 | 2,8 | 0,74 |
| necklace | média | top | média | 4,0 | 1,4 | 0,72 |
| gloves | baixa | top | média | 3,0 | 1,0 | 0,70 |
| bag | média | bracelet | baixa | 12,0 | 5,6 | 0,68 |

Mais negativos:

| lado A | valor A | lado B | valor B | n | esperado | pontos |
| --- | --- | --- | --- | --- | --- | --- |
| neckwear | baixa | pants | média | 0,0 | 4,0 | −1,61 |
| parte de baixo (grupo) | média | peça única (grupo) | baixa | 0,0 | 2,7 | −1,32 |
| dress | baixa | pants | média | 0,0 | 2,2 | −1,15 |
| bracelet | média | top | alta | 3,0 | 11,4 | −1,14 |
| skirt | alta | watch | média | 0,0 | 2,1 | −1,13 |
| hats | média | top | média | 0,0 | 1,9 | −1,07 |
| hats | média | pants | alta | 0,0 | 1,7 | −1,01 |
| earrings | baixa | pants | média | 0,0 | 1,7 | −0,99 |
| bracelet | alta | watch | média | 0,0 | 1,7 | −0,98 |
| dress | baixa | earrings | média | 0,0 | 1,6 | −0,95 |

### estilo

Mais positivos:

| lado A | valor A | lado B | valor B | n | esperado | pontos |
| --- | --- | --- | --- | --- | --- | --- |
| bracelet | clássico | shoes | clássico | 10,1 | 4,0 | 0,79 |
| hats | clássico | rings | romântico | 2,0 | 0,4 | 0,75 |
| eyewear | esportivo | top | romântico | 2,3 | 0,6 | 0,72 |
| earrings | romântico | watch | romântico | 1,5 | 0,2 | 0,69 |
| earrings | romântico | necklace | romântico | 10,2 | 4,7 | 0,68 |
| bag | romântico | rings | romântico | 8,8 | 4,0 | 0,67 |
| bag | clássico | hairwear | clássico | 4,0 | 1,6 | 0,67 |
| shoes | romântico | watch | romântico | 1,5 | 0,3 | 0,65 |
| dress | casual | watch | romântico | 2,0 | 0,6 | 0,65 |
| outerwear | romântico | shoes | romântico | 8,2 | 3,9 | 0,63 |

Mais negativos:

| lado A | valor A | lado B | valor B | n | esperado | pontos |
| --- | --- | --- | --- | --- | --- | --- |
| necklace | clássico | watch | casual | 0,0 | 2,8 | −1,35 |
| eyewear | clássico | rings | romântico | 0,2 | 2,5 | −1,10 |
| earrings | clássico | outerwear | esportivo | 0,0 | 1,9 | −1,08 |
| earrings | moderno | neckwear | clássico | 0,0 | 1,9 | −1,06 |
| dress | clássico | hats | romântico | 0,0 | 1,7 | −0,98 |
| shoes | romântico | watch | casual | 0,0 | 1,5 | −0,93 |
| bag | romântico | neckwear | clássico | 0,3 | 2,1 | −0,85 |
| bracelet | moderno | earrings | romântico | 4,0 | 10,6 | −0,84 |
| necklace | casual | shoes | esportivo | 0,3 | 2,1 | −0,84 |
| calçado (grupo) | esportivo | parte de baixo (grupo) | romântico | 1,6 | 4,9 | −0,83 |

### ocasião

Mais positivos:

| lado A | valor A | lado B | valor B | n | esperado | pontos |
| --- | --- | --- | --- | --- | --- | --- |
| bag | esporte | shoes | esporte | 4,0 | 0,7 | 1,07 |
| hats | esporte | shoes | esporte | 2,9 | 0,7 | 0,86 |
| bag | praia | shoes | praia | 1,5 | 0,1 | 0,84 |
| outerwear | esporte | shoes | esporte | 3,8 | 1,1 | 0,81 |
| bag | praia | top | praia | 1,2 | 0,1 | 0,72 |
| shoes | esporte | top | esporte | 5,4 | 2,4 | 0,64 |
| bag | esporte | top | esporte | 2,9 | 1,1 | 0,62 |
| bag | esporte | hats | esporte | 1,1 | 0,1 | 0,62 |
| parte de baixo (grupo) | esporte | parte de cima (grupo) | esporte | 7,3 | 3,6 | 0,58 |
| calçado (grupo) | esporte | parte de baixo (grupo) | esporte | 4,5 | 2,1 | 0,57 |

Mais negativos:

| lado A | valor A | lado B | valor B | n | esperado | pontos |
| --- | --- | --- | --- | --- | --- | --- |
| dress | trabalho | outerwear | dia a dia | 0,4 | 1,9 | −0,77 |
| bracelet | festa | pants | praia | 2,2 | 5,7 | −0,72 |
| necklace | trabalho | watch | dia a dia | 0,3 | 1,7 | −0,72 |
| bracelet | festa | skirt | praia | 0,2 | 1,5 | −0,71 |
| bracelet | dia a dia | hats | esporte | 0,0 | 1,0 | −0,69 |
| bag | trabalho | top | praia | 0,6 | 2,0 | −0,63 |
| eyewear | esporte | top | trabalho | 0,0 | 0,9 | −0,63 |
| pants | praia | rings | dia a dia | 0,8 | 2,2 | −0,59 |
| hats | praia | outerwear | dia a dia | 0,0 | 0,8 | −0,57 |
| hats | esporte | watch | festa | 1,1 | 2,6 | −0,55 |

### comprimento

Mais positivos:

| lado A | valor A | lado B | valor B | n | esperado | pontos |
| --- | --- | --- | --- | --- | --- | --- |
| parte de baixo (grupo) | médio | parte de baixo (grupo) | longo | 2,0 | 1,0 | 0,42 |
| parte de baixo (grupo) | longo | peça única (grupo) | curto | 4,0 | 2,4 | 0,39 |
| dress | curto | pants | longo | 3,0 | 1,9 | 0,31 |
| parte de baixo (grupo) | curto | peça única (grupo) | longo | 7,0 | 5,2 | 0,25 |
| dress | longo | pants | curto | 5,0 | 3,9 | 0,21 |
| parte de baixo (grupo) | curto | parte de baixo (grupo) | curto | 6,0 | 5,5 | 0,07 |
| parte de baixo (grupo) | longo | peça única (grupo) | médio | 2,0 | 1,8 | 0,07 |
| dress | médio | pants | longo | 2,0 | 1,9 | 0,02 |
| parte de baixo (grupo) | curto | parte de baixo (grupo) | longo | 3,5 | 3,4 | 0,02 |
| parte de cima (grupo) | longo | peça única (grupo) | longo | 4,0 | 4,0 | 0,00 |

Mais negativos:

| lado A | valor A | lado B | valor B | n | esperado | pontos |
| --- | --- | --- | --- | --- | --- | --- |
| parte de baixo (grupo) | curto | peça única (grupo) | curto | 0,0 | 1,6 | −0,96 |
| dress | curto | pants | curto | 0,0 | 1,1 | −0,72 |
| parte de baixo (grupo) | longo | parte de baixo (grupo) | longo | 1,0 | 2,1 | −0,44 |
| parte de baixo (grupo) | médio | parte de baixo (grupo) | médio | 0,0 | 0,5 | −0,37 |
| parte de baixo (grupo) | curto | parte de baixo (grupo) | médio | 1,0 | 1,6 | −0,25 |
| parte de baixo (grupo) | longo | peça única (grupo) | longo | 6,0 | 7,8 | −0,23 |
| dress | longo | pants | longo | 6,0 | 7,1 | −0,15 |
| parte de baixo (grupo) | curto | peça única (grupo) | médio | 1,0 | 1,2 | −0,10 |
| dress | médio | pants | curto | 1,0 | 1,1 | −0,03 |
| dress | longo | outerwear | longo | 4,0 | 4,0 | 0,00 |

## Diagnóstico na validação

Só diagnóstico, não escolhe nada. As tabelas são refeitas sem os looks de validação. Em cada consulta de validação (mesmas escondidas da avaliação justa), comparo os pontos da escondida com as peças do look contra os pontos de 10 peças sorteadas da mesma categoria (média das sorteadas). Pesos todos 1. O valor é a média por par de peças. O erro padrão sai das consultas, que são looks diferentes. Diferença maior que uns 3 erros padrão não parece sorte.

Há dois sorteios. No catálogo, toda peça da categoria tem a mesma chance, como na lista que o recomendador ordena. Nas aparições, a peça sai na proporção dos looks de treino em que aparece, como a escondida. O segundo tira o efeito de a cor ou o valor ser mais comum nos looks do que no catálogo e deixa só o efeito do par.

### Sorteio no catálogo

2316 consultas, 7377 pares, 2,5% deles com a tabela de grupo.

| atributo | par conhecido (%) | certo | sorteado | diferença | erro padrão | certo ganha (%) | certo perde (%) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| cor | 100,0 | 0,0428 | −0,0787 | 0,1215 | 0,0067 | 65,9 | 34,1 |
| estampa | 56,4 | −0,0005 | −0,0016 | 0,0011 | 0,0011 | 33,7 | 28,2 |
| formalidade | 60,4 | 0,0011 | −0,0035 | 0,0046 | 0,0012 | 55,1 | 38,3 |
| estilo | 39,7 | −0,0001 | −0,0028 | 0,0026 | 0,0008 | 50,7 | 38,7 |
| ocasião | 92,3 | 0,0013 | −0,0016 | 0,0029 | 0,0005 | 56,2 | 42,6 |
| comprimento | 0,1 | 0,0000 | 0,0000 | 0,0000 | 0,0000 | 0,0 | 0,1 |
| soma |  | 0,0446 | −0,0882 | 0,1328 | 0,0071 | 67,0 | 33,0 |

### Sorteio nas aparições

2316 consultas, 7377 pares, 2,5% deles com a tabela de grupo.

| atributo | par conhecido (%) | certo | sorteado | diferença | erro padrão | certo ganha (%) | certo perde (%) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| cor | 100,0 | 0,0428 | −0,0726 | 0,1154 | 0,0067 | 65,0 | 35,0 |
| estampa | 56,4 | −0,0005 | −0,0012 | 0,0007 | 0,0011 | 31,5 | 28,1 |
| formalidade | 60,4 | 0,0011 | −0,0031 | 0,0042 | 0,0012 | 53,8 | 39,3 |
| estilo | 39,7 | −0,0001 | −0,0026 | 0,0025 | 0,0008 | 49,5 | 39,9 |
| ocasião | 92,3 | 0,0013 | −0,0018 | 0,0031 | 0,0005 | 55,9 | 43,0 |
| comprimento | 0,1 | 0,0000 | 0,0000 | 0,0000 | 0,0000 | 0,1 | 0,1 |
| soma |  | 0,0446 | −0,0814 | 0,1260 | 0,0070 | 65,8 | 34,2 |

Certo ganha e certo perde contam as consultas em que a soma da escondida fica acima ou abaixo da média das sorteadas. O resto empata, quase sempre porque o valor é desconhecido.
