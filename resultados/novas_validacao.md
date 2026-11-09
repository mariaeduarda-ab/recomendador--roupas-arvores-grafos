# Variações para a peça nova (validação)

Gerado por `./recomendador ajustar_novas`. O teste não é montado.

- Consultas de validação: 2332. Metade A: 1166. Metade B: 1166 (sorteio com semente 2711).
- Escondidas sem nenhum look de treino (peça nova): 1718 de 2332 (73,7%).
- A metade A escolhe. A metade B só confirma: uma variação fica se ganhar da F atual nas duas metades.
- Hit@k é a média das 10 ordens de desempate. Os intervalos são de bootstrap na metade B (diferença para a F atual).
- Todas as notas desempatam pela popularidade, como a F.

## Melhor de cada variação (escolhida na metade A)

| Variação | Configuração | Hit@10 A | Hit@10 B | FITB A | FITB B | Hit@10 B x F | FITB B x F |
| --- | --- | --- | --- | --- | --- | --- | --- |
| F atual | w = (1,0; 0,0; 0,5; 1,0), grupos Kruskal | 1,123% | 1,312% | 30,39% | 31,13% |  |  |
| forma 3 com recuo | w = (1,0; 0,0; 0,5; 1,0), grupos forma 3 | 1,029% | 1,072% | 39,35% | 40,70% | −0,240 pp (−0,652 a 0,129) pode ser sorte | 9,57 pp (6,73 a 12,18) real |
| peso por grau (Kruskal) | w = (0,5; 0,0; 1,0; 1,0), grupos Kruskal, a = 0,5 | 1,175% | 1,141% | 30,30% | 31,22% | −0,172 pp (−0,429 a 0,000) pode ser sorte | 0,09 pp (0,00 a 0,26) pode ser sorte |
| forma 3 + peso por grau | w = (0,0; 0,5; 0,5; 0,0), grupos forma 3, a = 0,5 | 1,630% | 0,995% | 32,32% | 35,09% | −0,317 pp (−1,149 a 0,540) pode ser sorte | 3,95 pp (0,39 a 7,18) real |
| regressão logística | regressão logística (13 colunas) | 1,552% | 1,286% | 39,61% | 41,05% | −0,026 pp (−0,892 a 0,866) pode ser sorte | 9,92 pp (6,59 a 12,95) real |

Total de configurações testadas: 596. Todas estão em `novas_grade.csv`.

## Escolhas

- Melhor pelo Hit@10 na metade A: forma 3 + peso por grau, w = (0,0; 0,5; 0,5; 0,0), grupos forma 3, a = 0,5. Não ganha da F nas duas metades e não vai ao teste.
- Melhor pelo FITB na metade A (critério acrescentado depois): forma 3 + peso por grau, w = (1,0; 0,0; 0,5; 1,0), grupos forma 3, a = 0,5. Ganha da F no FITB nas duas metades e vai ao teste.

## Regressão logística

Pares da metade A: 12822 (1166 certos). 3000 passos de descida de gradiente em lote, taxa 1,0, L2 0.001. Perda média de 0,6931 para 0,2558.

| Coluna | Peso aprendido |
| --- | --- |
| C | 0,551 |
| popularidade | 0,616 |
| cor | 0,703 |
| estampa | 0,061 |
| formalidade | 0,108 |
| estilo | 0,084 |
| ocasiao | 0,018 |
| comprimento | −0,031 |
| grupos Kruskal | −0,107 |
| grupos forma 3 | 0,532 |
| atributos / (1 + grau) | 0,097 |
| forma 3 / (1 + grau) | −0,044 |
| peca nova | 2,138 |
