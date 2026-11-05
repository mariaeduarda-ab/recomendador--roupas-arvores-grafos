# Escolha final da versão F (validação)

Gerado por `./recomendador ajustar_f`. Tudo aqui usa só os looks de validação (semente 7), com a avaliação justa. As tabelas de pontos, os grupos, o grafo peça–look e a popularidade são montados sem esses looks. O teste não é montado.

- Consultas: 2332. Cada número é a média das 10 ordens de desempate.
- `nota_F(c) = w1 × C(c) + w2 × pontos_atributos(c) + w3 × pontos_grupos(c) + w4 × popularidade(c)`.
- Cada termo vai de 0 a 1 dentro da consulta (mínimo e máximo entre as candidatas). Se todas as candidatas têm o mesmo valor, o termo vale 0.
- Popularidade = log(1 + grau da peça no grafo peça–look). No empate ganha a mais popular. Depois vale a ordem sorteada.
- Pesos de atributo na ordem cor; estampa; formalidade; estilo; ocasião; comprimento.

## Como a grade foi feita

A grade cheia teria 3^4 × 3^6 combinações para cada opção de grupos. Por isso:

1. Grade de w1 a w4 em (0; 0,5; 1) com os pesos de atributo em 1. Primeiro sem grupos (w3 = 0). Depois, para cada opção de grupos (K = 20, 50, 100 e peso da estampa 0; 0,1; 0,2), as combinações com w3 > 0. Os grupos só ficam se a melhor com grupos ganhar da melhor sem grupos.
2. Com os w fixos, cada peso de atributo passa por 0; 0,5 e 1, um de cada vez, com os outros fixos. Fica o valor que ganha. Se a melhor até aqui tem w2 = 0, os pesos não mudam nada nela. Então o ajuste parte da melhor combinação com w2 > 0 (mesmos grupos), e os pesos que saem dele vão para o passo 3.
3. Grade dos w de novo, com os pesos de atributo do passo 2.
4. Pesos de atributo de novo, um de cada vez.

Critério: maior Hit@10 (média das 10 ordens). No empate, maior Hit@5. Depois, a combinação com mais zeros. Por último, a com mais pesos iguais a 1, porque (1; 1) e (0,5; 0,5) dão a mesma lista. Foram 623 combinações. Todas estão em `grade_f.csv`.

## Grupos com Kruskal

Melhor combinação de cada opção de grupos (só com w3 > 0) contra a melhor sem grupos, no passo 1.

| Opção | Melhor combinação | Hit@5 | Hit@10 | FITB |
| --- | --- | --- | --- | --- |
| Sem grupos | w = (1; 0; 0; 1) | 0,922% | 1,175% | 30,14% |
| K = 20, estampa 0 | w = (1; 0; 0,5; 1), K = 20, estampa 0 | 0,901% | 1,158% | 30,36% |
| K = 50, estampa 0 | w = (1; 0; 0,5; 1), K = 50, estampa 0 | 0,901% | 1,171% | 30,39% |
| K = 100, estampa 0 | w = (1; 0; 0,5; 1), K = 100, estampa 0 | 0,901% | 1,171% | 30,04% |
| K = 20, estampa 0,1 | w = (1; 0; 0,5; 1), K = 20, estampa 0,1 | 0,901% | 1,132% | 30,35% |
| K = 50, estampa 0,1 | w = (1; 0; 0,5; 1), K = 50, estampa 0,1 | 0,901% | 1,132% | 30,41% |
| K = 100, estampa 0,1 | w = (1; 0; 0,5; 1), K = 100, estampa 0,1 | 0,858% | 1,218% | 30,76% |
| K = 20, estampa 0,2 | w = (1; 0; 0,5; 1), K = 20, estampa 0,2 | 0,901% | 1,175% | 30,65% |
| K = 50, estampa 0,2 | w = (1; 0; 0,5; 1), K = 50, estampa 0,2 | 0,901% | 1,175% | 30,72% |
| K = 100, estampa 0,2 | w = (1; 0; 0,5; 1), K = 100, estampa 0,2 | 0,858% | 1,218% | 30,76% |

A melhor com grupos ganhou da melhor sem grupos. Os grupos ficam na F.

## Escolha

- F: w = (1; 0; 0,5; 1), K = 100, estampa 0,1. Hit@10 = 1,218%, Hit@5 = 0,858%.
- C + popularidade: w = (1; 0; 0; 1). Hit@10 = 1,175%.
- As duas estão em `config_f.csv`, que o `avaliar_justo` lê.

Caminho da escolha:

| Passo | Melhor até aqui | Hit@5 | Hit@10 |
| --- | --- | --- | --- |
| 1 | w = (1; 0; 0,5; 1), K = 100, estampa 0,1 | 0,858% | 1,218% |
| 2 | w = (1; 0; 0,5; 1), K = 100, estampa 0,1 | 0,858% | 1,218% |
| 3 | w = (1; 0; 0,5; 1), K = 100, estampa 0,1 | 0,858% | 1,218% |
| 4 | w = (1; 0; 0,5; 1), K = 100, estampa 0,1 | 0,858% | 1,218% |

## Variações na validação

| Versão | Configuração | Hit@1 | Hit@5 | Hit@10 | FITB | Obs. |
| --- | --- | --- | --- | --- | --- | --- |
| F (escolhida) | w = (1; 0; 0,5; 1), K = 100, estampa 0,1 | 0,515% | 0,858% | 1,218% | 30,76% |  |
| C + popularidade | w = (1; 0; 0; 1) | 0,527% | 0,922% | 1,175% | 30,14% |  |
| C (sem desempate, como na tabela corrigida) | w = (1; 0; 0; 0) | 0,596% | 0,862% | 0,926% | 25,45% |  |
| Só C | w = (1; 0; 0; 0) | 0,656% | 0,892% | 1,175% | 30,14% |  |
| Só pontos por atributo | w = (0; 1; 0; 0), atributos (0; 1; 0,5; 1; 1; 0) | 0,000% | 0,043% | 0,069% | 28,54% | pesos de atributo da F |
| Só grupos | w = (0; 0; 1; 0), K = 100, estampa 0,1 | 0,000% | 0,043% | 0,056% | 29,70% | K = 100, estampa 0,1 |
| Só popularidade | w = (0; 0; 0; 1) | 0,026% | 0,309% | 0,686% | 29,74% |  |
| F sem C | w = (0; 0; 0,5; 1), K = 100, estampa 0,1 | 0,013% | 0,287% | 0,695% | 30,37% |  |
| F sem grupos | w = (1; 0; 0; 1) | 0,527% | 0,922% | 1,175% | 30,14% |  |
| F sem popularidade | w = (1; 0; 0,5; 0), K = 100, estampa 0,1 | 0,643% | 0,858% | 0,858% | 30,08% |  |

"Só popularidade" é a versão Popularidade. "C (sem desempate)" é a C da tabela corrigida. "Só C" é a mesma C com o desempate pela popularidade.

## Diferenças com intervalo de 95% (bootstrap, validação)

Mesmo bootstrap da avaliação justa: 1.000 sorteios dos looks com reposição. Se o intervalo não passa pelo 0, a diferença é real. Se passa, pode ser sorte. pp = pontos percentuais.

| Comparação | Métrica | Diferença | Intervalo de 95% | Conclusão |
| --- | --- | --- | --- | --- |
| F (escolhida) x Só popularidade | Hit@1 | 0,489 pp | 0,232 a 0,789 pp | ganha (diferença real) |
| F (escolhida) x Só popularidade | Hit@5 | 0,549 pp | 0,206 a 0,952 pp | ganha (diferença real) |
| F (escolhida) x Só popularidade | Hit@10 | 0,532 pp | 0,142 a 1,003 pp | ganha (diferença real) |
| F (escolhida) x Só popularidade | FITB | 1,02 pp | 0,23 a 1,86 pp | ganha (diferença real) |
| F (escolhida) x C (sem desempate, como na tabela corrigida) | Hit@1 | −0,081 pp | −0,244 a 0,051 pp | pode ser sorte |
| F (escolhida) x C (sem desempate, como na tabela corrigida) | Hit@5 | −0,004 pp | −0,142 a 0,137 pp | pode ser sorte |
| F (escolhida) x C (sem desempate, como na tabela corrigida) | Hit@10 | 0,292 pp | 0,077 a 0,557 pp | ganha (diferença real) |
| F (escolhida) x C (sem desempate, como na tabela corrigida) | FITB | 5,30 pp | 4,07 a 6,62 pp | ganha (diferença real) |
| F (escolhida) x C + popularidade | Hit@1 | −0,013 pp | −0,039 a 0,000 pp | pode ser sorte |
| F (escolhida) x C + popularidade | Hit@5 | −0,064 pp | −0,193 a 0,000 pp | pode ser sorte |
| F (escolhida) x C + popularidade | Hit@10 | 0,043 pp | 0,000 a 0,129 pp | pode ser sorte |
| F (escolhida) x C + popularidade | FITB | 0,62 pp | −0,13 a 1,44 pp | pode ser sorte |
| C + popularidade x Só popularidade | Hit@1 | 0,502 pp | 0,244 a 0,815 pp | ganha (diferença real) |
| C + popularidade x Só popularidade | Hit@5 | 0,613 pp | 0,262 a 1,033 pp | ganha (diferença real) |
| C + popularidade x Só popularidade | Hit@10 | 0,489 pp | 0,086 a 0,943 pp | ganha (diferença real) |
| C + popularidade x Só popularidade | FITB | 0,40 pp | 0,15 a 0,70 pp | ganha (diferença real) |
| C + popularidade x C (sem desempate, como na tabela corrigida) | Hit@1 | −0,069 pp | −0,232 a 0,073 pp | pode ser sorte |
| C + popularidade x C (sem desempate, como na tabela corrigida) | Hit@5 | 0,060 pp | −0,034 a 0,193 pp | pode ser sorte |
| C + popularidade x C (sem desempate, como na tabela corrigida) | Hit@10 | 0,249 pp | 0,043 a 0,497 pp | ganha (diferença real) |
| C + popularidade x C (sem desempate, como na tabela corrigida) | FITB | 4,68 pp | 3,76 a 5,65 pp | ganha (diferença real) |
| F sem grupos x F (escolhida) | Hit@1 | 0,013 pp | 0,000 a 0,039 pp | pode ser sorte |
| F sem grupos x F (escolhida) | Hit@5 | 0,064 pp | 0,000 a 0,193 pp | pode ser sorte |
| F sem grupos x F (escolhida) | Hit@10 | −0,043 pp | −0,129 a 0,000 pp | pode ser sorte |
| F sem grupos x F (escolhida) | FITB | −0,62 pp | −1,44 a 0,13 pp | pode ser sorte |
