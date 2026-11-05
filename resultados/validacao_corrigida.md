# Validação com a avaliação corrigida

Gerado por `./recomendador avaliar_justo --validacao`. Looks de validação (semente 7). Eles saem do grafo, do bônus e da popularidade. O teste não é usado.

- Consultas: 2332. Com pergunta de FITB: 2332. Candidatas por consulta: 10835 em média.
- Cada versão dá nota a todas as peças da categoria da peça escondida. Só as peças do próprio look saem. Peça sem nota numa versão (B, C e as listas curtas da D) fica com nota 0 e entra no empate.
- Posição da escondida = peças com nota maior + peças com a mesma nota que vêm antes dela na ordem sorteada.
- 10 ordens, sementes 1 a 10. Cada número é a média das 10 ordens, com o menor e o maior valor entre parênteses.
- FITB: a escondida contra 3 erradas da mesma categoria (as mesmas perguntas do `avaliar_fitb`). Ela acerta se tem a maior nota das 4. No empate vale a mesma ordem sorteada. O chute acerta 25%.
- D: config de `config_d.csv` com `pecas_v2.csv`. D nova: mesma config com `pecas_v4.csv` e só os grupos de `vetor_d.csv`. As duas agora dão nota a todas as peças da categoria. As linhas "só lista curta" mostram a D antiga, que só pontua as vizinhas das arestas.

## Por que sortear a ordem é o mesmo que embaralhar as linhas

Todas as versões desempatam pelo índice da peça, que é a linha dela no catálogo. Embaralhar as linhas troca esses índices por uma ordem sorteada. A nota de cada peça não muda, só a posição de quem empata. Então ordenar pela nota e, no empate, pela ordem sorteada dá a mesma lista que embaralhar o arquivo e rodar de novo. A diferença é que aqui a nota é calculada uma vez por consulta e só o desempate muda. Isso não vale para a lista curta da D, porque a busca kNN também desempata pela linha. Por isso a D principal agora dá nota à categoria inteira.

## Resultado

| Versão | Hit@1 | Hit@5 | Hit@10 | FITB | Empatadas com a escondida |
| --- | --- | --- | --- | --- | --- |
| Aleatória (esperado) | 0,018% | 0,089% | 0,179% | 25,00% | todas |
| Aleatória | 0,013% (0,000% a 0,043%) | 0,111% (0,043% a 0,214%) | 0,223% (0,086% a 0,343%) | 24,87% (24,01% a 25,64%) | 10834 |
| Popularidade | 0,026% (0,000% a 0,043%) | 0,309% (0,257% a 0,386%) | 0,686% (0,600% a 0,772%) | 29,74% (29,03% a 30,40%) | 3135 |
| A | 0,214% (0,214% a 0,214%) | 0,386% (0,386% a 0,386%) | 0,429% (0,429% a 0,429%) | 34,22% (34,22% a 34,22%) | 0 |
| B | 0,605% (0,557% a 0,686%) | 0,806% (0,729% a 0,858%) | 0,931% (0,815% a 1,072%) | 25,41% (24,57% a 26,11%) | 10758 |
| C | 0,596% (0,557% a 0,643%) | 0,862% (0,772% a 0,943%) | 0,926% (0,858% a 0,943%) | 25,45% (24,66% a 26,20%) | 10747 |
| D | 0,540% (0,515% a 0,557%) | 0,776% (0,686% a 0,858%) | 0,901% (0,815% a 1,072%) | 29,27% (28,95% a 29,72%) | 1018 |
| D nova | 0,536% (0,515% a 0,600%) | 0,708% (0,600% a 0,815%) | 0,815% (0,729% a 0,986%) | 26,28% (25,34% a 27,44%) | 3405 |
| D (só lista curta) | 0,086% (0,086% a 0,086%) | 0,154% (0,129% a 0,214%) | 0,180% (0,129% a 0,214%) | 24,57% (23,93% a 25,13%) | 10536 |
| D nova (só lista curta) | 0,103% (0,086% a 0,129%) | 0,180% (0,129% a 0,214%) | 0,283% (0,214% a 0,472%) | 24,73% (24,06% a 25,60%) | 10589 |

A última coluna é a média de outras peças com exatamente a mesma nota da escondida.

## Diferenças com intervalo de 95% (bootstrap)

Para cada consulta, o acerto é a média das 10 ordens. Os looks são sorteados com reposição 1.000 vezes (semente 2026). O intervalo vai do percentil 2,5 ao 97,5 da diferença. Se o intervalo não passa pelo 0, a diferença é real. Se passa, pode ser sorte. pp = pontos percentuais.

| Comparação | Métrica | Diferença | Intervalo de 95% | Conclusão |
| --- | --- | --- | --- | --- |
| Aleatória x Popularidade | Hit@1 | −0,013 pp | −0,056 a 0,021 pp | pode ser sorte |
| Aleatória x Popularidade | Hit@5 | −0,197 pp | −0,412 a −0,009 pp | Aleatória perde de Popularidade (diferença real) |
| Aleatória x Popularidade | Hit@10 | −0,463 pp | −0,810 a −0,159 pp | Aleatória perde de Popularidade (diferença real) |
| Aleatória x Popularidade | FITB | −4,87 pp | −5,86 a −3,89 pp | Aleatória perde de Popularidade (diferença real) |
| A x Popularidade | Hit@1 | 0,189 pp | 0,017 a 0,403 pp | A ganha de Popularidade (diferença real) |
| A x Popularidade | Hit@5 | 0,077 pp | −0,240 a 0,420 pp | pode ser sorte |
| A x Popularidade | Hit@10 | −0,257 pp | −0,678 a 0,184 pp | pode ser sorte |
| A x Popularidade | FITB | 4,48 pp | 2,36 a 6,52 pp | A ganha de Popularidade (diferença real) |
| B x Popularidade | Hit@1 | 0,579 pp | 0,296 a 0,918 pp | B ganha de Popularidade (diferença real) |
| B x Popularidade | Hit@5 | 0,497 pp | 0,116 a 0,913 pp | B ganha de Popularidade (diferença real) |
| B x Popularidade | Hit@10 | 0,244 pp | −0,236 a 0,725 pp | pode ser sorte |
| B x Popularidade | FITB | −4,33 pp | −5,36 a −3,36 pp | B perde de Popularidade (diferença real) |
| C x Popularidade | Hit@1 | 0,570 pp | 0,287 a 0,905 pp | C ganha de Popularidade (diferença real) |
| C x Popularidade | Hit@5 | 0,553 pp | 0,176 a 0,995 pp | C ganha de Popularidade (diferença real) |
| C x Popularidade | Hit@10 | 0,240 pp | −0,270 a 0,750 pp | pode ser sorte |
| C x Popularidade | FITB | −4,28 pp | −5,29 a −3,29 pp | C perde de Popularidade (diferença real) |
| D x Popularidade | Hit@1 | 0,515 pp | 0,236 a 0,853 pp | D ganha de Popularidade (diferença real) |
| D x Popularidade | Hit@5 | 0,467 pp | 0,090 a 0,888 pp | D ganha de Popularidade (diferença real) |
| D x Popularidade | Hit@10 | 0,214 pp | −0,227 a 0,708 pp | pode ser sorte |
| D x Popularidade | FITB | −0,47 pp | −2,38 a 1,62 pp | pode ser sorte |
| D nova x Popularidade | Hit@1 | 0,510 pp | 0,240 a 0,840 pp | D nova ganha de Popularidade (diferença real) |
| D nova x Popularidade | Hit@5 | 0,399 pp | 0,039 a 0,810 pp | D nova ganha de Popularidade (diferença real) |
| D nova x Popularidade | Hit@10 | 0,129 pp | −0,339 a 0,622 pp | pode ser sorte |
| D nova x Popularidade | FITB | −3,46 pp | −5,28 a −1,78 pp | D nova perde de Popularidade (diferença real) |
| Aleatória x C | Hit@1 | −0,583 pp | −0,918 a −0,300 pp | Aleatória perde de C (diferença real) |
| Aleatória x C | Hit@5 | −0,750 pp | −1,149 a −0,420 pp | Aleatória perde de C (diferença real) |
| Aleatória x C | Hit@10 | −0,703 pp | −1,106 a −0,373 pp | Aleatória perde de C (diferença real) |
| Aleatória x C | FITB | −0,58 pp | −0,88 a −0,33 pp | Aleatória perde de C (diferença real) |
| A x C | Hit@1 | −0,382 pp | −0,759 a −0,039 pp | A perde de C (diferença real) |
| A x C | Hit@5 | −0,476 pp | −0,943 a −0,021 pp | A perde de C (diferença real) |
| A x C | Hit@10 | −0,497 pp | −0,969 a −0,034 pp | A perde de C (diferença real) |
| A x C | FITB | 8,77 pp | 6,80 a 10,57 pp | A ganha de C (diferença real) |
| B x C | Hit@1 | 0,009 pp | 0,000 a 0,021 pp | pode ser sorte |
| B x C | Hit@5 | −0,056 pp | −0,214 a 0,039 pp | pode ser sorte |
| B x C | Hit@10 | 0,004 pp | −0,154 a 0,111 pp | pode ser sorte |
| B x C | FITB | −0,04 pp | −0,16 a 0,04 pp | pode ser sorte |
| D x C | Hit@1 | −0,056 pp | −0,202 a 0,051 pp | pode ser sorte |
| D x C | Hit@5 | −0,086 pp | −0,304 a 0,107 pp | pode ser sorte |
| D x C | Hit@10 | −0,026 pp | −0,266 a 0,197 pp | pode ser sorte |
| D x C | FITB | 3,82 pp | 2,03 a 5,60 pp | D ganha de C (diferença real) |
| D nova x C | Hit@1 | −0,060 pp | −0,202 a 0,047 pp | pode ser sorte |
| D nova x C | Hit@5 | −0,154 pp | −0,373 a 0,009 pp | pode ser sorte |
| D nova x C | Hit@10 | −0,111 pp | −0,352 a 0,060 pp | pode ser sorte |
| D nova x C | FITB | 0,83 pp | −0,66 a 2,28 pp | pode ser sorte |
