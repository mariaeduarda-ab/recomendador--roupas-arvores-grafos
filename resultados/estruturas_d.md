# Medições da estrutura da versão D

Medido na base inteira (102.370 peças) com k = 5. Nada aqui usa looks de teste. A cobertura usa 2.343 looks de treino, porque as arestas não dependem dos looks. É uma medida da estrutura, não o resultado final.

## Empates no vetor de busca

O grupo 1 tem 11 números, e 10 deles são 0 ou 1.

- Existem só 637 vetores diferentes na base inteira.
- Em média, cada vetor se repete em 25,6 peças da mesma categoria.
- Em 95,1% das buscas, a 5ª e a 6ª vizinhas têm a mesma distância.
- Em 80,9% das buscas, a 5ª vizinha está a distância 0.

Consequência: o corte em k = 5 escolhe entre muitas peças empatadas. A escolha é sempre a mesma em toda execução, mas não tem significado. Ela depende da posição da peça na árvore. Isso explica boa parte dos números abaixo.

A KD-tree não degenera com peças iguais. A mediana é por posição, então a altura continua perto de log n. Com o heap cheio em distância 0, a poda corta o resto. Um teste com 50 mil pontos iguais visita menos de 100 nós.

A KD-tree por categoria deu as mesmas distâncias da força bruta em 1.000 buscas aleatórias na base real.

## Arestas

| Busca | Regra | Construir | Arestas | Peças sem aresta | Candidatas por teste | Cobertura |
| --- | --- | --- | --- | --- | --- | --- |
| Grupo 1 | mútuo | 40 s | 242.060 | 90,9% | 0,9 | 0,04% |
| Grupo 1 | basta um lado | 40 s | 9.483.090 | 0% | 39,9 | 0,73% |
| Todos misturados | mútuo | 577 s | 1.206.089 | 1,4% | 6,4 | 0,60% |
| Todos misturados | basta um lado | 560 s | 8.519.061 | 0% | 37,8 | 1,20% |

Cobertura é a porcentagem de testes em que a peça escondida aparece entre as candidatas. No mútuo com grupo 1, 81,6% dos testes ficam sem nenhuma candidata.

Como pedido, o mútuo deixa muitas peças sem aresta, então a variante "basta um lado" também foi implementada e medida.

"Todos misturados" tem 15 números, acima do limite de 12. Ele existe só como comparação. O tempo de 10 a 15 vezes maior mostra a KD-tree perdendo eficiência com mais dimensões e menos empates.

## Decisões

- **Candidata:** peça da categoria pedida ligada por aresta a pelo menos uma peça do look.
- **Soma:** nota da candidata com cada peça do look de outra categoria, mesmo que essa dupla não tenha aresta. Assim, looks maiores não prejudicam quem tem poucas arestas.
- **Bônus de grupo:** grupo é categoria + primeiro estilo + estampa (lisa, desconhecida ou estampada). O bônus é `juntos / raiz(aparições de p × aparições de q)`. O prompt divide só por q, mas no grupo isso daria valores muito acima das outras partes da nota.
- **Regra de cor:** 0,5 se uma das peças é neutra. 60% das peças são neutras, então a regra separa pouco.
- **Desligar uma parte:** peso 0.
