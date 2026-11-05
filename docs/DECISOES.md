# Decisões tomadas sem perguntar

Cada linha diz o que foi decidido, o motivo e se a escolha foi testada na validação. "Não se aplica" quer dizer que a decisão não muda nenhum resultado de acerto (por exemplo, um nome de arquivo ou um jeito de medir tempo).

## Antes da versão F (05/11)

| Decisão | Motivo | Testada na validação? |
| --- | --- | --- |
| `pecas_v4.csv` mantém o comprimento do texto. | A opção 2 manda manter os atributos do texto. A D não usa o comprimento. | Não. A D não usa esse atributo. |
| O vetor da KD-tree da D usa só a ocasião. | Ganhou das 8 combinações de estilo, ocasião e formalidade (7 acertos contra 6 sem nenhum). | Sim (`vetor_d`). A diferença é de 1 look. |
| A regra de estampa da D fica desligada (c = 0). | Ligada, caiu de 7 para 5 acertos. | Sim (`estampa_d`). |
| O trabalho de 05/11 entrou como os primeiros commits de `versao-f`. | O prompt da F pede commits nesse branch, e a F parte desse trabalho. | Não se aplica. |

## Versão F

### Dados

| Decisão | Motivo | Testada na validação? |
| --- | --- | --- |
| `grupo_cor` foi gravado em `dados/pecas_v5.csv`, e não no `pecas_v4.csv`. | O prompt também pede para não sobrescrever dados, e o v4 já existia. As 20 primeiras colunas são iguais às do v4. | Não se aplica. |
| A cor usa L/100 e (a + 128)/100. | O CSV guarda L/100 e (a + 128)/255. Multiplicar a e b por 2,55 põe os três canais na mesma unidade do Lab, dividida por 100, e a distância vira o Delta E/100. As constantes são fixas. Nenhum canal é normalizado pelos próprios dados, e o deslocamento de 128 não muda distâncias. | Não se aplica. É só a unidade da cor. |

### Avaliação justa (etapa 1)

| Decisão | Motivo | Testada na validação? |
| --- | --- | --- |
| As 10 "ordens das linhas" são 10 permutações fixas das peças (sementes 1 a 10), usadas só no desempate. | Para as notas, mudar a ordem das linhas só muda quem vem antes entre as empatadas. Assim a nota é calculada uma vez por consulta. | Não se aplica. |
| A e D também dão nota à categoria inteira. A nota da A é 1/(1 + distância até a média do look). A D usa a própria nota em todas as peças. | A etapa 1.1 pede todas as peças da categoria. As listas curtas antigas aparecem como linhas extras. | Não se aplica. Só muda a forma de medir. |
| No bootstrap, o acerto de cada consulta é a média das 10 ordens. São 1.000 reamostragens com semente 2026, com as mesmas amostras em todas as comparações. | Assim a margem de erro não mistura a sorte do desempate com a sorte dos looks. | Não se aplica. |
| No FITB, um empate entre as 4 opções é decidido pela ordem sorteada. As perguntas são as mesmas do `avaliar_fitb` (sementes 13 no teste e 11 na validação). | É o mesmo desempate do Hit@k. A média das 10 ordens fica perto do valor antigo. | Não se aplica. |

### Grupos com Kruskal (etapa 3)

| Decisão | Motivo | Testada na validação? |
| --- | --- | --- |
| O vetor do grupo é a cor Lab (na unidade acima) mais a estampa: lisa = 0, estampada = 1, desconhecida = 0,5. | O 0,5 não puxa a peça para nenhum dos lados. | Não. |
| O peso da estampa é 0,1, ou seja, estampa diferente vale como uma diferença de cor Delta E 10. | No grafo, sem looks: com 0 a estampa some, com 0,2 ou mais ela domina, e 0,1 fica no meio. | Ainda não. Ela pediu um teste com 3 valores na validação. |
| As arestas são ordenadas por (peso, u, v). | Assim o resultado sai igual em toda execução. | Não se aplica. |
| Só o Gephi com K = 100 entra no git. | Cada K gera cerca de 2 MB. Os outros saem com `./recomendador grupos`. | Não se aplica. |

### Tempos (etapa 6)

| Decisão | Motivo | Testada na validação? |
| --- | --- | --- |
| Os 3 tamanhos usam dados reais (`pecas_N.csv`), sem vetores aleatórios. | Os arquivos de 100 mil já existiam. | Não se aplica. |
| O grafo kNN dos tempos usa todas as peças, sem separar por categoria. | Assim o n é o mesmo em todas as medidas. | Não se aplica. |
| No Union-Find, a sequência une a peça 0 com cada peça e depois faz n buscas sorteadas. | Sem as técnicas, isso forma uma corrente de n elementos, que é o pior caso que a aula mostra. | Não se aplica. |
| A montagem da lista do Prim é medida à parte, como a ordenação do Kruskal. | Assim os dois algoritmos são comparados só no laço principal. | Não se aplica. |

### Pontos por atributo (etapa 2)

| Decisão | Motivo | Testada na validação? |
| --- | --- | --- |
| O k-means da cor usa semente 42 e início k-means++. O nome de cada grupo é a cor de referência mais perto. Quando um nome se repete, ganha número, e o 1 é o mais escuro. | É um resultado reprodutível. Nenhum grupo ficou como jeans, laranja ou roxo. | Não. O k-means não usa looks. |
| A fração de um valor é a parte dos pares de treino A-B em que a peça de A tem esse valor. | Assim a soma dos esperados é igual à soma dos n. | Não. |
| Estilo e ocasião com vários valores: a peça reparte o peso 1 entre os valores marcados. Nenhum valor marcado conta como desconhecido. | É o mesmo peso na contagem e na nota. | Não. |
| A formalidade é baixa abaixo de 1/3, média entre 1/3 e 2/3 e alta acima de 2/3. O valor 0,5 conta como desconhecido. | No v4 não dá para separar o empate da falta de palavra. Segue `comparacao_fontes.md`. | Não. |
| O comprimento só vale para parte de cima, parte de baixo e peça única. | O atributo não se aplica a sapato, bolsa e acessórios. | Não. |
| Duas peças da mesma categoria no mesmo look ficam fora das contas, e o par vale 0. | A tarefa sempre pede uma categoria que ainda não está no look. | Não. |
| Um par de categorias com menos de 200 pares usa a tabela dos grupos de categoria. Parte de cima: top e outerwear. Parte de baixo: pants, skirt e legwear. Peça única: dress e jumpsuit. Calçado: shoes. Bolsa: bag. Acessório: o resto. | O limite de 200 vem do prompt. A meia-calça vai junto com a parte de baixo. | Não. Só 2,5% dos pares da validação usaram a tabela do grupo. |
| `explicar` some com os termos abaixo de 0,005 e ordena pelo valor absoluto. | Assim o texto fica curto. | Não se aplica. |
