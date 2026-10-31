# Diagnóstico da versão atual

Base: `dados/pecas.csv` e `dados/looks.csv`. Mesma divisão e mesma semente (42) de `./recomendador avaliar`. São 102.370 peças, 25.066 looks de treino e 6.267 looks de teste. Destes, 5.842 têm 2 peças ou mais e entram no teste.

## 1. O 27% é Hit@1, Hit@5 ou Hit@10?

Nenhum dos três. **O 27% não é acerto.** Em 1.600 dos 5.842 looks de teste (27,4%), a peça escondida aparece em algum look de treino. Isso é um teto teórico para as versões B e C.

O acerto real é bem menor. A versão C tem Hit@10 de 0,63%. A frase "27% de acerto" não deve ir para o artigo.

## 2. Quantas peças candidatas entram em cada teste?

Depende da versão.

- Popularidade e versão A: todas as peças da categoria pedida. A média é 10.940 por teste. A mediana é 16.033. O mínimo é 169 (legwear) e o máximo é 17.052 (bag).
- Versão B: em média 0,9 candidata por teste. Em 3.651 testes (62,5%) a lista fica vazia.
- Versão C: em média 8,2 candidatas por teste. Em 267 testes (4,6%) a lista fica vazia.

## 3. Resultado de cada versão no mesmo teste

| Versão | Hit@1 | Hit@5 | Hit@10 | Cobertura |
| --- | --- | --- | --- | --- |
| Aleatória (valor esperado) | 0,018% | 0,091% | 0,182% | 100% |
| Popularidade | 0,171% | 0,565% | 0,873% | 100% |
| A. Só semelhança | 0,051% | 0,188% | 0,514% | 100% |
| B. Só grafo | 0,377% | 0,582% | 0,599% | 0,599% |
| C. Grafo + semelhança | 0,359% | 0,599% | 0,633% | 0,650% |

Em quantidade de looks, de 5.842:

| Versão | Hit@1 | Hit@5 | Hit@10 | Cobertura |
| --- | --- | --- | --- | --- |
| Popularidade | 10 | 33 | 51 | 5.842 |
| A | 3 | 11 | 30 | 5.842 |
| B | 22 | 34 | 35 | 35 |
| C | 21 | 35 | 37 | 38 |

A aleatória usa o valor esperado, min(k, N) / N, com N igual ao número de candidatas. Uma simulação deu 1, 3 e 10 acertos, que combina com o esperado.

As diferenças entre B e C são de 1 ou 2 looks. Isso é pequeno demais para dizer que uma é melhor que a outra.

## 4. Cobertura

- Popularidade e A: 100%, porque a lista tem a categoria inteira.
- B: 0,599% (35 looks).
- C: 0,650% (38 looks).

**Este é o achado principal.** O teto de 27% nunca chega perto de ser usado. A peça escondida está no treino em 1.600 looks, mas B e C só alcançam 38. Estar no treino não basta. A peça precisa dividir um look de treino com uma peça de partida.

Quando B e C alcançam a peça, quase sempre ela já fica no topo 10. Por isso o problema é alcançar a peça, não ordenar a lista.

## 5. Peças em poucos looks

- Peças em só 1 look, contando treino e teste: 87.617 de 102.370 (85,6%).
- Peças em exatamente 1 look de treino: 73.650 (71,9%).
- Peças que aparecem em looks de teste: 23.724.
- Dessas, sem nenhum look de treino: 17.840 (75,2%).
- Peças escondidas sem nenhum look de treino: 4.242 de 5.842 (72,6%).

## 6. Vazamento de dados

Não achei vazamento no grafo.

- O grafo tem 101.408 arestas, igual ao número de linhas de treino. A contagem bate peça por peça.
- Nenhum look tem linhas de treino e de teste ao mesmo tempo (0 de 31.333).
- B nunca devolveu como candidata uma peça escondida sem look de treino (0 casos).
- O filtro de categoria está certo. Nenhuma candidata veio de outra categoria (0 casos).
- Nas versões A, B e C, nenhuma candidata já estava no look (0 casos).

Pontos pequenos:

- 1 look de teste tem as mesmas peças de um look de treino. Isso vem da junção de peças repetidas. O efeito é no máximo 1 look em 5.842.
- `Recomendador::popularidade` não retira as peças do look, ao contrário do que diz `recomendador.hpp`. A lista média é 0,1 peça maior que a da versão A. A escondida nunca é retirada, então não infla o acerto. Uma peça do look pode ocupar uma vaga do topo.

## Erro que explique o 27%

Não existe erro de código por trás do 27%. O número foi lido como acerto, mas é o teto de B e C.
