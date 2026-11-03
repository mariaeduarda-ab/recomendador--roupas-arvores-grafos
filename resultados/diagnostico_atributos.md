# Diagnóstico dos atributos tirados do texto

Etapa 1. A pergunta é se o "desconhecido" em `pecas_v2.csv` é tão grande quanto parece.

A base são as 102.370 peças de `Combined_Metadata.json`, carregadas com `preprocessar.carregar_pecas`.
Os números "antes" vêm de `dados/pecas_v2.csv`. Os números "depois" vêm de uma cópia gerada em pasta temporária com as listas novas.
Nada em `dados/*.csv` nem em `resultados/amostra_atributos.md` foi alterado.

## 1. Onde cada atributo se aplica

A regra é a de `preprocess/aplicacao.py`. Estilo, ocasião e formalidade valem para todas as categorias.
"n.a." quer dizer que o atributo não se aplica.

| Categoria | Peças | estilo | ocasiao | formalidade | estampa | comprimento | modelagem |
| --- | --- | --- | --- | --- | --- | --- | --- |
| bag | 17.052 | sim | sim | sim | sim | n.a. | n.a. |
| top | 16.717 | sim | sim | sim | sim | n.a. | sim |
| shoes | 16.033 | sim | sim | sim | sim | n.a. | n.a. |
| outerwear | 8.116 | sim | sim | sim | sim | sim | sim |
| pants | 6.632 | sim | sim | sim | sim | sim | sim |
| dress | 6.165 | sim | sim | sim | sim | sim | sim |
| skirt | 4.567 | sim | sim | sim | sim | sim | sim |
| earrings | 4.502 | sim | sim | sim | n.a. | n.a. | n.a. |
| necklace | 4.449 | sim | sim | sim | n.a. | n.a. | n.a. |
| eyewear | 4.201 | sim | sim | sim | n.a. | n.a. | n.a. |
| bracelet | 3.700 | sim | sim | sim | n.a. | n.a. | n.a. |
| rings | 2.885 | sim | sim | sim | n.a. | n.a. | n.a. |
| hats | 2.322 | sim | sim | sim | sim | n.a. | n.a. |
| watch | 1.708 | sim | sim | sim | n.a. | n.a. | n.a. |
| neckwear | 1.329 | sim | sim | sim | sim | n.a. | n.a. |
| brooch | 832 | sim | sim | sim | n.a. | n.a. | n.a. |
| hairwear | 461 | sim | sim | sim | n.a. | n.a. | n.a. |
| gloves | 344 | sim | sim | sim | n.a. | n.a. | n.a. |
| jumpsuit | 186 | sim | sim | sim | sim | sim | sim |
| legwear | 169 | sim | sim | sim | sim | n.a. | n.a. |

O atributo se aplica a 79.288 peças em estampa, 25.666 em comprimento e 42.383 em modelagem.

Sobre `aplicacao.py`, que não foi alterado:

- A regra parece correta. Não achei categoria fora do lugar.
- O `atributos.py` ainda calcula estampa para todas as categorias e comprimento para `top`. Por isso o relatório antigo mistura peças em que o atributo não se aplica. O CSV não foi mudado porque a versão D depende dele.
- Em `outerwear` o comprimento continua 97,4% desconhecido. O texto quase nunca fala do comprimento de casaco. Esse valor só deve vir da imagem.

## 2. Desconhecido só onde o atributo se aplica

### Geral

| Atributo | Relatório antigo | Recalculado (antes) | Depois das listas novas |
| --- | --- | --- | --- |
| estilo | 38,4% | 38,4% | 37,7% |
| ocasiao | 4,0% | 4,0% | 4,0% |
| formalidade | 22,4% | 22,4% | 22,4% |
| estampa | 85,7% | 83,7% | 81,9% |
| comprimento | 89,1% | 82,6% | 64,6% |
| modelagem | 92,1% | 92,1% | 87,7% |

O relatório antigo dividia a estampa por todas as 102.370 peças e o comprimento por todas as roupas, com `top`.
Com a regra de aplicação, a queda é pequena em estampa (2 pontos) e maior em comprimento (6,5 pontos).
O problema continua grande nos três atributos visuais.

### Por categoria, antes das listas novas

| Categoria | estilo | ocasiao | formalidade | estampa | comprimento | modelagem |
| --- | --- | --- | --- | --- | --- | --- |
| bag | 39,3% | 1,8% | 27,8% | 91,5% | n.a. | n.a. |
| top | 17,6% | 3,3% | 18,3% | 71,4% | n.a. | 94,6% |
| shoes | 52,2% | 2,5% | 22,9% | 93,8% | n.a. | n.a. |
| outerwear | 23,8% | 4,5% | 26,1% | 88,6% | 97,4% | 98,4% |
| pants | 8,9% | 6,1% | 15,5% | 89,1% | 68,8% | 94,5% |
| dress | 44,2% | 3,9% | 19,7% | 71,5% | 84,8% | 83,7% |
| skirt | 34,4% | 6,2% | 31,6% | 67,2% | 72,4% | 79,9% |
| earrings | 66,0% | 5,6% | 15,4% | n.a. | n.a. | n.a. |
| necklace | 55,9% | 3,7% | 22,3% | n.a. | n.a. | n.a. |
| eyewear | 37,3% | 7,6% | 31,2% | n.a. | n.a. | n.a. |
| bracelet | 55,8% | 3,6% | 22,3% | n.a. | n.a. | n.a. |
| rings | 62,1% | 4,1% | 14,3% | n.a. | n.a. | n.a. |
| hats | 35,8% | 8,1% | 23,3% | 92,3% | n.a. | n.a. |
| watch | 50,6% | 10,9% | 14,4% | n.a. | n.a. | n.a. |
| neckwear | 55,6% | 6,0% | 18,7% | 64,9% | n.a. | n.a. |
| brooch | 67,3% | 8,7% | 13,5% | n.a. | n.a. | n.a. |
| hairwear | 55,3% | 6,9% | 20,2% | n.a. | n.a. | n.a. |
| gloves | 63,4% | 4,9% | 9,3% | n.a. | n.a. | n.a. |
| jumpsuit | 30,1% | 5,4% | 29,6% | 74,7% | 99,5% | 87,6% |
| legwear | 23,7% | 4,1% | 25,4% | 68,0% | n.a. | n.a. |

### Por categoria, depois das listas novas

| Categoria | estilo | ocasiao | formalidade | estampa | comprimento | modelagem |
| --- | --- | --- | --- | --- | --- | --- |
| bag | 38,6% | 1,8% | 27,8% | 88,7% | n.a. | n.a. |
| top | 17,3% | 3,3% | 18,3% | 68,8% | n.a. | 91,4% |
| shoes | 51,8% | 2,5% | 22,9% | 92,8% | n.a. | n.a. |
| outerwear | 23,0% | 4,5% | 26,1% | 87,7% | 97,4% | 97,7% |
| pants | 8,6% | 6,1% | 15,5% | 88,4% | 1,6% | 89,0% |
| dress | 43,6% | 3,9% | 19,7% | 69,4% | 83,1% | 74,2% |
| skirt | 33,4% | 6,2% | 31,6% | 65,9% | 71,6% | 73,4% |
| earrings | 64,6% | 5,6% | 15,4% | n.a. | n.a. | n.a. |
| necklace | 54,7% | 3,7% | 22,3% | n.a. | n.a. | n.a. |
| eyewear | 36,9% | 7,6% | 31,2% | n.a. | n.a. | n.a. |
| bracelet | 54,3% | 3,6% | 22,3% | n.a. | n.a. | n.a. |
| rings | 61,6% | 4,1% | 14,3% | n.a. | n.a. | n.a. |
| hats | 35,0% | 8,1% | 23,3% | 89,5% | n.a. | n.a. |
| watch | 50,5% | 10,9% | 14,4% | n.a. | n.a. | n.a. |
| neckwear | 54,2% | 6,0% | 18,7% | 62,5% | n.a. | n.a. |
| brooch | 66,2% | 8,7% | 13,5% | n.a. | n.a. | n.a. |
| hairwear | 55,1% | 6,9% | 20,2% | n.a. | n.a. | n.a. |
| gloves | 63,1% | 4,9% | 9,3% | n.a. | n.a. | n.a. |
| jumpsuit | 29,6% | 5,4% | 29,6% | 74,7% | 99,5% | 81,7% |
| legwear | 23,7% | 4,1% | 25,4% | 63,3% | n.a. | n.a. |

## 3. Estampa: 30 imagens conferidas a olho

Sorteio com semente 42. Os candidatos são os ids de `dados/amostra_clip_500.csv` com estampa "desconhecido", categoria em que a estampa se aplica e imagem em `dados/imagens`. Foram 329 candidatos. Nenhum id de `dados/referencia/amostra_300.csv` entrou (as duas listas não têm id em comum).

| id | categoria | o que a imagem mostra | classe | variacao_cor | estampa nova |
| --- | --- | --- | --- | --- | --- |
| 102246 | dress | trench rosa transparente com faixa | lisa | 0,135 | desconhecido |
| 15924 | skirt | saia em xadrez vichy marrom e creme | estampada (xadrez) | 0,199 | desconhecido |
| 3558 | top | regata amarela | lisa | 0,255 | desconhecido |
| 53136 | bag | bolsa saco caramelo com franjas | lisa | 0,171 | desconhecido |
| 44080 | pants | jeans skinny azul com rasgos | lisa | 0,260 | desconhecido |
| 41729 | pants | calça skinny preta | lisa | 0,458 | desconhecido |
| 27615 | dress | vestido vinho de malha canelada | lisa | 0,436 | desconhecido |
| 15010 | top | regata creme com babados na barra | lisa | 0,225 | desconhecido |
| 83379 | shoes | scarpin preto | lisa | 0,497 | desconhecido |
| 12252 | top | blusa roxa transpassada com botões | lisa | 0,363 | desconhecido |
| 97449 | outerwear | casaco rosa com gola de pelo | lisa | 0,192 | desconhecido |
| 70698 | shoes | tênis preto de camurça | lisa | 0,491 | desconhecido |
| 5020 | top | camiseta branca com frase azul | estampada (texto) | 0,411 | outra |
| 4728 | top | camiseta bege com paetês | textura (paetê) | 0,089 | desconhecido |
| 13590 | top | top branco com correntes | lisa | 0,235 | desconhecido |
| 41318 | pants | legging verde oliva com botões | lisa | 0,269 | desconhecido |
| 43051 | pants | jeans azul escuro | lisa | 0,393 | desconhecido |
| 79306 | shoes | bota preta de salto | lisa | 0,403 | desconhecido |
| 98355 | dress | vestido preto sem manga com faixa | lisa | 0,430 | desconhecido |
| 3810 | top | camiseta branca | lisa | 0,341 | desconhecido |
| 90303 | jumpsuit | macacão de tweed xadrez cinza | estampada (xadrez) | 0,223 | desconhecido |
| 34864 | hats | chapéu coco preto | lisa | 0,445 | desconhecido |
| 70236 | shoes | tênis branco com três listras pretas da marca | lisa | 0,180 | desconhecido |
| 41582 | pants | jeans azul claro | lisa | 0,283 | desconhecido |
| 72745 | shoes | oxford preto | lisa | 0,340 | desconhecido |
| 96900 | outerwear | colete de camurça com pelo | lisa | 0,118 | desconhecido |
| 53440 | bag | bolsa caramelo | lisa | 0,194 | desconhecido |
| 774 | top | regata preta com lua e estrelas | estampada (gráfica) | 0,330 | outra |
| 29237 | dress | vestido branco de renda | textura (renda) | 0,033 | desconhecido |
| 59820 | bag | bolsa branca matelassê | lisa | 0,084 | desconhecido |

- 24 de 30 são lisas (80%). Com uma amostra de 30, o intervalo de 95% vai de cerca de 63% a 91%.
- 2 têm textura sem estampa (paetê e renda).
- 4 são estampadas: 2 xadrez, 1 com texto, 1 gráfica.
- As listas novas acharam as 2 estampas de texto e gráfica. Os 2 xadrez seguem sem saída porque o texto não fala de xadrez. A legenda da saia diz só "skirt with a bow". A do macacão diz "gray and white suit".
- O texto também erra. A descrição do vestido 29237 diz "maxi", mas a foto mostra um vestido curto.

O valor "lisa" quase nunca aparece no texto. Na base toda, só 20 peças têm estampa "lisa". O modelo que escreveu as descrições não costuma dizer que a peça é lisa. Então o "desconhecido" de estampa é, na maior parte, peça lisa.

### variacao_cor separa lisa de estampada?

| Grupo | n | variacao_cor |
| --- | --- | --- |
| lisa | 24 | mediana 0,276, de 0,084 a 0,497 |
| estampada | 4 | 0,199; 0,223; 0,330; 0,411 |
| textura | 2 | 0,033; 0,089 |

| Limite | lisas abaixo | estampadas abaixo | texturas abaixo |
| --- | --- | --- | --- |
| 0,15 | 3 de 24 | 0 de 4 | 2 de 2 |
| 0,19 | 5 de 24 | 0 de 4 | 2 de 2 |
| 0,20 | 7 de 24 | 1 de 4 | 2 de 2 |
| 0,25 | 9 de 24 | 2 de 4 | 2 de 2 |
| 0,30 | 13 de 24 | 2 de 4 | 2 de 2 |

As 7 peças lisas pretas têm variacao_cor de 0,340 a 0,497. Nas peças escuras, a diferença para o fundo branco da foto parece entrar na conta. Na base toda, nas categorias em que a estampa se aplica, a mediana é 0,403 para peças com cor dominante preta e 0,246 para as outras.

A base toda confirma que o número não separa as classes. Peças com estampa achada no texto têm mediana 0,310. Peças com estampa "desconhecido" têm mediana 0,307. Das peças com estampa achada, 24,5% ficam abaixo de 0,20.

Um limite possível é variacao_cor abaixo de 0,19 para peça sem estampa. Nas 30 peças ele não erra, mas cobre só 7 delas. Na base toda, cerca de 1 em cada 4 peças estampadas também fica abaixo de 0,20. Com 4 estampadas na amostra, esse limite não tem apoio. Ele só deve ser usado junto com outra pista, e nunca sozinho para dizer que a peça é estampada.

## 4. Palavras novas nas listas

As contagens foram feitas por categoria nas legendas e descrições depois do `limpar`. O `atributos.py` não precisou de mudança.
Vários exemplos do pedido já estavam nas listas: maxi, midi, mini, ankle-length, skinny, wide-leg, bodycon, a-line.

| Lista | Palavras novas |
| --- | --- |
| estilo classico | preppy, vintage, retro |
| estilo esportivo | athleisure |
| estilo romantico | bohemian, boho |
| estilo moderno | streetwear, urban |
| estampa floral | flowers, flowery, roses |
| estampa animal | python, giraffe |
| estampa outra | tie dye, tie-dyed, geometric, abstract, argyle, chevron, ombre, patchwork, monogram, color block, color-block, color-blocked, colorblock, two-tone, motif, motifs, skull, skulls, stars, hearts, slogan, lettering, letters, words |
| comprimento curto | miniskirt, above the knee, above-the-knee |
| comprimento medio | tea-length, mid-length, below the knee, bermuda, culottes |
| comprimento longo | floor length, ankle length, floor-sweeping, jeans, trousers, pants, sweatpants, leggings, chinos |
| modelagem oversized | oversize, slouchy |
| modelagem justa | form-fitting, figure-hugging, body-hugging, close-fitting, pencil, sheath, leggings, bodysuit, mermaid |
| modelagem ampla | flowing, billowy, relaxed fit, relaxed-fit, fit-and-flare, skater, palazzo, culottes |
| modelagem reta | shift |

Algumas palavras foram testadas e ficaram de fora.

- "relaxed" sozinho em modelagem. Na maioria das vezes fala do clima ("relaxed evening"). Só "relaxed fit" entrou.
- "flower" no singular. Quase sempre é um enfeite só, no salto ou na bolsa.
- "gown". Nas fotos, 3 de 4 vestidos chamados de "gown" eram curtos. O modelo usa a palavra para qualquer vestido de festa.
- "monochromatic" e "all-black". O texto usa as duas para o conjunto ou para peça preta e branca.
- "logo", "embroidered", "sequins", "lace", "crocodile". Logo pequeno, bordado, paetê, renda e couro de crocodilo em relevo não são estampa.
- "cow". Vem de "cow neck", gola e não estampa.
- "knees". Vem de "holes on the knees", jeans rasgado.
- "elegant" em estilo. Aparece em 49% de todas as descrições e em 75% das peças sem estilo. Ele baixaria o desconhecido de estilo para perto de 10%, mas marcaria metade da base como clássica. A palavra parece um hábito do modelo e não separa peças.

Há uma regra nova de comprimento. Em calça, a palavra da própria peça ("jeans", "pants", "trousers", "leggings") marca "longo". "shorts", "capri" e "bermuda" vêm antes na ordem e ganham. Essa regra explica quase todo o ganho de comprimento. Ela trouxe 4.457 das 4.605 peças novas. Sem ela, o comprimento ficaria em cerca de 82% desconhecido.

Outros efeitos das listas novas:

- As palavras novas entram também na lista de adjetivos que o `limpar` usa para apagar a citação de outra peça. Isso mudou a formalidade de 1 peça.
- 76 peças que já tinham estampa mudaram de valor. Por exemplo, 44 passaram de "outra" para "floral", porque o texto cita "flowers" e "print". Também 2 peças "lisa" viraram "outra". Nas duas o texto dizia "solid black background" ou "solid white to solid black" de um degradê, e a nova saída está certa.
- 4 peças de comprimento mudaram de valor. Duas saias passaram de "longo" para "curto" porque o texto diz ao mesmo tempo "maxi" e "falls just above the knee". O texto se contradiz e nenhuma das duas saídas é segura.
- 11 peças de modelagem mudaram de valor, 7 de "ampla" para "justa" e 4 de "reta" para "ampla". Nos casos lidos, o texto cita duas modelagens, como "flowy bodysuit" ou "flowy skirt" e "pencil skirt".

### Quantas peças ganharam valor

| Atributo | Peças que saíram de desconhecido |
| --- | --- |
| estampa | 2.764 (2.370 outra, 348 floral, 46 animal) |
| comprimento | 4.605 (4.457 em calça) |
| modelagem | 1.855 (1.084 ampla, 604 justa, 166 reta, 1 oversized) |

## 5. Conferência de falsos positivos

Foram sorteadas 20 peças com valor novo, com semente 42, só em categorias em que o atributo se aplica: 8 de estampa, 6 de comprimento e 6 de modelagem. Cada uma foi conferida pela leitura do texto.

| Atributo | Certas pelo texto | Dúvida ou erro |
| --- | --- | --- |
| estampa | 8 de 8 | nenhuma |
| comprimento | 6 de 6 | nenhuma (as 6 são calça) |
| modelagem | 5 de 6 | 30863: "sheath dress" com "gathered pleated skirt". O texto mistura justa e ampla |

No total, 19 de 20 estão certas pelo texto, cerca de 5% de erro. Com 20 peças, o erro real pode estar entre 0% e 25%.

Também foram feitas outras conferências.

- 5 peças de comprimento fora de calça (vestido e saia) foram lidas. As 5 batem com o texto ("floor-sweeping", "above the knee", "mid-length").
- 8 calças marcadas como "longo" pela regra nova têm imagem. As 8 são compridas na foto.
- 4 vestidos marcados como "longo" por "gown" tinham imagem. Só 1 era longo. Por isso "gown" saiu da lista.

A conferência pelo texto não pega o erro do próprio texto. O caso do vestido 29237, chamado de "maxi" e curto na foto, mostra que esse erro existe.

## 6. Conclusões

1. O problema é real. Com a regra de aplicação, o desconhecido cai pouco. A estampa vai de 85,7% para 83,7%, o comprimento vai de 89,1% para 82,6% e a modelagem fica em 92,1%.
2. Na estampa, o desconhecido é quase sempre peça lisa. Nas 30 fotos, 24 eram lisas e só 4 eram estampadas. O texto não diz que a peça é lisa. Tratar "desconhecido" como "provavelmente lisa" é mais fiel do que tratar como falta de dado.
3. A variacao_cor não separa lisa de estampada. Ela depende muito da claridade da peça, e as peças pretas lisas ficam com os valores mais altos. Abaixo de 0,19 a peça tende a ser lisa, mas a amostra é pequena e o limite cobre poucas peças.
4. As listas novas ajudam pouco em estampa e modelagem (menos 1,8 e 4,4 pontos). Em comprimento a queda é de 18 pontos, quase toda vinda da regra de calça. O texto não traz mais do que isso.
5. Comprimento de casaco e modelagem de casaco e blusa quase nunca estão no texto. Para esses atributos, a imagem é a fonte que falta.

## 7. Categoria errada na base

Os agentes que rotularam a referência de 300 peças viram 10 imagens que não batem com a categoria da base. Exemplos: um "top" que é um tênis e um "rings" que é uma pilha de pulseiras. São 3,3% da amostra. A lista está em `dados/referencia/categoria_errada.csv`.

A categoria é o filtro da KD-tree. Então essas peças entram na busca da categoria errada. O projeto não corrige a categoria. Isso fica no artigo como limitação da base.
