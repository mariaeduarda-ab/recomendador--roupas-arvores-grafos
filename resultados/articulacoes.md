# Articulações e pontes entre grupos

Gerado por `./recomendador articulacoes`. É uma análise feita depois do congelamento. Nenhuma recomendação muda.

## O que é uma articulação

Uma articulação é um grupo que é a única ligação entre estilos. Se ele sai do grafo, um conjunto de grupos fica sem caminho até o resto. Isso não quer dizer que o grupo é o mais versátil. O grupo mais versátil é o que combina com mais grupos, e isso é medido pelo grau. O grau conta os vizinhos. A articulação conta os grupos que só chegam ao resto por ela.

## Grafo

Vértices: os 1255 grupos da forma escolhida (forma 3, mesma combinação de grupo_cor, estampa e formalidade dentro de uma categoria). Os grupos estão em `grupos_formas.md`. Nas tabelas, "estampa?" quer dizer estampa desconhecida.

Arestas: pares de grupos de categorias diferentes que aparecem juntos nos 25066 looks de treino. O peso é `pontos = log((n + 1) / (esperado + 1))` de `PontosGrupos`. Positivo quer dizer que o par aparece mais do que o esperado pelo tamanho dos grupos. Há 65180 pares com n ≥ 1.

Mínimo de contagem: a aresta só entra com n ≥ 3 pares de peças. Com n = 1 ou 2, o par pode vir de um look só. Sem esse corte, 39628 das 50836 arestas positivas teriam n = 1 ou 2 e dominariam o grafo. Com o corte, ficam 15739 pares, dos quais 11208 positivos.

Limiares: os 3 quartis dos pontos das 11208 arestas positivas com n ≥ 3. Assim cada limiar guarda 75%, 50% e 25% dessas arestas, e o grafo vai do mais denso ao mais esparso. Como os pontos são um log, o limiar l quer dizer (n + 1) / (esperado + 1) acima de e^l. Os limiares não foram ajustados para nenhum resultado. A tabela completa usa a mediana.

Articulações que soltam só 1 ou 2 grupos ficam de fora das contas "com 3 ou mais". Grupos separados = grupos que ficam fora do maior pedaço quando a articulação sai.

| limiar | pontos > | razão > | arestas | grupos com aresta | componentes (2+ grupos) | maior componente | articulações | com 3+ separados | maior pedaço solto | pontes | pontes com 3+ separados | tempo do Tarjan |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1º quartil | 0,223 | 1,25 | 8406 | 714 | 1 | 714 | 51 | 6 | 1 | 89 | 0 | 166 µs |
| mediana | 0,437 | 1,55 | 5604 | 711 | 1 | 711 | 53 | 6 | 1 | 96 | 0 | 126 µs |
| 3º quartil | 0,705 | 2,02 | 2802 | 684 | 1 | 684 | 66 | 7 | 1 | 108 | 0 | 84 µs |

O tempo é a média de 1000 execuções do Tarjan no mesmo grafo. Inclui a lista de adjacência que o Tarjan monta, mas não a escolha das arestas pelo limiar. O custo é O(V + E): a DFS passa uma vez por vértice e duas vezes por aresta. A DFS usa uma pilha explícita, sem recursão.

## As 10 articulações mais importantes (limiar na mediana, pontos > 0,437)

Ordem: mais grupos separados, depois maior grau. Pedaços = tamanho de cada pedaço que sobra quando o grupo sai, o primeiro é o maior.

| # | grupo | categoria | cor | estampa | formalidade | peças | grau | grupos separados | pedaços | o que se solta |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 1120 | top | branco | lisa | baixa | 1591 | 67 | 10 | 700 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 | 2 necklace, 2 rings, 1 bag, 1 hairwear, 1 jumpsuit, 1 legwear, 1 neckwear, 1 outerwear |
| 2 | 946 | shoes | preto | lisa | alta | 3067 | 82 | 8 | 702 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 | 2 dress, 2 outerwear, 2 pants, 1 jumpsuit, 1 top |
| 3 | 4 | bag | preto | lisa | alta | 2346 | 66 | 8 | 702 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 | 2 pants, 1 dress, 1 hairwear, 1 rings, 1 shoes, 1 top, 1 watch |
| 4 | 1125 | top | branco | lisa | alta | 1809 | 64 | 4 | 706 + 1 + 1 + 1 + 1 | 2 skirt, 1 bag, 1 earrings |
| 5 | 603 | necklace | branco | estampa? | alta | 1248 | 58 | 3 | 707 + 1 + 1 + 1 | 1 bag, 1 bracelet, 1 top |
| 6 | 1127 | top | branco | estampada | baixa | 1362 | 48 | 3 | 707 + 1 + 1 + 1 | 1 pants, 1 skirt, 1 watch |

Em todas as 53 articulações desse limiar, o maior pedaço que se solta tem 1 grupo. Ou seja, o que se solta são grupos com uma ligação só, pendurados na articulação, e não um estilo inteiro com várias ligações internas. Fora do grafo ficam 544 grupos sem nenhuma aresta nesse limiar.

### Exemplos de peças

As 3 peças de cada grupo que mais aparecem nos looks de treino.

| # | id | categoria | grupo_cor | estampa | formalidade | estilo | ocasião | looks de treino |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 2305 | top | branco | lisa | baixa (0,00) | casual, classico | dia a dia | 13 |
| 1 | 751 | top | branco | lisa | baixa (0,00) | casual, esportivo, moderno | dia a dia, esporte | 11 |
| 1 | 6627 | top | branco | lisa | baixa (0,00) | casual | dia a dia | 8 |
| 2 | 70119 | shoes | preto | lisa | alta (1,00) | nenhum | festa | 14 |
| 2 | 69461 | shoes | preto | lisa | alta (1,00) | nenhum | festa | 12 |
| 2 | 69716 | shoes | preto | lisa | alta (1,00) | moderno | festa | 12 |
| 3 | 53704 | bag | preto | lisa | alta (1,00) | nenhum | festa | 9 |
| 3 | 55636 | bag | preto | lisa | alta (1,00) | moderno | festa | 8 |
| 3 | 55285 | bag | preto | lisa | alta (1,00) | moderno | dia a dia, festa | 7 |
| 4 | 36 | top | branco | lisa | alta (1,00) | classico | dia a dia, festa | 6 |
| 4 | 43 | top | branco | lisa | alta (1,00) | classico | dia a dia, trabalho, festa | 6 |
| 4 | 269 | top | branco | lisa | alta (1,00) | nenhum | festa | 6 |
| 5 | 90481 | necklace | branco | desconhecido | alta (1,00) | nenhum | festa | 8 |
| 5 | 90685 | necklace | branco | desconhecido | alta (1,00) | classico | festa | 7 |
| 5 | 91242 | necklace | branco | desconhecido | alta (1,00) | nenhum | festa | 6 |
| 6 | 1652 | top | branco | outra | baixa (0,00) | casual, romantico | dia a dia | 7 |
| 6 | 2147 | top | branco | outra | baixa (0,00) | casual | dia a dia, festa | 6 |
| 6 | 589 | top | branco | outra | baixa (0,00) | casual, romantico | dia a dia, festa | 5 |

## Os 10 grupos de maior grau (mesmo limiar)

Para comparar: o grau mede com quantos grupos de outras categorias o grupo combina.

| # | grupo | descrição | peças | grau | articulação? | grupos separados |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 957 | shoes marrom lisa formalidade alta | 661 | 96 | sim | 2 |
| 2 | 953 | shoes bege lisa formalidade alta | 1188 | 84 | sim | 2 |
| 3 | 946 | shoes preto lisa formalidade alta | 3067 | 82 | sim | 8 |
| 4 | 266 | earrings bege estampa? formalidade alta | 795 | 80 | sim | 1 |
| 5 | 19 | bag marrom lisa formalidade alta | 568 | 79 | sim | 2 |
| 6 | 9 | bag bege lisa formalidade alta | 817 | 75 | não | 0 |
| 7 | 0 | bag preto lisa formalidade desconhecido | 1275 | 72 | não | 0 |
| 8 | 97 | bracelet bege estampa? formalidade alta | 589 | 70 | sim | 2 |
| 9 | 10 | bag bege lisa formalidade desconhecido | 352 | 69 | sim | 1 |
| 10 | 1142 | top branco lisa formalidade desconhecido | 731 | 69 | sim | 1 |

Dos 10 grupos de maior grau, 8 são articulações, mas só 1 separa 3 grupos ou mais. O maior grau entre as articulações da tabela acima é 82, contra 96 do grupo de maior grau. Um grupo com muitos vizinhos tem mais chance de ser a única ligação de algum grupo pequeno, mas o grau sozinho não diz quantos grupos dependem dele.

## Pontes (mesmo limiar)

Uma ponte é uma aresta que, se sair, separa o grafo. Há 96 pontes. Separa = grupos do lado menor. Nenhuma ponte separa mais de 1 grupo, então a lista abaixo vem na ordem dos pontos.

| # | grupo A | grupo B | n | pontos | separa |
| --- | --- | --- | --- | --- | --- |
| 1 | 46 bag rosa lisa formalidade alta | 268 earrings rosa estampa? formalidade alta | 6 | 1,705 | 1 |
| 2 | 358 gloves cinza 2 estampa? formalidade alta | 440 hats cinza 2 lisa formalidade desconhecido | 4 | 1,573 | 1 |
| 3 | 278 earrings vermelho estampa? formalidade alta | 934 rings vermelho estampa? formalidade desconhecido | 4 | 1,523 | 1 |
| 4 | 85 bag rosa estampada formalidade desconhecido | 984 shoes rosa lisa formalidade alta | 4 | 1,514 | 1 |
| 5 | 43 bag amarelo lisa formalidade baixa | 231 dress amarelo estampada formalidade baixa | 4 | 1,498 | 1 |
| 6 | 410 hairwear vermelho estampa? formalidade desconhecido | 1120 top branco lisa formalidade baixa | 5 | 1,448 | 1 |
| 7 | 10 bag bege lisa formalidade desconhecido | 690 neckwear bege estampada formalidade baixa | 4 | 1,445 | 1 |
| 8 | 348 eyewear amarelo estampa? formalidade desconhecido | 1143 top amarelo lisa formalidade baixa | 4 | 1,422 | 1 |
| 9 | 19 bag marrom lisa formalidade alta | 196 dress marrom estampada formalidade desconhecido | 5 | 1,390 | 1 |
| 10 | 957 shoes marrom lisa formalidade alta | 1088 skirt vinho lisa formalidade baixa | 5 | 1,377 | 1 |

## Nos outros limiares

1º quartil (pontos > 0,223), as 10 primeiras articulações:

| # | grupo | descrição | grau | grupos separados |
| --- | --- | --- | --- | --- |
| 1 | 946 | shoes preto lisa formalidade alta | 148 | 9 |
| 2 | 1120 | top branco lisa formalidade baixa | 140 | 9 |
| 3 | 4 | bag preto lisa formalidade alta | 139 | 8 |
| 4 | 1125 | top branco lisa formalidade alta | 153 | 3 |
| 5 | 603 | necklace branco estampa? formalidade alta | 133 | 3 |
| 6 | 1127 | top branco estampada formalidade baixa | 93 | 3 |

89 pontes, 0 com 3 ou mais grupos do lado menor.

3º quartil (pontos > 0,705), as 10 primeiras articulações:

| # | grupo | descrição | grau | grupos separados |
| --- | --- | --- | --- | --- |
| 1 | 1120 | top branco lisa formalidade baixa | 22 | 10 |
| 2 | 4 | bag preto lisa formalidade alta | 25 | 5 |
| 3 | 97 | bracelet bege estampa? formalidade alta | 31 | 4 |
| 4 | 19 | bag marrom lisa formalidade alta | 44 | 3 |
| 5 | 12 | bag vermelho lisa formalidade alta | 38 | 3 |
| 6 | 953 | shoes bege lisa formalidade alta | 35 | 3 |
| 7 | 39 | bag amarelo lisa formalidade desconhecido | 18 | 3 |

108 pontes, 0 com 3 ou mais grupos do lado menor.

## Gephi

`gephi_articulacoes/q1_*`, `q2_*` e `q3_*`, um par por limiar. Nos nós, `articulacao` = 1 marca toda articulação do Tarjan, `grupos_separados` diz quantos grupos se soltam e `destaque` = 1 marca as que soltam 3 ou mais. Nas arestas, `Weight` são os pontos, `n` a contagem e `ponte` = 1 marca as pontes. O Id do nó é o número do grupo usado nas tabelas.
