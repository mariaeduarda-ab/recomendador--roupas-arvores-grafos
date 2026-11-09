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
| O peso da estampa é 0,1, ou seja, estampa diferente vale como uma diferença de cor Delta E 10. | No grafo, sem looks: com 0 a estampa some, com 0,2 ou mais ela domina, e 0,1 fica no meio. | Sim, depois, no `ajustar_f` (0; 0,1; 0,2). Com K = 100, os pesos 0,1 e 0,2 empataram em todas as métricas. O 0,1 ficou porque vem antes na grade. |
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

### Formas de agrupar para o Gephi e as articulações

| Decisão | Motivo | Testada na validação? |
| --- | --- | --- |
| A forma escolhida é a de menor média do maior grupo, com no máximo 20% de grupos de 1 peça. O critério foi escrito e commitado antes de rodar. | Ela pediu a forma mais equilibrada. Assim o critério não foi ajustado depois de ver os números. | Não. Os grupos não usam looks. |
| A forma escolhida foi a 3: grupo_cor + estampa (lisa, estampada, desconhecida) + faixa de formalidade, sem ocasião. | A ocasião pode ter vários valores marcados ao mesmo tempo (32 combinações) e picaria os grupos. Resultado: maior grupo de 17,6% em média e 9,1% de grupos de 1 peça. | Não. Os grupos não usam looks. |
| A forma 1 para em K = 50 ou quando nenhuma união cabe no limite de 5%, que tem no mínimo 2 peças. | 50 é o valor do meio dos K testados. Com menos de 40 peças, 5% daria menos de 2 e ninguém se uniria. | Não. |
| Os 3 pesos da estampa não foram testados para os grupos do Gephi. | A forma 3 não usa o vetor de cor e estampa. Os 3 pesos são testados no Kruskal puro da F (agente 4). | Não se aplica. |
| No Gephi da forma 3, as arestas são as mútuas do kNN dentro de cada grupo. | A forma 3 não tem arestas próprias, e essas desenham cada grupo. | Não se aplica. |

### Versão F e ajuste na validação (agente 4)

| Decisão | Motivo | Testada na validação? |
| --- | --- | --- |
| O desempate pela popularidade entrou como parâmetro opcional de `avaliar_notas`. | Assim o desempate é exato, sem somar um valor pequeno à nota. As outras versões não mudam. | Sim. As versões antigas repetiram a `validacao_corrigida`. |
| O termo de atributo é guardado por valor da candidata. | A conta só depende do valor da candidata. Isso deixa a grade rápida (pico de 190 MB). Um teste confere que o resultado é a mesma soma por peça. | Não se aplica. |
| A grade tem 4 passos: os w, os pesos de atributo um de cada vez, os w de novo e os pesos de novo. Os 9 tipos de grupo (K = 20, 50, 100 e estampa 0; 0,1; 0,2) entram no primeiro passo. | A grade cheia é grande demais. São 623 combinações, em `grade_f.csv`. | Sim. |
| Critério de escolha: maior Hit@10, depois maior Hit@5, depois mais zeros, depois mais pesos iguais a 1. | Foi combinado antes de ver os números. Pesos (1; 1) e (0,5; 0,5) dão a mesma lista. | Sim. |
| Os grupos só ficam se a melhor combinação com grupos ganhar da melhor sem grupos pelo critério acima. | É a regra dela aplicada com o critério da grade. | Sim. Os grupos ganharam por 1 consulta. |
| Quando w2 = 0, os pesos de atributo são ajustados na melhor combinação com w2 > 0. | Sem isso, os pesos nunca seriam testados. | Sim. Mesmo assim os atributos não ganharam. |
| A "C + popularidade" foi ajustada numa grade de w1 e w4 em (0; 0,5; 1). | Foi o que ela pediu. O resultado foi w1 = w4 = 1. | Sim. |
| No teste final, as tabelas, os grupos e a popularidade usam todos os looks de treino, inclusive os de validação. | É o que a C, a D e as outras versões fazem no teste. | Não se aplica. |

### Decisão dela antes do teste final (05/11)

| Decisão | Motivo | Testada na validação? |
| --- | --- | --- |
| A F testada é C + 0,5 × grupos (K = 100, estampa 0,1) + popularidade. Os pontos por atributo ficaram com peso 0. | Ela escolheu seguir o critério combinado antes. Na validação, a diferença para a C + popularidade é de 1 consulta e pode ser sorte. Isso vai dito no artigo. | Sim. |

### Articulações com Tarjan (análise depois do congelamento)

| Decisão | Motivo | Testada na validação? |
| --- | --- | --- |
| O grafo do Tarjan usa os grupos da forma 3 e os pontos de `PontosGrupos` com todos os looks de treino. | É o grafo entre grupos que a F usa no teste final. | Não se aplica. Não muda nenhuma recomendação. |
| A aresta só entra com n ≥ 3 pares de peças. | Com 1 ou 2 pares, a aresta pode vir de um look só. Sem o corte, 39628 das 50836 arestas positivas teriam n = 1 ou 2. | Não se aplica. |
| Os 3 limiares são os quartis dos pontos das arestas positivas com n ≥ 3 (0,223; 0,437; 0,705). | Cada um guarda 75%, 50% e 25% dessas arestas. São tirados da distribuição, sem olhar as articulações. | Não se aplica. |
| A tabela completa usa a mediana. Os outros limiares aparecem em forma curta. | É o limiar do meio. | Não se aplica. |
| Grupos separados = grupos fora do maior pedaço quando a articulação sai. Só contam as que separam 3 ou mais. | É o pedido dela (ignorar as que soltam 1 ou 2 grupos). | Não se aplica. |
| Arestas paralelas contam como arestas diferentes no Tarjan, e laços são ignorados. | Duas arestas entre os mesmos vértices nunca são ponte. No grafo dos grupos não há paralelas, mas os testes cobrem o caso. | Não se aplica. |
| Os exemplos de peças são as 3 do grupo que mais aparecem nos looks de treino. | São as peças que mais pesam nos pontos do grupo. | Não se aplica. |
| O tempo do Tarjan é a média de 1000 execuções no mesmo grafo. | Uma execução leva menos de 1 ms e sairia com muito ruído. | Não se aplica. |

## Últimas melhorias (até 13/11)

### Etapa 1: peças gêmeas

| Decisão | Motivo | Testada na validação? |
| --- | --- | --- |
| A impressão digital é categoria + as 3 cores com cada canal dividido por 16 + legenda do BLIP. | O JSON não tem código de produto, nome nem preço. O passo 16 foi escolhido antes de medir. | Medida só na validação. |
| **Erro registrado:** antes do prompt de 09/11, a primeira versão do script mediu a cobertura também nas escondidas do teste (27,4% para 28,6%). | O pedido anterior dizia para medir no teste. A regra nova proíbe. O script foi trocado para medir só na validação, e o número do teste não foi usado em nenhuma escolha. | Não se aplica. |
| As gêmeas não entram. | Só 24 de 50 pares conferidos pela imagem são a mesma peça, abaixo dos 45 exigidos. Na validação, as peças com alguma peça da classe no treino iriam de 26,0% para 27,2%. | Sim (cobertura) e pela imagem. |
| A variante sem legenda (passo 8) foi medida, mas as imagens dela não foram conferidas. | Ela foge da impressão digital pedida e junta mais peças diferentes. Na validação, a cobertura chega a 32,8%. | Só a cobertura. |

### Etapa 2: variações para a peça nova (`ajustar_novas`)

| Decisão | Motivo | Testada na validação? |
| --- | --- | --- |
| As metades A e B são um sorteio das 2.332 consultas de validação (semente 2711), com 1.166 em cada. | Cada consulta é um look, então dividir as consultas é dividir os looks. | Não se aplica. |
| "Ganhar" é ter mais acertos somados nas 10 ordens que a F atual, na metade A e na metade B. Os intervalos de bootstrap da metade B vão no relatório, mas não decidem. | É a regra do prompt. | Sim. |
| Recuo dos grupos da forma 3: com menos de 20 pares no treino, usa os grupos de categoria + cor. Com menos de 20 também neles, o ponto é 0. | O ponto de "só a categoria" é sempre 0, porque o esperado é igual à contagem. | Sim. Sozinha, a forma 3 teve Hit@10 menor que a F na metade A (120 contra 131 acertos). |
| No peso por grau, o fator a / (1 + grau) multiplica os atributos e os grupos. C e popularidade ficam sem fator. a fica em 0,5; 1; 2; 5 ou 10. | É a fórmula do prompt. A peça nova tem grau 0 e recebe o fator a. | Sim. |
| Regressão logística com 13 colunas: C, popularidade, os 6 atributos separados, grupos do Kruskal, grupos da forma 3, atributos / (1 + grau), forma 3 / (1 + grau) e "peça nova". Os dados são pares da metade A: a certa e 10 sorteadas da mesma categoria (semente 31). São 3.000 passos de descida de gradiente em lote, com taxa 1 e L2 0,001. | Escrita em C++ no próprio projeto, sem biblioteca de aprendizado. A coluna "peça nova" deixa o modelo dar um bônus à peça sem look. | Sim. Ela ganhou na metade A (181 contra 131), mas perdeu na B (−0,026 pp, pode ser sorte). |
| Na metade A, a melhor pelo Hit@10 foi forma 3 + peso por grau, com w = (0; 0,5; 0,5; 0) e a = 0,5. Ela não vai ao teste. | Ganhou na A (190 contra 131 acertos), mas perdeu na B (116 contra 153). Nenhuma variação ganhou da F no Hit@10 nas duas metades. | Sim. |
| A configuração pelo FITB foi acrescentada depois, como o prompt pede: forma 3 + peso por grau, com w = (1; 0; 0,5; 1) e a = 0,5. | Foi a de maior FITB na metade A (39,79% contra 30,39% da F) e ganhou na B (40,77% contra 31,13%). Ela perde no Hit@10 nas duas metades (120 contra 131 e 140 contra 153). | Sim. |
| A configuração pelo FITB vai ao teste uma única vez, com o nome "F2 (FITB)", pelo `avaliar_justo --novas`. A `tabela_final.md` da F não muda. | O teste já foi usado uma vez pela F. Este é o segundo uso, e ele fica registrado aqui. | Não se aplica. |
