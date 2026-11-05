# Formas de agrupar as peças

Gerado por `./recomendador grupos_formas`. Nenhuma forma usa looks. Grupos sempre dentro de uma categoria.
As formas 1 e 2 usam o grafo kNN (k = 10) do vetor L, a, b e estampa com peso 0,1.
A forma 1 para quando chega a 50 grupos ou quando nenhuma aresta une dois grupos sem passar o limite. O limite é 5% das peças da categoria, e no mínimo 2.
A forma 3 não usa ocasião porque a peça pode ter várias ao mesmo tempo, e as combinações picariam os grupos.

Critério, escrito antes de rodar: fica a forma de menor média da fração do maior grupo (média entre as 20 categorias), entre as que têm no máximo 20% dos grupos com 1 peça só. O Kruskal puro fica só como comparação.

| Forma | grupos | maior grupo (média das categorias) | pior categoria | grupos com 1 peça | peças sozinhas |
| --- | --- | --- | --- | --- | --- |
| Kruskal puro (K = 50) | 1000 | 83,4% | outerwear 99,3% | 691 (69,1%) | 0,7% |
| 1. Kruskal com limite de 5% (K = 50) | 1076 | 5,0% | bracelet 5,0% | 409 (38,0%) | 0,4% |
| 2. kNN mútuo + Union-Find (k = 10) | 1515 | 96,3% | eyewear 99,8% | 997 (65,8%) | 1,0% |
| 3. Mesma combinação de grupo_cor, estampa e formalidade **(escolhida)** | 1255 | 17,6% | gloves 33,7% | 114 (9,1%) | 0,1% |

No Kruskal puro, as arestas mais curtas são unidas primeiro. Num grafo denso de cores parecidas, sempre existe uma aresta curta que liga o grupo grande a mais uma peça, então ele cresce como uma corrente. As peças de cor rara ficam por último e viram os outros grupos, quase todos pequenos.

## Por categoria (grupos / maior grupo)

| categoria | peças | forma pura | forma 1 | forma 2 | forma 3 |
| --- | --- | --- | --- | --- | --- |
| bag | 17052 | 50 / 98,9% | 78 / 5,0% | 349 / 94,2% | 92 / 13,8% |
| bracelet | 3700 | 50 / 98,0% | 50 / 5,0% | 17 / 99,4% | 46 / 17,4% |
| brooch | 832 | 50 / 83,1% | 50 / 4,9% | 7 / 99,3% | 38 / 22,4% |
| dress | 6165 | 50 / 98,8% | 50 / 5,0% | 43 / 97,4% | 90 / 15,1% |
| earrings | 4502 | 50 / 98,5% | 50 / 5,0% | 19 / 99,3% | 45 / 23,1% |
| eyewear | 4201 | 50 / 97,9% | 50 / 5,0% | 9 / 99,8% | 46 / 9,2% |
| gloves | 344 | 50 / 50,6% | 50 / 4,9% | 6 / 97,1% | 31 / 33,7% |
| hairwear | 461 | 50 / 51,8% | 50 / 5,0% | 4 / 99,3% | 37 / 16,5% |
| hats | 2322 | 50 / 96,5% | 50 / 5,0% | 35 / 95,8% | 73 / 15,7% |
| jumpsuit | 186 | 50 / 29,6% | 50 / 4,8% | 5 / 97,8% | 51 / 11,8% |
| legwear | 169 | 50 / 17,2% | 50 / 4,7% | 7 / 81,7% | 44 / 10,1% |
| necklace | 4449 | 50 / 97,9% | 50 / 5,0% | 20 / 99,3% | 45 / 28,1% |
| neckwear | 1329 | 50 / 94,3% | 50 / 5,0% | 7 / 99,3% | 83 / 11,3% |
| outerwear | 8116 | 50 / 99,3% | 54 / 5,0% | 206 / 93,0% | 94 / 10,7% |
| pants | 6632 | 50 / 98,8% | 50 / 5,0% | 100 / 94,7% | 87 / 16,3% |
| rings | 2885 | 50 / 97,0% | 50 / 5,0% | 28 / 98,4% | 42 / 28,8% |
| shoes | 16033 | 50 / 98,9% | 81 / 5,0% | 241 / 94,2% | 87 / 19,1% |
| skirt | 4567 | 50 / 98,5% | 50 / 5,0% | 54 / 94,7% | 88 / 12,3% |
| top | 16717 | 50 / 68,6% | 63 / 5,0% | 340 / 93,3% | 95 / 10,8% |
| watch | 1708 | 50 / 93,7% | 50 / 5,0% | 18 / 98,7% | 41 / 26,5% |

## Exemplos

Cinco peças (as de menor índice) de 3 grupos de top em cada forma.

### Kruskal puro (K = 50)

top, maior grupo (11460 peças):

| id | grupo_cor | Lab | estampa | formalidade | ocasião |
| --- | --- | --- | --- | --- | --- |
| 0 | verde | 78/-24/-12 | lisa | baixa | dia a dia, festa |
| 1 | branco | 87/4/7 | lisa | baixa | dia a dia, festa |
| 3 | preto | 11/3/-1 | lisa | alta | dia a dia, festa |
| 6 | preto | 5/2/-1 | lisa | baixa | trabalho, festa |
| 7 | verde | 53/-25/8 | lisa | desconhecido | trabalho, festa |

top, grupo do meio (1 peça):

| id | grupo_cor | Lab | estampa | formalidade | ocasião |
| --- | --- | --- | --- | --- | --- |
| 7865 | marinho | 59/25/-59 | lisa | baixa | dia a dia, festa |

top, menor grupo com 5 peças ou mais (6 peças):

| id | grupo_cor | Lab | estampa | formalidade | ocasião |
| --- | --- | --- | --- | --- | --- |
| 1870 | verde | 66/-27/38 | lisa | desconhecido | dia a dia, festa |
| 4772 | verde | 71/-32/39 | lisa | baixa | dia a dia, festa |
| 7230 | verde | 65/-37/35 | lisa | media | dia a dia, trabalho |
| 7679 | verde | 75/-41/29 | lisa | baixa | dia a dia, festa |
| 12755 | verde | 70/-22/34 | lisa | desconhecido | dia a dia, festa |

### 1. Kruskal com limite de 5% (K = 50)

top, maior grupo (835 peças):

| id | grupo_cor | Lab | estampa | formalidade | ocasião |
| --- | --- | --- | --- | --- | --- |
| 0 | verde | 78/-24/-12 | lisa | baixa | dia a dia, festa |
| 7 | verde | 53/-25/8 | lisa | desconhecido | trabalho, festa |
| 16 | vinho | 31/40/-10 | lisa | alta | festa |
| 22 | verde | 37/-35/20 | lisa | baixa | dia a dia |
| 27 | marinho | 50/5/-29 | lisa | alta | festa |

top, grupo do meio (3 peças):

| id | grupo_cor | Lab | estampa | formalidade | ocasião |
| --- | --- | --- | --- | --- | --- |
| 6815 | branco | 88/-2/5 | lisa | desconhecido | festa |
| 12929 | branco | 89/-2/6 | lisa | alta | dia a dia, festa |
| 30723 | branco | 88/-2/4 | lisa | desconhecido | dia a dia, festa |

top, menor grupo com 5 peças ou mais (5 peças):

| id | grupo_cor | Lab | estampa | formalidade | ocasião |
| --- | --- | --- | --- | --- | --- |
| 5504 | branco | 95/1/3 | lisa | desconhecido | dia a dia |
| 5784 | branco | 96/1/2 | lisa | baixa | dia a dia |
| 7702 | branco | 96/1/2 | lisa | media | dia a dia |
| 15342 | branco | 95/1/3 | lisa | baixa | dia a dia, festa |
| 15623 | branco | 96/1/3 | lisa | alta | dia a dia, festa |

### 2. kNN mútuo + Union-Find (k = 10)

top, maior grupo (15593 peças):

| id | grupo_cor | Lab | estampa | formalidade | ocasião |
| --- | --- | --- | --- | --- | --- |
| 0 | verde | 78/-24/-12 | lisa | baixa | dia a dia, festa |
| 1 | branco | 87/4/7 | lisa | baixa | dia a dia, festa |
| 2 | cinza 2 | 49/1/-8 | outra | baixa | dia a dia |
| 3 | preto | 11/3/-1 | lisa | alta | dia a dia, festa |
| 5 | cinza 2 | 68/4/5 | outra | baixa | dia a dia |

top, grupo do meio (1 peça):

| id | grupo_cor | Lab | estampa | formalidade | ocasião |
| --- | --- | --- | --- | --- | --- |
| 1752 | preto | 11/0/0 | lisa | desconhecido | festa |

top, menor grupo com 5 peças ou mais (5 peças):

| id | grupo_cor | Lab | estampa | formalidade | ocasião |
| --- | --- | --- | --- | --- | --- |
| 7526 | preto | 7/0/0 | lisa | desconhecido | dia a dia, festa |
| 10349 | preto | 7/0/0 | lisa | alta | festa |
| 11457 | preto | 7/0/0 | lisa | alta | dia a dia, festa |
| 13196 | preto | 7/0/0 | lisa | desconhecido | nenhuma |
| 14491 | preto | 7/0/0 | lisa | desconhecido | festa |

### 3. Mesma combinação de grupo_cor, estampa e formalidade

top, maior grupo (1809 peças):

| id | grupo_cor | Lab | estampa | formalidade | ocasião |
| --- | --- | --- | --- | --- | --- |
| 8 | branco | 87/2/6 | lisa | alta | dia a dia, trabalho, festa |
| 12 | branco | 94/1/-2 | lisa | alta | festa |
| 17 | branco | 94/1/-1 | lisa | alta | dia a dia, festa |
| 20 | branco | 86/1/-10 | lisa | alta | dia a dia, trabalho |
| 21 | branco | 93/1/9 | lisa | alta | festa |

top, grupo do meio (63 peças):

| id | grupo_cor | Lab | estampa | formalidade | ocasião |
| --- | --- | --- | --- | --- | --- |
| 624 | rosa | 74/42/-4 | lisa | alta | dia a dia, trabalho |
| 626 | rosa | 64/61/2 | lisa | alta | trabalho, festa |
| 1013 | rosa | 69/37/15 | lisa | alta | festa |
| 1576 | rosa | 73/41/1 | lisa | alta | festa |
| 2812 | rosa | 64/59/5 | lisa | alta | festa |

top, menor grupo com 5 peças ou mais (5 peças):

| id | grupo_cor | Lab | estampa | formalidade | ocasião |
| --- | --- | --- | --- | --- | --- |
| 6292 | amarelo | 94/-12/49 | outra | desconhecido | dia a dia, festa |
| 7456 | amarelo | 68/22/62 | floral | desconhecido | dia a dia, festa |
| 9005 | amarelo | 79/3/67 | outra | desconhecido | dia a dia, festa |
| 9039 | amarelo | 73/11/70 | floral | desconhecido | dia a dia |
| 14508 | amarelo | 79/1/58 | listrada | desconhecido | dia a dia, festa |

## Peso da estampa

A forma escolhida não usa o vetor, então o peso da estampa não se aplica.

Gephi da forma escolhida (top, pants e shoes): `gephi_grupos/escolhida_nodes.csv` e `_edges.csv`. A coluna Grupo serve para colorir (Partition).
