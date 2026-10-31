# Avaliação da versão D

Teste rodado uma única vez, em 31/10/2026. A execução completa (`./recomendador avaliar_d`) levou cerca de 28 minutos. Nada foi ajustado depois de ver o teste.

## Como foi feito

- Mesma divisão de treino e teste e mesma função `sortear_escondidas` em todas as versões. A lista das 5.842 peças escondidas está em `escondidas.csv`.
- Validação: 2.332 looks com semente 7, ou seja, 10% dos looks de treino, retirados do grafo e do bônus durante o ajuste.
- Grade: k = 5, 10, 20, mútuo ou um lado, a = 0,5 ou 1, b = 0 ou 1, c = 0 ou 1, d = 1 ou 5. São 96 combinações, em `validacao_d.csv`. Critério: Hit@10, com desempate em Hit@5. Em empate total, fica a primeira da grade.

## Configuração escolhida

k = 20, basta um lado, a = 0,5, b = 0, c = 0, d = 1.

A melhor combinação acertou só 5 de 2.332 looks na validação. Com tão poucos acertos, essa escolha é muito sensível ao acaso. As regras de cor e estampa ficaram desligadas porque não ajudaram na validação.

## Resultado no teste (5.842 looks)

| Versão | Acertos @1 | Acertos @5 | Acertos @10 | Hit@1 | Hit@5 | Hit@10 | Cobertura |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Aleatória (esperado) | 1,1 | 5,3 | 10,6 | 0,018% | 0,091% | 0,182% | 100% |
| Aleatória (sorteio) | 2 | 6 | 11 | 0,034% | 0,103% | 0,188% | 100% |
| Popularidade | 10 | 33 | 52 | 0,171% | 0,565% | 0,890% | 100% |
| A | 3 | 11 | 30 | 0,051% | 0,188% | 0,514% | 100% |
| B | 22 | 34 | 35 | 0,377% | 0,582% | 0,599% | 0,60% |
| C | 21 | 35 | 37 | 0,359% | 0,599% | 0,633% | 0,65% |
| **D** | 0 | 5 | 10 | 0% | 0,086% | 0,171% | 1,90% |
| D sem regras de cor e estampa | 0 | 5 | 10 | 0% | 0,086% | 0,171% | 1,90% |
| D sem bônus de looks | 0 | 6 | 8 | 0% | 0,103% | 0,137% | 1,90% |
| D com pesos iniciais | 1 | 4 | 11 | 0,017% | 0,068% | 0,188% | 1,90% |
| D com kNN mútuo | 2 | 3 | 5 | 0,034% | 0,051% | 0,086% | 0,15% |
| D com atributos misturados | 4 | 14 | 25 | 0,068% | 0,240% | 0,428% | 3,54% |

"D sem regras" é igual a D porque a configuração escolhida já tem b = c = 0.

## Diferenças com intervalo de 95% (Hit@10)

| Comparação | Só D acerta | Só a outra acerta | Diferença | Intervalo |
| --- | --- | --- | --- | --- |
| D x B | 10 | 35 | −0,428% | −0,653% a −0,203% |
| D x C | 10 | 37 | −0,462% | −0,692% a −0,232% |
| D x Popularidade | 10 | 52 | −0,719% | −0,982% a −0,455% |

Os intervalos não passam pelo 0: D é pior que as três. Os valores de k = 1 e 5 estão em `diferencas_d.csv`.

## O que isso significa

- **A versão D piorou.** No Hit@10, ela fica no mesmo nível da escolha aleatória (10 contra 10,6 esperados), e essa escolha olha a categoria inteira.
- **A cobertura subiu, mas o acerto não.** D alcança 111 peças escondidas contra 38 da C. Só que a nota não coloca essas peças no topo: 10 de 111 entram no top 10.
- **Os atributos separam pouco.** Com só 637 vetores diferentes na busca e muitos empates, as arestas dependem da posição na árvore, como mostra `estruturas_d.md`.
- **Misturar todos os atributos foi melhor que a D escolhida.** Foram 25 acertos contra 10. A separação em dois grupos não ajudou nesta base. Essa versão usa 15 números e leva bem mais tempo. A comparação foi feita só no teste, então não pode virar a nova escolha sem outra rodada de validação.
- **Nenhuma variação de D passou de B, C ou popularidade no Hit@10.**

## Tempos

| Peças | KD-trees por categoria | Arestas | Uma recomendação |
| --- | --- | --- | --- |
| 1.000 | 0,32 ms | 67 ms | 0,010 ms |
| 10.000 | 3,1 ms | 1.880 ms | 0,016 ms |
| 100.000 | 39,5 ms | 92.907 ms | 0,165 ms |

Média de 3 repetições, com a configuração escolhida. Uma recomendação é a média de até 100 looks.

## Versão antiga na base real

`hit_k.csv` e `tempos.csv` foram gerados de novo na base real. Os Hit@k de A, B e C batem com `diagnostico.md`. A popularidade passou de 51 para 52 acertos no Hit@10 porque agora as peças do look saem da lista. Os tempos mudam um pouco entre execuções: nesta rodada, a busca na KD-tree levou 0,066 ms e a força bruta 1,46 ms com 100 mil peças, cerca de 22 vezes mais lenta.

Gráficos: `acertos.png` e `tempos.png`, gerados por `python3 preprocess/graficos.py`.
