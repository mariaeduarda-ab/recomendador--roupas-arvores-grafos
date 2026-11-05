# Comparação das fontes de atributo

Gerado por `preprocess/comparar_fontes.py`. A pergunta é qual fonte de cada atributo é boa o bastante para entrar no sistema.

## Regras da comparação

1. Peças com `categoria_errada=sim` ficam fora de toda conta. A lista está abaixo.
2. Valores "não se aplica" e "incerto" na referência não contam.
3. Todas as fontes são comparadas nas mesmas peças: a metade de teste. As frases do CLIP foram ajustadas na metade de ajuste. As fontes só de texto também aparecem nas 400 peças, numa tabela separada.
4. Cada fonte tem dois números. "Acerto total" conta a falta de resposta ("desconhecido") como erro. "Acerto quando responde" conta só as peças com resposta. A regra dos 70% usa o acerto total, porque o atributo precisa servir para todas as peças do sistema.
5. Todo acerto vem com o intervalo de Wilson de 95% e com a contagem.
6. Valores do texto: estampa lisa, listrada, xadrez, floral, poa vira poá, animal vira animal print, outra vira outra estampa. Comprimento medio vira médio. Modelagem fica igual. Estilo e ocasião no texto podem ter vários valores marcados. Se o texto marca um só, acerta quando bate. Se marca vários, acerta quando a referência está entre eles (versão solta). A versão estrita, em que vários valores contam como erro, está numa tabela à parte. Formalidade no texto é um número de 0 a 1. Dois cortes, escolhidos só na metade de ajuste, viram baixa, média e alta. O valor 0,5 sem nenhuma palavra conta como desconhecido.
7. Fontes: (a) texto; (b) CLIP; (c) combinação, que usa o texto quando ele achou palavra e o CLIP nos outros casos; (d) só na estampa, texto+lisa, que usa o texto e dá "lisa" quando não há palavra de estampa.
8. Um atributo entra só se a melhor fonte chega a 70% de acerto total na metade de teste.
9. A formalidade do CLIP é "não se aplica" em joias e acessórios por decisão do ajuste. No acerto total isso conta como desconhecido.

## Dados

- Referência: 400 peças rotuladas pela imagem. Válidas: 389. Ajuste: 202. Teste: 187.
- Texto: `pecas_v2.csv` gerado com as listas atuais de `palavras_chave.py` em `/tmp/claude-1000/-home-dudaab/5f6e1f0e-9ae6-4f4a-8fd7-c9b3e003cf11/scratchpad/texto_atual`.
- CLIP: `dados/atributos_clip.csv`. Peças válidas sem linha do CLIP: 0 (contam como desconhecido).
- Formalidade: o JSON foi lido para separar o empate (0,5 com palavra) da falta de palavra. Empates entre as válidas: 24.

### Categoria errada na base

11 peças mostram na imagem outra coisa que a categoria da base. Elas ficam fora das contas. Isso é um problema da base, não das fontes.

| id | categoria | metade | o que a imagem mostra |
| --- | --- | --- | --- |
| 14616 | outerwear | ajuste | parece camisa |
| 15499 | dress | ajuste | blusa cortada no quadril |
| 40868 | brooch | teste | par de brincos |
| 45964 | skirt | teste | short de cintura alta |
| 48462 | brooch | ajuste | parece anel |
| 49056 | top | teste | tiara de cristais |
| 51528 | rings | ajuste | pilha de pulseiras |
| 51757 | brooch | teste | parece anel |
| 65293 | necklace | ajuste | bolsa em forma de frasco |
| 71354 | top | teste | tenis de cano alto |
| 78608 | top | ajuste | tenis branco de cano alto |

## Cortes da formalidade do texto

Escolhidos na metade de ajuste, com 177 peças que têm palavra: baixa abaixo de 0,833, nenhum valor vira alta, média no resto. No ajuste esses cortes acertam 106 de 177.

Valor do texto contra o rótulo, na metade de ajuste (só peças com palavra):

| valor do texto | baixa | média | alta |
| --- | --- | --- | --- |
| 0,000 | 51 | 10 | 1 |
| 0,200 | 1 | 0 | 0 |
| 0,333 | 1 | 0 | 0 |
| 0,500 | 6 | 1 | 1 |
| 0,667 | 1 | 0 | 0 |
| 1,000 | 45 | 46 | 13 |

## Resultado na metade de teste

Acerto total, com intervalo de 95% e contagem, e a parte de desconhecido de cada fonte. Em estilo e ocasião vale a versão solta.

| atributo | n | texto | CLIP | combinação | texto+lisa | % desc. texto | % desc. CLIP | % desc. combinação | % desc. texto+lisa |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| estampa | 126 | 13,5% (8,6 a 20,5; 17/126) | 69,8% (61,3 a 77,2; 88/126) | 68,3% (59,7 a 75,7; 86/126) | 77,0% (68,9 a 83,5; 97/126) | 79,4% | 0,0% | 0,0% | 0,0% |
| comprimento | 78 | 23,1% (15,1 a 33,6; 18/78) | 64,1% (53,0 a 73,9; 50/78) | 65,4% (54,3 a 75,0; 51/78) | - | 73,1% | 0,0% | 0,0% | - |
| modelagem | 91 | 3,3% (1,1 a 9,2; 3/91) | 30,8% (22,2 a 40,9; 28/91) | 29,7% (21,3 a 39,7; 27/91) | - | 86,8% | 0,0% | 0,0% | - |
| estilo | 187 | 31,6% (25,3 a 38,5; 59/187) | 44,9% (38,0 a 52,1; 84/187) | 51,3% (44,2 a 58,4; 96/187) | - | 42,2% | 0,0% | 0,0% | - |
| ocasiao | 187 | 60,4% (53,3 a 67,2; 113/187) | 57,8% (50,6 a 64,6; 108/187) | 64,7% (57,6 a 71,2; 121/187) | - | 5,9% | 0,0% | 0,0% | - |
| formalidade | 187 | 49,7% (42,6 a 56,8; 93/187) | 27,3% (21,4 a 34,1; 51/187) | 56,1% (49,0 a 63,1; 105/187) | - | 16,6% | 32,6% | 3,7% | - |

### Acerto quando responde (teste)

| atributo | texto | CLIP | combinação | texto+lisa |
| --- | --- | --- | --- | --- |
| estampa | 65,4% (46,2 a 80,6; 17/26) | 69,8% (61,3 a 77,2; 88/126) | 68,3% (59,7 a 75,7; 86/126) | 77,0% (68,9 a 83,5; 97/126) |
| comprimento | 85,7% (65,4 a 95,0; 18/21) | 64,1% (53,0 a 73,9; 50/78) | 65,4% (54,3 a 75,0; 51/78) | - |
| modelagem | 25,0% (8,9 a 53,2; 3/12) | 30,8% (22,2 a 40,9; 28/91) | 29,7% (21,3 a 39,7; 27/91) | - |
| estilo | 54,6% (45,2 a 63,7; 59/108) | 44,9% (38,0 a 52,1; 84/187) | 51,3% (44,2 a 58,4; 96/187) | - |
| ocasiao | 64,2% (56,9 a 70,9; 113/176) | 57,8% (50,6 a 64,6; 108/187) | 64,7% (57,6 a 71,2; 121/187) | - |
| formalidade | 59,6% (51,8 a 67,0; 93/156) | 40,5% (32,3 a 49,2; 51/126) | 58,3% (51,0 a 65,3; 105/180) | - |

### Estilo e ocasião: versão solta e estrita (teste)

| atributo | fonte | acerto total (solto) | acerto total (estrito) | peças com vários valores |
| --- | --- | --- | --- | --- |
| estilo | texto | 31,6% (25,3 a 38,5; 59/187) | 22,5% (17,1 a 29,0; 42/187) | 28 de 187 (15,0%) |
| estilo | combinação | 51,3% (44,2 a 58,4; 96/187) | 42,2% (35,4 a 49,4; 79/187) | 28 de 187 (15,0%) |
| ocasiao | texto | 60,4% (53,3 a 67,2; 113/187) | 29,9% (23,8 a 36,9; 56/187) | 66 de 187 (35,3%) |
| ocasiao | combinação | 64,7% (57,6 a 71,2; 121/187) | 34,2% (27,8 a 41,3; 64/187) | 66 de 187 (35,3%) |

## Só texto, nas 400 peças (linha secundária)

As fontes de texto não foram ajustadas na referência, então podem ser medidas nas duas metades. Exceção: os cortes da formalidade saíram do ajuste, e aqui o ajuste entra junto.

| atributo | n | texto | texto+lisa | % desc. texto | % desc. texto+lisa |
| --- | --- | --- | --- | --- | --- |
| estampa | 259 | 13,9% (10,2 a 18,6; 36/259) | 80,3% (75,0 a 84,7; 208/259) | 78,8% | 0,0% |
| comprimento | 156 | 25,0% (18,9 a 32,3; 39/156) | - | 71,2% | - |
| modelagem | 184 | 4,9% (2,6 a 9,0; 9/184) | - | 87,0% | - |
| estilo | 389 | 35,5% (30,9 a 40,4; 138/389) | - | 40,6% | - |
| ocasiao | 389 | 59,6% (54,7 a 64,4; 232/389) | - | 4,4% | - |
| formalidade | 389 | 51,2% (46,2 a 56,1; 199/389) | - | 14,4% | - |

| atributo | texto | texto+lisa |
| --- | --- | --- |
| estampa | 65,5% (52,3 a 76,6; 36/55) | 80,3% (75,0 a 84,7; 208/259) |
| comprimento | 86,7% (73,8 a 93,7; 39/45) | - |
| modelagem | 37,5% (21,2 a 57,3; 9/24) | - |
| estilo | 59,7% (53,3 a 65,9; 138/231) | - |
| ocasiao | 62,4% (57,3 a 67,1; 232/372) | - |
| formalidade | 59,8% (54,4 a 64,9; 199/333) | - |

## Decisão

| atributo | melhor fonte | acerto total no teste | entra? |
| --- | --- | --- | --- |
| estampa | texto+lisa | 77,0% (68,9 a 83,5; 97/126) | sim |
| comprimento | combinação | 65,4% (54,3 a 75,0; 51/78) | não |
| modelagem | CLIP | 30,8% (22,2 a 40,9; 28/91) | não |
| estilo | combinação | 51,3% (44,2 a 58,4; 96/187) | não |
| ocasiao | combinação | 64,7% (57,6 a 71,2; 121/187) | não |
| formalidade | combinação | 56,1% (49,0 a 63,1; 105/187) | não |

- **estampa** entra com a fonte texto+lisa. O limite de baixo do intervalo (68,9%) fica abaixo de 70%, então a margem é pequena.
- **comprimento** fica fora. A melhor fonte (combinação) acerta 65,4% no total, abaixo de 70%. Vai para o artigo como limitação.
- **modelagem** fica fora. A melhor fonte (CLIP) acerta 30,8% no total, abaixo de 70%. Vai para o artigo como limitação.
- **estilo** fica fora. A melhor fonte (combinação) acerta 51,3% no total, abaixo de 70%. Vai para o artigo como limitação.
- **ocasiao** fica fora. A melhor fonte (combinação) acerta 64,7% no total, abaixo de 70%. Vai para o artigo como limitação.
- **formalidade** fica fora. A melhor fonte (combinação) acerta 56,1% no total, abaixo de 70%. Vai para o artigo como limitação.

Na formalidade, o 0,5 de empate não conta como desconhecido. Na tabela, isso só muda o texto e a combinação.

### Efeito na versão D

O vetor de busca da versão D é o grupo 1: 5 colunas de estilo, 5 de ocasião e a formalidade. Ficam fora: estilo, ocasiao, formalidade. No arquivo final, estilo e ocasião fora ficam com todas as colunas em 0 e a formalidade fora fica em 0,5. O código da D já lê esses valores.

Com os três fora, o vetor de busca fica igual para todas as peças. A KD-tree deixa de separar peças e a parte "a" da nota (proximidade) vale 1 para todo par. A D passa a depender só da cor, da estampa e do bônus dos looks. Isso precisa ser decidido antes de rodar a D com o arquivo final.

## Revisão manual (50 peças)

Pendente. O arquivo `revisao/revisao_50.csv` ainda não existe. Quando a revisão for exportada, basta rodar o script de novo. Ele mede a concordância dos rótulos dos agentes com a revisão e compara a formalidade do texto e do CLIP com as 50 peças.
