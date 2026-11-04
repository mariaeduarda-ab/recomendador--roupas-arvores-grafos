# Ajuste das frases do CLIP

Este texto mostra como as frases do `preprocess/atributos_clip.py` foram ajustadas e quanto o CLIP acerta depois disso.

## O que mudou no programa

O programa não abre mais as imagens. Ele lê os vetores de imagem que o `clip_vetores.py` salvou em arquivos `.npz`. Assim, uma troca de frase não exige passar as fotos pelo modelo de novo. O catálogo de 32.535 peças roda em cerca de 13 s. Quase todo esse tempo vai para os vetores das frases.

## Protocolo

- Referência: as 400 peças rotuladas por agentes que viram só a imagem (`referencia_300.csv` e `referencia_extra_100.csv`).
- Divisão: `dados/referencia/metades.csv`. Metade de ajuste: 202 peças. Metade de teste: 187 peças. As peças com `categoria_errada=sim` ficam fora (6 no ajuste, 5 no teste).
- Valores "incerto" e "não se aplica" na referência não contam.
- Acerto: o valor do CLIP é igual ao valor da referência.
- Todas as escolhas usaram só a metade de ajuste. Os scripts de análise carregavam só os rótulos do ajuste.
- A metade de teste foi medida uma vez só, depois das frases e da correção estarem fechadas.
- A correção de viés (abaixo) usa fotos sem rótulo: as 500 da amostra de velocidade e as 202 fotos da metade de ajuste. São 702 fotos. Nenhuma foto do teste entra nessa conta.

## Mudanças, com o acerto na metade de ajuste

Cada linha soma uma mudança à linha de cima. Na última linha, a formalidade conta só as categorias que ainda recebem valor.

| Passo | estampa | comprimento | modelagem | estilo | ocasião | formalidade |
|---|---|---|---|---|---|---|
| 0. Frases originais | 98/133 = 0,737 | 36/78 = 0,462 | 27/93 = 0,290 | 78/202 = 0,386 | 113/202 = 0,559 | 70/202 = 0,347 |
| 1. Novas frases de estilo | 0,737 | 0,462 | 0,290 | 90/202 = 0,446 | 0,559 | 0,347 |
| 2. Correção geral no estilo | 0,737 | 0,462 | 0,290 | 94/202 = 0,465 | 0,559 | 0,347 |
| 3. Correção por categoria em comprimento, modelagem e formalidade | 0,737 | 46/78 = 0,590 | 35/93 = 0,376 | 0,465 | 0,559 | 97/202 = 0,480 |
| 4. Joias e acessórios sem formalidade | 0,737 | 0,590 | 0,376 | 0,465 | 0,559 | 64/133 = 0,481 |

### 1. Novas frases de estilo

As frases de "moderno" eram "modern trendy" e "edgy fashion forward". Essas palavras combinam com quase qualquer foto de moda. Por isso "moderno" ganhava quase sempre. As novas frases de moderno falam de desenho ousado ("modern ... with a bold design", "contemporary designer ..."). Casual, clássico e esportivo ganharam uma frase a mais cada. Romântico ficou igual.

### 2 e 3. Correção de viés

Algumas frases ficam perto de quase toda foto. Elas ganham sem relação com a peça. A correção é esta: para cada frase, calculamos a semelhança média dela com as 702 fotos sem rótulo. Depois, na hora de escolher, essa média sai da semelhança de cada peça. A frase que combina com tudo perde a vantagem.

- Estilo: a média usa todas as 702 fotos. O estilo muda de uma categoria para outra (tênis tende a esportivo, por exemplo). A média só da mesma categoria apagava essa diferença: com ela o estilo caiu para 0,356.
- Comprimento, modelagem e formalidade: a média usa só as fotos da mesma categoria. As frases desses atributos falam daquela peça. Com a média de todas as fotos, não houve ganho (comprimento 0,474, modelagem 0,280, formalidade 0,351).
- Estampa e ocasião: sem correção. Nesses dois atributos um valor é o normal (lisa, dia a dia). A correção empurra a escolha para valores raros. Com a correção por categoria, a estampa caiu de 0,737 para 0,383 e a ocasião de 0,559 para 0,262.

Com a correção, "moderno" deixou de ganhar quase tudo. Nos sapatos da amostra de 900 peças, moderno caiu de 74 de 92 para 35 de 92.

### Tentativas que não ficaram

- Frases novas para "trabalho" na ocasião ("business ... to wear to the office" e outras). O CLIP passou a dar "trabalho" a peças de dia a dia, e o acerto caiu de 0,559 para 0,475. As peças rotuladas como trabalho continuaram com outro valor. As frases originais ficaram.
- Frases de formalidade iguais à regra dos rótulos (baixa casual, média escritório, alta festa). Sem correção, o acerto subiu de 0,347 para 0,416. Com a correção, as frases originais ficaram melhores (0,481 contra 0,459 fora das joias). Ficaram as originais.
- Frases novas para comprimento e modelagem (calça capri, macacão inteiro, cropped acima da cintura, corte regular). Sozinhas deram ganho pequeno. Com a correção, não somaram nada. Ficaram as originais.

## Formalidade em joias e acessórios

Categorias: brinco, colar, pulseira, anel, broche, óculos, relógio, enfeite de cabelo e luva. São 69 peças com formalidade rotulada no ajuste.

| Na metade de ajuste | Acerto |
|---|---|
| Frases originais, sem correção | 18/69 = 0,261 (deu "alta" em 47 de 69) |
| Com a correção por categoria | 33/69 = 0,478 |
| Chutar sempre "média" | 31/69 = 0,449 |

Rótulos dessas 69 peças: média 31, baixa 30, alta 8.

Por categoria, com a correção: pulseira 2/8, anel 2/7, brinco 3/8, colar 3/7, enfeite de cabelo 4/8, relógio 4/8, óculos 5/8, luva 5/8, broche 5/7. São de 7 a 8 peças por categoria, então esses números variam muito.

Mesmo com a correção, o acerto fica abaixo de 50% e quase igual ao chute do valor mais comum. Decisão: nessas 9 categorias o `atributos_clip.py` escreve "não se aplica" na formalidade. A lista fica no próprio programa (`SEM_FORMALIDADE`). O `aplicacao.py` não mudou, porque o validador da referência depende dele.

## Resultado na metade de teste

Medido uma vez, com tudo fechado. O intervalo é o de Wilson com 95% de confiança. A formalidade conta só as categorias que recebem valor (fora as 9 de joias e acessórios). Para comparar, a tabela mostra também as frases originais e o chute do valor mais comum na metade de ajuste.

| Atributo | n | Acerto final | Intervalo 95% | Frases originais | Chute do mais comum |
|---|---|---|---|---|---|
| estampa | 126 | 88 = 0,698 | 0,613 a 0,772 | 0,698 | 0,683 (lisa) |
| comprimento | 78 | 50 = 0,641 | 0,530 a 0,739 | 0,397 | 0,513 (curto) |
| modelagem | 91 | 28 = 0,308 | 0,222 a 0,409 | 0,297 | 0,451 (reta) |
| estilo | 187 | 84 = 0,449 | 0,380 a 0,521 | 0,390 | 0,235 (moderno) |
| ocasião | 187 | 108 = 0,578 | 0,506 a 0,646 | 0,578 | 0,588 (dia a dia) |
| formalidade | 126 | 51 = 0,405 | 0,323 a 0,492 | 0,492 | 0,532 (baixa) |

Nas joias e acessórios do teste, as frases originais acertaram a formalidade em 14 de 61 peças (0,230). Chutar "média" acertaria 26 de 61. Isso confirma a decisão de deixar essas categorias sem formalidade.

## Leitura dos números

- Comprimento e estilo melhoraram no teste. O ganho do comprimento (0,397 para 0,641) é o maior.
- Estampa e ocasião não mudaram, porque as frases e a regra delas não mudaram.
- A modelagem fica abaixo do chute "reta". O CLIP ainda dá "cropped" a muitas peças que não são cropped.
- A formalidade fora das joias piorou no teste (0,492 para 0,405), apesar do ganho no ajuste (0,391 para 0,481). Com cerca de 130 peças por metade, uma diferença de 9 pontos pode vir do acaso. Esse resultado mostra o limite de ajustar com poucas peças.
- A ocasião "trabalho" quase nunca sai. O CLIP não separa roupa de trabalho de roupa do dia a dia nestas fotos.
- Os intervalos são largos. Com menos de 200 peças por atributo, diferenças de poucos pontos entre versões não são confiáveis.
