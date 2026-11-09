# Atributos, grupos e divisão de looks para a FashionVerse-MME

Dados gerados pelo trabalho final de AEDS 2 (CEFET-MG), um recomendador de roupas com KD-tree e grafos. Código: https://github.com/mariaeduarda-ab/recomendador--roupas-arvores-grafos (tag `v-final-2`).

Base original: FashionVerse-MME (Multimodal Metadata Enrichment), Mendeley Data, versão 1, DOI 10.17632/6sx3pbsfvz.1, licença CC BY 4.0. Link: https://data.mendeley.com/datasets/6sx3pbsfvz/1

Esta pasta **não** traz imagens, legendas nem descrições da base. Para ver a peça, use `image_name` e `pasta_original` dentro do zip `FashionVerse-MME.zip` da base original.

## Como os dados foram gerados

1. `preprocess/preprocessar.py` lê o `Combined_Metadata.json` (126.928 registros). 277 registros fogem do padrão `<look>_<posição>.jpg` e ficam de fora. Registros com a mesma categoria, as mesmas 3 cores dominantes e a mesma legenda do BLIP viram uma peça só. Assim saem 102.370 peças. Os nomes `outwear` e `watches` viram `outerwear` e `watch`, o que dá 20 categorias.
2. Os looks são sorteados com semente 42: 80% para treino e 20% para teste. Depois, 10% dos looks de treino são sorteados para validação com semente 7 (no código C++, `sortear_validacao`).
3. `preprocess/atributos.py` tira do texto os atributos de estilo, ocasião, formalidade, estampa e comprimento, por listas de palavras. `preprocess/atributos_finais.py` usa "lisa" quando o texto não acha palavra de estampa.
4. `preprocess/grupos_cor.py` agrupa as cores Lab em 12 grupos com k-means (semente 42).
5. `./recomendador exportar_grupos` grava os grupos. `./recomendador pontos` grava as tabelas de pontos, só com os looks de treino.
6. `python3 preprocess/zenodo.py --entrada Combined_Metadata.json` monta esta pasta.

## Arquivos

### `pecas.csv`

Uma linha por peça.

| Coluna | Significado |
| --- | --- |
| `id` | id da peça no nosso projeto (0 a 102.369) |
| `categoria` | uma das 20 categorias |
| `imagem_original` | pasta e nome do primeiro registro da peça dentro do zip original |
| `registros` | quantos registros da base original viraram esta peça |

### `registros_originais.csv`

Uma linha por registro original usado (126.651).

| Coluna | Significado |
| --- | --- |
| `id` | id da peça no nosso projeto |
| `image_name` | campo `image_name` do JSON original, no formato `<número do look>_<posição>.jpg` |
| `pasta_original` | pasta do registro dentro do zip (nem sempre igual ao campo `category`) |

### `atributos.csv`

Uma linha por peça. Os atributos de texto vêm da legenda refinada da base original. Quando o texto não tem palavra para o atributo, o valor é "desconhecido".

| Coluna | Significado |
| --- | --- |
| `id`, `categoria` | como em `pecas.csv` |
| `variacao_cor` | distância média entre a cor dominante e as outras 2 cores em Lab, de 0 a 1 (min-max na base) |
| `formalidade_vetor` | formalidade pela contagem de palavras formais e casuais, vezes √0,5, usada no vetor da KD-tree |
| `estilo_casual` a `estilo_moderno` | 1 se o texto marca o estilo. A peça pode ter vários ou nenhum |
| `ocasiao_dia` a `ocasiao_praia` | 1 se o texto marca a ocasião. A peça pode ter várias ou nenhuma |
| `formalidade` | de 0 (casual) a 1 (formal). 0,5 quando não há palavra ou quando há empate |
| `L`, `a`, `b` | cor dominante em Lab, gravada como L/100, (a + 128)/255 e (b + 128)/255 |
| `cor_neutra` | 1 se o nome da cor dominante da base é preto, branco, cinza, creme, bege, caramelo ou marinho |
| `estampa` | lisa, listrada, xadrez, floral, poa, animal, outra ou desconhecido (desconhecido onde a estampa não se aplica, como joias) |
| `comprimento` | curto, medio, longo ou desconhecido. Só para roupas |
| `grupo_cor` | um dos 12 grupos do k-means na cor Lab, com o nome da cor de referência mais perto |

Acerto destes atributos (fonte texto) contra peças rotuladas pela imagem, na metade de teste da referência (`resultados/comparacao_fontes.md` no repositório). "Total" conta "desconhecido" como erro. "Quando responde" conta só as peças com valor.

| Atributo | Acerto total | Acerto quando responde |
| --- | --- | --- |
| estampa (texto, com "lisa" quando não há palavra) | 77,0% (97 de 126) | 77,0% |
| comprimento | 23,1% (18 de 78) | 85,7% (18 de 21) |
| estilo | 31,6% (59 de 187) | 54,6% (59 de 108) |
| ocasião | 60,4% (113 de 187) | 64,2% (113 de 176) |
| formalidade | 49,7% (93 de 187) | 59,6% (93 de 156) |

### `grupos.csv`

Os grupos não usam looks.

| Coluna | Significado |
| --- | --- |
| `id` | id da peça |
| `grupo_forma3` | grupo pela mesma combinação de `grupo_cor`, estampa (lisa, estampada, desconhecida) e faixa de formalidade, dentro da categoria |
| `chave_forma3` | a combinação em texto, por exemplo `branco|lisa|alta` |
| `grupo_kruskal` | grupo do Kruskal no grafo dos 10 vizinhos mais próximos (cor Lab + estampa com peso 0,1), parado em 100 grupos por categoria |

### `looks.csv`

Uma linha por par (look, peça).

| Coluna | Significado |
| --- | --- |
| `look_id` | id do look no nosso projeto |
| `look_original` | número do look na base original (o começo do `image_name`) |
| `peca_id` | id da peça |
| `conjunto` | treino, validacao ou teste |

### `escondidas_teste.csv`

A peça escondida de cada look de teste com pelo menos 2 peças (5.842 looks). Todas as versões do projeto foram testadas com esta lista.

### `tabelas_pontos/`

Um CSV por atributo (`cor`, `estampa`, `formalidade`, `estilo`, `ocasiao`, `comprimento`), contado só nos looks de treino (com os de validação dentro).

| Coluna | Significado |
| --- | --- |
| `tabela` | `categoria` (par de categorias) ou `grupo` (par de grupos de categoria, usado quando o par tem menos de 200 pares) |
| `lado_a`, `lado_b` | as duas categorias ou grupos |
| `pares` | pares de peças A–B com valor conhecido nos looks de treino |
| `valor_a`, `valor_b` | valores do atributo |
| `n` | vezes que os dois valores aparecem juntos |
| `esperado` | pares × fração de `valor_a` em A × fração de `valor_b` em B |
| `pontos` | log((n + 1) / (esperado + 1)). Positivo: aparecem juntos mais que o acaso |

## Licença

CC BY 4.0 (https://creativecommons.org/licenses/by/4.0/), a mesma da base original. Ao usar, cite a FashionVerse-MME (DOI 10.17632/6sx3pbsfvz.1) e este conjunto. Mudanças em relação à base original: registros repetidos juntados em peças, atributos tirados do texto, grupos, divisão de looks e tabelas de pontos. Nenhuma imagem ou texto da base foi incluído.
