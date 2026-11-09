# Peças gêmeas

Gerado com `python3 preprocess/gemeas.py --entrada Combined_Metadata.json` (passo 16, com legenda).

## O que existe no JSON

Cada registro tem só `image_name` (número do look + posição, por exemplo `200266907_1.jpg`), `category`, as duas legendas, o caminho da imagem e as cores. Não há código de produto, nome nem preço. O nome do arquivo identifica o look, não o produto.

O `preprocessar.py` já junta os registros com a mesma categoria, as mesmas 3 cores exatas e a mesma legenda do BLIP. As 126.928 linhas viram 102.370 peças.

## Impressão digital testada

Categoria + as 3 cores com cada canal dividido por 16 (sem resto) + legenda do BLIP. Peças com a mesma impressão digital vão para a mesma classe, com uma tabela hash (dicionário) e Union-Find.

| | Valor |
| --- | --- |
| Classes | 101.146 |
| Classes com mais de 1 peça | 960 (2.184 peças) |
| Maior classe | 11 peças |
| Peças dos looks de validação com alguma peça da classe num look de treino | 26,0% antes, 27,2% depois |

A conta da validação usa todas as peças dos 2.506 looks de validação e tira esses looks do treino. O teste não entra.

## Conferência pela imagem

50 classes sorteadas (semente 7) e 2 peças sorteadas de cada uma. As imagens foram baixadas só para essas 100 peças. Resultado em `gemeas_pares.csv`, coluna `mesma_peca`.

**24 dos 50 pares são a mesma peça.** Os erros são peças parecidas da mesma cor com a mesma legenda genérica do BLIP, por exemplo "a black skirt" para saias diferentes, calças jeans diferentes e bolsas pretas diferentes.

## Decisão

A regra era usar as gêmeas só com pelo menos 45 de 50 pares certos e com menos peças novas na validação. A segunda parte passa (de 26,0% para 27,2%), mas a primeira não: 24 de 50. **As gêmeas não entram.** A avaliação e o catálogo continuam como estão.

Uma variante mais solta, sem a legenda e com passo 8, chega a 32,8% na validação, mas junta ainda mais peças diferentes. Ela foi medida e os pares foram baixados, mas as imagens não foram conferidas, porque ela foge da impressão digital pedida e tende a errar mais.
