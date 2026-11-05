# Tabela final (teste)

Gerado por `./recomendador avaliar_justo`. Looks de teste, uma peça escondida por look com `sortear_escondidas`. Conferência: as 5842 escondidas batem com resultados/escondidas.csv.

- Consultas: 5842. Com pergunta de FITB: 5842. Candidatas por consulta: 10940 em média.
- Cada versão dá nota a todas as peças da categoria da peça escondida. Só as peças do próprio look saem. Peça sem nota numa versão (B, C e as listas curtas da D) fica com nota 0 e entra no empate.
- Posição da escondida = peças com nota maior + peças com a mesma nota que vêm antes dela na ordem sorteada.
- 10 ordens, sementes 1 a 10. Cada número é a média das 10 ordens, com o menor e o maior valor entre parênteses.
- FITB: a escondida contra 3 erradas da mesma categoria (as mesmas perguntas do `avaliar_fitb`). Ela acerta se tem a maior nota das 4. No empate vale a mesma ordem sorteada. O chute acerta 25%.
- D: config de `config_d.csv` com `pecas_v2.csv`. D nova: mesma config com `pecas_v4.csv` e só os grupos de `vetor_d.csv`. As duas agora dão nota a todas as peças da categoria. As linhas "só lista curta" mostram a D antiga, que só pontua as vizinhas das arestas.
- F e C + popularidade: pesos de `config_f.csv`, escolhidos só na validação pelo `ajustar_f` (ver `escolha_final.md`). Cada termo vai de 0 a 1 dentro da consulta. As duas desempatam pela popularidade antes da ordem sorteada.

## Por que sortear a ordem é o mesmo que embaralhar as linhas

Todas as versões desempatam pelo índice da peça, que é a linha dela no catálogo. Embaralhar as linhas troca esses índices por uma ordem sorteada. A nota de cada peça não muda, só a posição de quem empata. Então ordenar pela nota e, no empate, pela ordem sorteada dá a mesma lista que embaralhar o arquivo e rodar de novo. A diferença é que aqui a nota é calculada uma vez por consulta e só o desempate muda. Isso não vale para a lista curta da D, porque a busca kNN também desempata pela linha. Por isso a D principal agora dá nota à categoria inteira.

## Resultado

| Versão | Hit@1 | Hit@5 | Hit@10 | FITB | Empatadas com a escondida |
| --- | --- | --- | --- | --- | --- |
| Aleatória (esperado) | 0,018% | 0,091% | 0,182% | 25,00% | todas |
| Aleatória | 0,027% (0,000% a 0,068%) | 0,096% (0,051% a 0,154%) | 0,187% (0,120% a 0,274%) | 24,92% (24,65% a 25,83%) | 10939 |
| Popularidade | 0,142% (0,120% a 0,154%) | 0,568% (0,531% a 0,616%) | 0,895% (0,839% a 0,941%) | 31,40% (30,95% a 32,10%) | 2633 |
| A | 0,051% (0,051% a 0,051%) | 0,188% (0,188% a 0,188%) | 0,514% (0,514% a 0,514%) | 34,29% (34,29% a 34,29%) | 0 |
| B | 0,373% (0,342% a 0,479%) | 0,647% (0,582% a 0,719%) | 0,772% (0,702% a 0,856%) | 25,35% (25,03% a 26,21%) | 10880 |
| C | 0,336% (0,308% a 0,411%) | 0,633% (0,582% a 0,668%) | 0,751% (0,702% a 0,787%) | 25,37% (25,06% a 26,21%) | 10870 |
| D | 0,329% (0,308% a 0,359%) | 0,531% (0,445% a 0,599%) | 0,659% (0,599% a 0,753%) | 28,14% (27,76% a 28,62%) | 1090 |
| D nova | 0,312% (0,308% a 0,325%) | 0,575% (0,531% a 0,668%) | 0,688% (0,599% a 0,805%) | 27,26% (26,74% a 27,85%) | 3508 |
| C + popularidade | 0,313% (0,308% a 0,325%) | 0,746% (0,702% a 0,787%) | 1,058% (1,027% a 1,096%) | 31,63% (31,19% a 32,30%) | 2607 |
| F | 0,313% (0,308% a 0,325%) | 0,716% (0,685% a 0,753%) | 1,056% (1,044% a 1,078%) | 32,04% (31,55% a 32,59%) | 1920 |
| D (só lista curta) | 0,015% (0,000% a 0,051%) | 0,072% (0,051% a 0,103%) | 0,154% (0,120% a 0,188%) | 24,86% (24,48% a 25,78%) | 10638 |
| D nova (só lista curta) | 0,021% (0,000% a 0,034%) | 0,082% (0,034% a 0,137%) | 0,193% (0,154% a 0,240%) | 24,85% (24,46% a 25,66%) | 10712 |

A última coluna é a média de outras peças com exatamente a mesma nota da escondida.

## Diferenças com intervalo de 95% (bootstrap)

Para cada consulta, o acerto é a média das 10 ordens. Os looks são sorteados com reposição 1.000 vezes (semente 2026). O intervalo vai do percentil 2,5 ao 97,5 da diferença. Se o intervalo não passa pelo 0, a diferença é real. Se passa, pode ser sorte. pp = pontos percentuais.

| Comparação | Métrica | Diferença | Intervalo de 95% | Conclusão |
| --- | --- | --- | --- | --- |
| Aleatória x Popularidade | Hit@1 | −0,115 pp | −0,217 a −0,024 pp | Aleatória perde de Popularidade (diferença real) |
| Aleatória x Popularidade | Hit@5 | −0,472 pp | −0,669 a −0,286 pp | Aleatória perde de Popularidade (diferença real) |
| Aleatória x Popularidade | Hit@10 | −0,709 pp | −0,959 a −0,488 pp | Aleatória perde de Popularidade (diferença real) |
| Aleatória x Popularidade | FITB | −6,48 pp | −7,11 a −5,88 pp | Aleatória perde de Popularidade (diferença real) |
| A x Popularidade | Hit@1 | −0,091 pp | −0,204 a 0,017 pp | pode ser sorte |
| A x Popularidade | Hit@5 | −0,380 pp | −0,604 a −0,164 pp | A perde de Popularidade (diferença real) |
| A x Popularidade | Hit@10 | −0,382 pp | −0,688 a −0,082 pp | A perde de Popularidade (diferença real) |
| A x Popularidade | FITB | 2,88 pp | 1,53 a 4,28 pp | A ganha de Popularidade (diferença real) |
| B x Popularidade | Hit@1 | 0,231 pp | 0,067 a 0,413 pp | B ganha de Popularidade (diferença real) |
| B x Popularidade | Hit@5 | 0,079 pp | −0,175 a 0,329 pp | pode ser sorte |
| B x Popularidade | Hit@10 | −0,123 pp | −0,394 a 0,168 pp | pode ser sorte |
| B x Popularidade | FITB | −6,05 pp | −6,69 a −5,47 pp | B perde de Popularidade (diferença real) |
| C x Popularidade | Hit@1 | 0,193 pp | 0,034 a 0,361 pp | C ganha de Popularidade (diferença real) |
| C x Popularidade | Hit@5 | 0,065 pp | −0,193 a 0,322 pp | pode ser sorte |
| C x Popularidade | Hit@10 | −0,144 pp | −0,428 a 0,147 pp | pode ser sorte |
| C x Popularidade | FITB | −6,03 pp | −6,65 a −5,47 pp | C perde de Popularidade (diferença real) |
| D x Popularidade | Hit@1 | 0,187 pp | 0,034 a 0,354 pp | D ganha de Popularidade (diferença real) |
| D x Popularidade | Hit@5 | −0,038 pp | −0,281 a 0,211 pp | pode ser sorte |
| D x Popularidade | Hit@10 | −0,236 pp | −0,537 a 0,046 pp | pode ser sorte |
| D x Popularidade | FITB | −3,27 pp | −4,41 a −2,10 pp | D perde de Popularidade (diferença real) |
| D nova x Popularidade | Hit@1 | 0,169 pp | 0,014 a 0,339 pp | D nova ganha de Popularidade (diferença real) |
| D nova x Popularidade | Hit@5 | 0,007 pp | −0,231 a 0,267 pp | pode ser sorte |
| D nova x Popularidade | Hit@10 | −0,207 pp | −0,495 a 0,087 pp | pode ser sorte |
| D nova x Popularidade | FITB | −4,14 pp | −5,25 a −3,06 pp | D nova perde de Popularidade (diferença real) |
| C + popularidade x Popularidade | Hit@1 | 0,171 pp | 0,024 a 0,330 pp | C + popularidade ganha de Popularidade (diferença real) |
| C + popularidade x Popularidade | Hit@5 | 0,178 pp | −0,019 a 0,383 pp | pode ser sorte |
| C + popularidade x Popularidade | Hit@10 | 0,163 pp | −0,067 a 0,380 pp | pode ser sorte |
| C + popularidade x Popularidade | FITB | 0,23 pp | 0,10 a 0,36 pp | C + popularidade ganha de Popularidade (diferença real) |
| F x Popularidade | Hit@1 | 0,171 pp | 0,024 a 0,330 pp | F ganha de Popularidade (diferença real) |
| F x Popularidade | Hit@5 | 0,147 pp | −0,058 a 0,359 pp | pode ser sorte |
| F x Popularidade | Hit@10 | 0,161 pp | −0,068 a 0,383 pp | pode ser sorte |
| F x Popularidade | FITB | 0,63 pp | 0,14 a 1,11 pp | F ganha de Popularidade (diferença real) |
| Aleatória x C | Hit@1 | −0,308 pp | −0,455 a −0,173 pp | Aleatória perde de C (diferença real) |
| Aleatória x C | Hit@5 | −0,537 pp | −0,750 a −0,347 pp | Aleatória perde de C (diferença real) |
| Aleatória x C | Hit@10 | −0,565 pp | −0,779 a −0,361 pp | Aleatória perde de C (diferença real) |
| Aleatória x C | FITB | −0,45 pp | −0,61 a −0,29 pp | Aleatória perde de C (diferença real) |
| A x C | Hit@1 | −0,284 pp | −0,436 a −0,140 pp | A perde de C (diferença real) |
| A x C | Hit@5 | −0,445 pp | −0,673 a −0,219 pp | A perde de C (diferença real) |
| A x C | Hit@10 | −0,238 pp | −0,514 a 0,053 pp | pode ser sorte |
| A x C | FITB | 8,92 pp | 7,62 a 10,14 pp | A ganha de C (diferença real) |
| B x C | Hit@1 | 0,038 pp | 0,010 a 0,080 pp | B ganha de C (diferença real) |
| B x C | Hit@5 | 0,014 pp | −0,027 a 0,046 pp | pode ser sorte |
| B x C | Hit@10 | 0,021 pp | −0,036 a 0,062 pp | pode ser sorte |
| B x C | FITB | −0,02 pp | −0,08 a 0,02 pp | pode ser sorte |
| D x C | Hit@1 | −0,007 pp | −0,082 a 0,068 pp | pode ser sorte |
| D x C | Hit@5 | −0,103 pp | −0,219 a 0,000 pp | pode ser sorte |
| D x C | Hit@10 | −0,092 pp | −0,226 a 0,029 pp | pode ser sorte |
| D x C | FITB | 2,77 pp | 1,67 a 3,79 pp | D ganha de C (diferença real) |
| D nova x C | Hit@1 | −0,024 pp | −0,099 a 0,050 pp | pode ser sorte |
| D nova x C | Hit@5 | −0,058 pp | −0,159 a 0,039 pp | pode ser sorte |
| D nova x C | Hit@10 | −0,063 pp | −0,185 a 0,046 pp | pode ser sorte |
| D nova x C | FITB | 1,89 pp | 0,97 a 2,77 pp | D nova ganha de C (diferença real) |
| C + popularidade x C | Hit@1 | −0,022 pp | −0,116 a 0,068 pp | pode ser sorte |
| C + popularidade x C | Hit@5 | 0,113 pp | −0,034 a 0,272 pp | pode ser sorte |
| C + popularidade x C | Hit@10 | 0,306 pp | 0,116 a 0,503 pp | C + popularidade ganha de C (diferença real) |
| C + popularidade x C | FITB | 6,26 pp | 5,69 a 6,87 pp | C + popularidade ganha de C (diferença real) |
| F x C | Hit@1 | −0,022 pp | −0,116 a 0,068 pp | pode ser sorte |
| F x C | Hit@5 | 0,082 pp | −0,068 a 0,245 pp | pode ser sorte |
| F x C | Hit@10 | 0,305 pp | 0,113 a 0,510 pp | F ganha de C (diferença real) |
| F x C | FITB | 6,67 pp | 5,90 a 7,43 pp | F ganha de C (diferença real) |
| F x C + popularidade | Hit@1 | 0,000 pp | 0,000 a 0,000 pp | pode ser sorte |
| F x C + popularidade | Hit@5 | −0,031 pp | −0,084 a 0,007 pp | pode ser sorte |
| F x C + popularidade | Hit@10 | −0,002 pp | −0,015 a 0,010 pp | pode ser sorte |
| F x C + popularidade | FITB | 0,41 pp | −0,05 a 0,86 pp | pode ser sorte |
