# Grupos com Kruskal

Gerado por `./recomendador grupos`. Vetor de cada peça: L, a, b (delta E / 100) e estampa com peso 0.10 (lisa 0, com estampa 1, desconhecido 0,5).
Na descrição dos grupos, desconhecido conta como sem estampa.
Grafo kNN com k = 10 dentro de cada categoria: 655776 arestas, montado em 1.38 s.

Corrente = categoria com um grupo que tem mais da metade das peças.

| K | grupos no total | categorias que não chegaram a K | maior grupo | categorias com corrente | Kruskal (s) | pontos (s) |
| --- | --- | --- | --- | --- | --- | --- |
| 20 | 400 | 0 | 16897 (bag, 99%) | 19 de 20 | 0.008 | 0.005 |
| 50 | 1000 | 0 | 16859 (bag, 99%) | 18 de 20 | 0.007 | 0.006 |
| 100 | 2000 | 0 | 14952 (bag, 88%) | 16 de 20 | 0.007 | 0.007 |

## K = 20

Categorias com corrente: bag 99%, bracelet 99%, brooch 97%, dress 99%, earrings 99%, eyewear 99%, gloves 92%, hairwear 89%, hats 99%, jumpsuit 82%, necklace 99%, neckwear 98%, outerwear 100%, pants 100%, rings 99%, shoes 99%, skirt 99%, top 100%, watch 97%.

Pares de peças nos looks de validação (pontos contados sem eles): média -0.001 no par certo e -0.001 quando a segunda peça é sorteada da mesma categoria (37458 pares).

Pares de grupos com mais pontos (contagem de pelo menos 20 pares):

| grupo 1 | grupo 2 | n | esperado | pontos |
| --- | --- | --- | --- | --- |
| bracelet (3666 peças, Lab 60/4/11, 0% com estampa) | hairwear (20 peças, Lab 82/-7/53, 0% com estampa) | 23 | 4.7 | 1.44 |
| bag (29 peças, Lab 10/0/0, 0% com estampa) | top (16682 peças, Lab 59/5/5, 31% com estampa) | 32 | 18.9 | 0.51 |
| bag (21 peças, Lab 13/0/0, 0% com estampa) | top (16682 peças, Lab 59/5/5, 31% com estampa) | 23 | 13.7 | 0.49 |
| shoes (21 peças, Lab 14/0/0, 0% com estampa) | top (16682 peças, Lab 59/5/5, 31% com estampa) | 21 | 13.1 | 0.44 |
| bag (23 peças, Lab 15/0/0, 0% com estampa) | top (16682 peças, Lab 59/5/5, 31% com estampa) | 22 | 15.0 | 0.36 |
| shoes (27 peças, Lab 8/0/0, 0% com estampa) | top (16682 peças, Lab 59/5/5, 31% com estampa) | 21 | 16.8 | 0.21 |
| hats (2293 peças, Lab 43/6/7, 10% com estampa) | jumpsuit (153 peças, Lab 48/2/-2, 27% com estampa) | 20 | 17.1 | 0.15 |
| bag (16897 peças, Lab 46/9/9, 11% com estampa) | shoes (27 peças, Lab 8/0/0, 0% com estampa) | 21 | 18.0 | 0.14 |
| legwear (75 peças, Lab 80/1/4, 28% com estampa) | top (16682 peças, Lab 59/5/5, 31% com estampa) | 58 | 50.5 | 0.14 |
| legwear (75 peças, Lab 80/1/4, 28% com estampa) | outerwear (8089 peças, Lab 45/6/6, 12% com estampa) | 28 | 25.2 | 0.10 |

## K = 50

Categorias com corrente: bag 99%, bracelet 98%, brooch 83%, dress 99%, earrings 98%, eyewear 98%, gloves 51%, hairwear 52%, hats 97%, necklace 98%, neckwear 94%, outerwear 99%, pants 99%, rings 97%, shoes 99%, skirt 99%, top 69%, watch 94%.

Pares de peças nos looks de validação (pontos contados sem eles): média -0.004 no par certo e -0.003 quando a segunda peça é sorteada da mesma categoria (37458 pares).

Pares de grupos com mais pontos (contagem de pelo menos 20 pares):

| grupo 1 | grupo 2 | n | esperado | pontos |
| --- | --- | --- | --- | --- |
| hairwear (30 peças, Lab 42/57/33, 0% com estampa) | top (11460 peças, Lab 60/5/5, 0% com estampa) | 24 | 12.7 | 0.60 |
| bag (29 peças, Lab 10/0/0, 0% com estampa) | top (11460 peças, Lab 60/5/5, 0% com estampa) | 23 | 13.0 | 0.54 |
| brooch (44 peças, Lab 39/54/34, 0% com estampa) | dress (6094 peças, Lab 49/9/6, 31% com estampa) | 21 | 12.2 | 0.51 |
| gloves (81 peças, Lab 79/5/11, 0% com estampa) | necklace (4354 peças, Lab 67/3/9, 0% com estampa) | 24 | 17.3 | 0.31 |
| jumpsuit (31 peças, Lab 88/2/2, 0% com estampa) | shoes (15864 peças, Lab 46/7/8, 7% com estampa) | 24 | 17.3 | 0.31 |
| hairwear (93 peças, Lab 13/1/-1, 0% com estampa) | top (5168 peças, Lab 58/5/3, 100% com estampa) | 23 | 17.7 | 0.25 |
| hairwear (93 peças, Lab 13/1/-1, 0% com estampa) | outerwear (8056 peças, Lab 45/6/6, 12% com estampa) | 28 | 21.6 | 0.25 |
| bracelet (3627 peças, Lab 60/5/12, 0% com estampa) | gloves (174 peças, Lab 16/2/1, 0% com estampa) | 25 | 19.3 | 0.25 |
| hairwear (93 peças, Lab 13/1/-1, 0% com estampa) | necklace (4354 peças, Lab 67/3/9, 0% com estampa) | 22 | 17.6 | 0.21 |
| gloves (81 peças, Lab 79/5/11, 0% com estampa) | top (11460 peças, Lab 60/5/5, 0% com estampa) | 42 | 35.0 | 0.18 |

## K = 100

Categorias com corrente: bag 88%, bracelet 96%, brooch 59%, dress 68%, earrings 95%, eyewear 95%, hats 81%, necklace 97%, neckwear 56%, outerwear 87%, pants 87%, rings 95%, shoes 92%, skirt 61%, top 68%, watch 91%.

Pares de peças nos looks de validação (pontos contados sem eles): média -0.005 no par certo e -0.006 quando a segunda peça é sorteada da mesma categoria (37458 pares).

Pares de grupos com mais pontos (contagem de pelo menos 20 pares):

| grupo 1 | grupo 2 | n | esperado | pontos |
| --- | --- | --- | --- | --- |
| bag (29 peças, Lab 10/0/0, 0% com estampa) | top (11436 peças, Lab 60/5/5, 0% com estampa) | 23 | 12.9 | 0.54 |
| jumpsuit (24 peças, Lab 90/0/-0, 0% com estampa) | shoes (14709 peças, Lab 46/7/8, 0% com estampa) | 20 | 12.4 | 0.45 |
| skirt (33 peças, Lab 40/55/31, 100% com estampa) | top (11436 peças, Lab 60/5/5, 0% com estampa) | 28 | 19.1 | 0.37 |
| bracelet (3537 peças, Lab 60/4/11, 0% com estampa) | gloves (156 peças, Lab 15/1/0, 0% com estampa) | 24 | 16.9 | 0.33 |
| earrings (4283 peças, Lab 62/5/11, 0% com estampa) | hats (115 peças, Lab 14/1/-0, 100% com estampa) | 23 | 17.1 | 0.28 |
| bag (14952 peças, Lab 46/9/10, 0% com estampa) | shoes (27 peças, Lab 8/0/0, 0% com estampa) | 21 | 16.0 | 0.26 |
| pants (5762 peças, Lab 41/3/-2, 0% com estampa) | skirt (1419 peças, Lab 46/4/2, 100% com estampa) | 23 | 17.5 | 0.26 |
| hairwear (90 peças, Lab 13/1/-1, 0% com estampa) | necklace (4311 peças, Lab 67/3/9, 0% com estampa) | 22 | 16.8 | 0.25 |
| dress (4209 peças, Lab 48/11/7, 0% com estampa) | hairwear (215 peças, Lab 79/3/9, 0% com estampa) | 50 | 38.8 | 0.25 |
| hairwear (90 peças, Lab 13/1/-1, 0% com estampa) | top (5126 peças, Lab 58/5/3, 100% com estampa) | 22 | 17.0 | 0.25 |

Arquivos do Gephi (top, pants e shoes; arestas = floresta do Kruskal): `gephi_grupos/k<K>_nodes.csv` e `_edges.csv`.
