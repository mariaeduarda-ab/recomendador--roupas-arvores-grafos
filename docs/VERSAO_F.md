# Versão F

Este texto explica a versão F do recomendador: por que a D falhou, o que a F faz, onde cada conceito da disciplina aparece no código, os resultados e as limitações. Todo número sai de um arquivo em `resultados/`, com o nome indicado ao lado.

A tarefa é sempre a mesma. Um look perde uma peça, e o sistema precisa achar essa peça entre todas as peças da mesma categoria. Hit@10 é a fração de looks em que a peça escondida fica entre as 10 primeiras. FITB é uma pergunta de 4 opções da mesma categoria, e o chute acerta 25%.

## 1. Por que a D falhou

A D tinha dois problemas: empates demais e uma lista curta demais.

**Empates.** A D usava uma KD-tree com poucos atributos, e muitas peças tinham o mesmo vetor. Na validação, o melhor vetor (só a ocasião) acertou 7 de 2.332 looks no Hit@10 (`validacao_vetor_d.csv`). Com tão poucos acertos, a escolha dependia do acaso. Quando duas peças têm a mesma nota, o desempate usa a linha da peça no catálogo. Por isso, só ao mudar a ordem das linhas do arquivo, sem mudar nenhum dado, a D foi de 6 a 13 acertos no Hit@10 do teste (`avaliacao_d.md`, tabela "Sensibilidade à ordem do catálogo").

**Lista curta.** A D só dava nota às peças ligadas por aresta a alguma peça do look. Isso cobria 1,90% das peças escondidas (`avaliacao_d.md`). Todas as outras ficavam com nota 0 e empatavam entre si. Na avaliação justa, a mesma D passou a dar nota à categoria inteira, e o Hit@10 dela foi de 0,154% para 0,659% (`tabela_corrigida.md`, linhas "D (só lista curta)" e "D").

Desde então todas as versões dão nota a todas as peças da categoria, e cada resultado é a média de 10 ordens sorteadas de desempate.

## 2. Pontos por atributo

A ideia é contar quais valores de atributo aparecem juntos nos looks de treino mais do que o acaso explicaria. Para um par de categorias (bolsa e sapato, por exemplo) e um par de valores (rosa e rosa):

- `n` é o número de pares de peças com esses valores no mesmo look de treino;
- `esperado` é quantos pares haveria se a cor da bolsa e a cor do sapato fossem independentes;
- `pontos = log((n + 1) / (esperado + 1))`, com log natural.

O +1 evita divisão por zero e puxa para perto de 0 os casos com poucas contagens. Exemplos reais de `tabelas_pontos.md` (atributo cor):

| lado A | lado B | n | esperado | pontos | leitura |
| --- | --- | --- | --- | --- | --- |
| bolsa rosa | sapato rosa | 47 | 3,2 | 2,43 | aparecem juntas muito mais que o esperado |
| casaco bege | saia cinza 1 | 0 | 10,0 | −2,40 | era esperado 10 vezes e nunca aparece |

A conta do primeiro: log(48 / 4,2) ≈ 2,44. A tabela mostra 2,43 porque o esperado está arredondado. A do segundo: log(1 / 11) ≈ −2,40.

A nota de uma candidata soma os pontos dela com cada peça do look, nos 6 atributos (cor, estampa, formalidade, estilo, ocasião e comprimento).

**O que aconteceu na validação.** Os pontos por atributo não melhoraram o Hit@10 e ficaram com peso 0 na F (`config_f.csv`, coluna `w_atributos`). Sozinhos, eles tiveram Hit@10 de 0,069%, abaixo do sorteio esperado de 0,179% (`escolha_final.md`, tabela "Variações na validação", e `validacao_final.md`). Eles ajudam no FITB: sozinhos acertaram 28,54% contra 25% do chute (`escolha_final.md`). Na grade da F, a melhor combinação com peso de atributo maior que 0 chegou a 43,36% de FITB, mas com Hit@10 de 0,815% (`grade_f.csv`, linha 54). A melhor com atributo no Hit@10 ficou em 1,115%, abaixo do 1,218% da F (`grade_f.csv`, linhas 519 e 336). Como o critério combinado antes era o Hit@10, os atributos ficaram de fora.

Uma leitura possível: os atributos ajudam a separar a peça certa de 3 peças sorteadas, mas não bastam para colocá-la entre as 10 primeiras de cerca de 10.835 candidatas (`validacao_final.md`). Isso é uma interpretação e não foi testado à parte.

## 3. Conceitos da disciplina no projeto

| Conceito | Onde usamos | Arquivo | Comando |
| --- | --- | --- | --- |
| KD-tree | Busca das peças parecidas na versão C e montagem do grafo kNN (k = 10) dos grupos | `src/kdtree.cpp`, `src/recomendador.cpp`, `src/grupos_kruskal.cpp` | `./recomendador avaliar_justo` |
| BFS | BFS de 2 passos no grafo peça–look: peça, looks dela, outras peças desses looks. É a base das versões B, C e F | `src/grafo_looks.cpp` (`bfs_2_passos`) | `./recomendador avaliar_justo` |
| Union-Find | Guarda os grupos durante o Kruskal. Usa união por posto e compressão de caminho | `src/union_find.hpp`, `src/grupos_kruskal.cpp` | `./recomendador grupos` |
| Kruskal (agrupamento) | Une as arestas mais curtas do grafo kNN de cada categoria e para quando restam K grupos (K = 100 na F) | `src/grupos_kruskal.cpp` (`agrupar`) | `./recomendador grupos` |
| Grafo de condensação | Cada grupo vira um vértice. Os pontos passam a ser contados entre grupos e não entre peças (`PontosGrupos`). O Tarjan também roda nesse grafo de grupos | `src/grupos_kruskal.cpp`, `src/grupos_formas.cpp` | `./recomendador grupos`, `./recomendador grupos_formas` |
| Grau do vértice | Popularidade = log(1 + grau), em que o grau é o número de looks de treino da peça no grafo peça–look (exercício 21 da lista do professor) | `src/versao_f.cpp` (`pop_`) | `./recomendador ajustar_f` |
| Tarjan | Articulações e pontes no grafo entre grupos, com DFS e pilha explícita, O(V + E) | `src/tarjan.cpp`, `src/articulacoes.cpp` | `./recomendador articulacoes` |
| Comparação de tempos | Prim x Kruskal, Union-Find com e sem as técnicas, KD-tree x força bruta, BFS de 2 passos, em 1.000, 10.000 e 100.000 peças | `src/tempos_f.cpp` | `make tempos_f` |

Alguns números dos tempos (`tempos_f.md`, média de 5 repetições):

- Com 100.000 peças, o kNN na KD-tree leva 0,095 ms por consulta e a força bruta 1,34 ms, 14 vezes mais.
- O Union-Find sem as técnicas leva 29.204 ms com 100.000 peças. Com tamanho e compressão, leva 0,781 ms.
- Kruskal e Prim dão florestas com o mesmo peso nos 3 tamanhos. Kruskal (ordenar + laço) foi 1,4 a 1,8 vez mais rápido que Prim com heap. Ordenar as arestas custa de 7,1 a 8,7 vezes o laço do Kruskal (`tempos_f/agm.csv`).

## 4. Montar o look inteiro

**[CONFERIR]**

Montar um look inteiro é escolher uma peça por categoria de modo que a soma das notas entre todos os pares seja a maior possível. No caso geral, com muitas categorias e muitas candidatas, esse problema é NP-difícil. A ideia é que, se cada par vale 0 ou 1, achar um look em que todos os pares valem 1 é procurar uma clique com uma peça de cada categoria, e o problema da clique é NP-completo.

A transformação é esta. Recebemos um grafo G = (V, E) e um número k, e a pergunta da clique é se G tem k vértices todos ligados entre si. Criamos k categorias, e cada categoria é uma cópia dos vértices de G: a peça (i, u) é o vértice u na categoria i. Duas peças de categorias diferentes, (i, u) e (j, v), têm nota 1 se u e v estão ligados em G e nota 0 nos outros casos. Como nenhum vértice está ligado a si mesmo, um look com todas as notas iguais a 1 usa k vértices diferentes, todos ligados entre si, ou seja, uma clique de tamanho k. O contrário também vale: uma clique de tamanho k vira um look com uma peça por categoria e todas as notas iguais a 1. A transformação cria k × |V| peças e no máximo k² × |V|² notas, então ela custa tempo polinomial. Por isso o nosso problema é pelo menos tão difícil quanto a clique.

No nosso caso, um look de treino tem em média 4,0 peças, de 20 categorias possíveis (média calculada sobre `dados/looks.csv`, categorias em `tabelas_pontos.md`). Se cada categoria ficar só com as 10 primeiras candidatas da F, um look de 4 categorias tem 10^4 = 10.000 combinações com 6 pares cada (60.000 somas), e mesmo 6 categorias dariam 10^6 = 1.000.000 de combinações com 15 pares cada. As notas dos pares podem ser calculadas uma vez antes do laço (10 × 10 × 15 = 1.500 notas com 6 categorias), então cada combinação custa só algumas somas e testar todas é viável.

**[CONFERIR]**

## 5. Resultados e limitações

### Tabela final (teste, 5.842 looks)

Fonte: `tabela_final.md`. Média de 10 ordens de desempate. A configuração da F foi escolhida só na validação (`escolha_final.md`). Cada termo vai de 0 a 1 dentro da consulta e F = C + 0,5 × pontos dos grupos do Kruskal (K = 100, peso da estampa 0,1) + popularidade.

| Versão | Hit@1 | Hit@5 | Hit@10 | FITB |
| --- | --- | --- | --- | --- |
| Aleatória (esperado) | 0,018% | 0,091% | 0,182% | 25,00% |
| Aleatória | 0,027% | 0,096% | 0,187% | 24,92% |
| Popularidade | 0,142% | 0,568% | 0,895% | 31,40% |
| A | 0,051% | 0,188% | 0,514% | 34,29% |
| B | 0,373% | 0,647% | 0,772% | 25,35% |
| C | 0,336% | 0,633% | 0,751% | 25,37% |
| D | 0,329% | 0,531% | 0,659% | 28,14% |
| D nova | 0,312% | 0,575% | 0,688% | 27,26% |
| C + popularidade | 0,313% | 0,746% | 1,058% | 31,63% |
| **F** | 0,313% | 0,716% | 1,056% | 32,04% |
| D (só lista curta) | 0,015% | 0,072% | 0,154% | 24,86% |
| D nova (só lista curta) | 0,021% | 0,082% | 0,193% | 24,85% |

### Diferenças da F com intervalo de 95% (teste)

Bootstrap com 1.000 sorteios dos looks (`tabela_final.md`). pp = pontos percentuais. Se o intervalo não passa pelo 0, a diferença é real. Se passa, pode ser sorte.

| Comparação | Métrica | Diferença | Intervalo de 95% | Conclusão |
| --- | --- | --- | --- | --- |
| F x Popularidade | Hit@1 | 0,171 pp | 0,024 a 0,330 pp | F ganha |
| F x Popularidade | Hit@5 | 0,147 pp | −0,058 a 0,359 pp | pode ser sorte |
| F x Popularidade | Hit@10 | 0,161 pp | −0,068 a 0,383 pp | pode ser sorte |
| F x Popularidade | FITB | 0,63 pp | 0,14 a 1,11 pp | F ganha |
| F x C | Hit@1 | −0,022 pp | −0,116 a 0,068 pp | pode ser sorte |
| F x C | Hit@5 | 0,082 pp | −0,068 a 0,245 pp | pode ser sorte |
| F x C | Hit@10 | 0,305 pp | 0,113 a 0,510 pp | F ganha |
| F x C | FITB | 6,67 pp | 5,90 a 7,43 pp | F ganha |
| F x C + popularidade | Hit@1 | 0,000 pp | 0,000 a 0,000 pp | empate |
| F x C + popularidade | Hit@5 | −0,031 pp | −0,084 a 0,007 pp | pode ser sorte |
| F x C + popularidade | Hit@10 | −0,002 pp | −0,015 a 0,010 pp | pode ser sorte |
| F x C + popularidade | FITB | 0,41 pp | −0,05 a 0,86 pp | pode ser sorte |

### O que a tabela diz

- **F contra C.** A F ganha da C de verdade no Hit@10 e no FITB. No Hit@1 e no Hit@5 a diferença pode ser sorte.
- **F contra a popularidade.** A F ganha de verdade no Hit@1 e no FITB. No Hit@10 a diferença pode ser sorte. Na validação, `escolha_final.md` e `validacao_final.md` dizem que a F ganha da popularidade no Hit@10 (0,532 pp, de 0,142 a 1,003 pp). Esse resultado vale só para a validação, que foi onde a F foi escolhida. A escolha favorece a F nesses mesmos looks. No teste, a diferença no Hit@10 é de 0,161 pp, com intervalo de −0,068 a 0,383 pp, e pode ser sorte.
- **F contra C + popularidade.** As duas empatam. No Hit@1 a diferença é exatamente 0. Nas outras métricas o intervalo passa pelo 0. Ou seja, os grupos do Kruskal não acrescentaram nada mensurável. Na validação, a vantagem da F foi de 1 consulta em 2.332 no Hit@10 (1,218% contra 1,175%, `escolha_final.md`), e a dona escolheu manter os grupos pelo critério combinado antes de ver os números (`DECISOES.md`).
- **B e C.** No teste final a B fica acima da C nas 3 métricas de Hit (0,373%, 0,647% e 0,772% contra 0,336%, 0,633% e 0,751%). Isso contraria a avaliação antiga, em que a C ficava acima. As diferenças B x C no Hit@5 e no Hit@10 podem ser sorte (`tabela_final.md`).
- **Onde a F perde.** A F fica abaixo da B e da C no Hit@1 (0,313% contra 0,373% e 0,336%) e abaixo da A no FITB (32,04% contra 34,29%). Não calculamos intervalo para essas comparações da F com B e com A.
- **Peso da estampa.** Na validação, com K = 100, os pesos de estampa 0,1 e 0,2 empataram em todas as métricas (`grade_f.csv`, linhas 336 e 498). O 0,1 ficou porque vem antes na ordem da grade.
- Todos os valores de Hit@10 são baixos, perto de 1%. Cada consulta tem em média 10.940 candidatas (`tabela_final.md`).

### Limitações

- **Peças novas.** Em 72,6% dos looks de teste (4.242 de 5.842), a peça escondida não aparece em nenhum look de treino (`diagnostico.md`). Para essas peças, a parte que vem dos looks (B, C, popularidade) não tem nada a dizer.
- **Efeito corrente do Kruskal.** No Kruskal puro, as arestas mais curtas são unidas primeiro, e o grupo grande sempre acha mais uma peça perto. Com K = 50, o maior grupo tem em média 83,4% das peças da categoria (`grupos_formas.md`). Com K = 100, que é o da F, 16 das 20 categorias têm um grupo com mais da metade das peças (`grupos_kruskal.md`). Por isso o Gephi e o Tarjan usam a forma 3 (mesma combinação de cor, estampa e formalidade): maior grupo de 17,6% em média e 9,1% de grupos com 1 peça. O critério foi escrito antes de rodar (`grupos_formas.md`, `DECISOES.md`).
- **Atributos fracos.** Na comparação com peças rotuladas pela imagem, a melhor fonte de cada atributo acerta de 51,3% (estilo) a 77,0% (estampa). Só a estampa passou do limite de 70% (`comparacao_fontes.md`, tabela "Decisão").
- **Tempos com carga.** A rodada final (09/11) teve só o navegador aberto, com carga média de 1 minuto de 0,94 no início e 1,22 no fim (`tempos_f/carga.csv`). A rodada de 05/11, com carga perto de 2, deu tempos de 10% a 40% maiores. Os valores absolutos mudam com a carga, a forma das curvas não (`tempos_f.md`).
- **Articulações rasas.** Em todas as 53 articulações da mediana, o maior pedaço que se solta tem 1 grupo (`articulacoes.md`). As articulações só seguram grupos pendurados por uma ligação, e não estilos inteiros.

## 6. Articulações

Uma articulação é um grupo que é a única ligação entre estilos. Se ele sai do grafo, algum conjunto de grupos fica sem caminho até o resto. Isso não quer dizer que ele é a peça mais versátil. A versatilidade é medida pelo grau, que conta com quantos grupos de outras categorias ele combina.

O grafo tem os 1.255 grupos da forma 3 como vértices. Uma aresta liga dois grupos de categorias diferentes que aparecem juntos em pelo menos 3 pares de peças, com pontos acima do limiar (`articulacoes.md`).

| limiar | pontos > | arestas | grupos com aresta | articulações | com 3+ separados | maior pedaço solto | pontes | tempo do Tarjan |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1º quartil | 0,223 | 8406 | 714 | 51 | 6 | 1 | 89 | 166 µs |
| mediana | 0,437 | 5604 | 711 | 53 | 6 | 1 | 96 | 126 µs |
| 3º quartil | 0,705 | 2802 | 684 | 66 | 7 | 1 | 108 | 84 µs |

As articulações que mais separam (mediana):

| # | grupo | descrição | grau | grupos separados |
| --- | --- | --- | --- | --- |
| 1 | 1120 | top branco lisa formalidade baixa | 67 | 10 |
| 2 | 946 | sapato preto liso formalidade alta | 82 | 8 |
| 3 | 4 | bolsa preta lisa formalidade alta | 66 | 8 |
| 4 | 1125 | top branco lisa formalidade alta | 64 | 4 |
| 5 | 603 | colar branco estampa desconhecida formalidade alta | 58 | 3 |
| 6 | 1127 | top branco estampada formalidade baixa | 48 | 3 |

O grupo de maior grau (sapato marrom liso, formalidade alta) tem grau 96 e separa só 2 grupos. Dos 10 grupos de maior grau, 8 são articulações, mas só 1 separa 3 grupos ou mais. Grau alto e articulação são coisas diferentes.

## 7. Perguntas que o professor pode fazer

**1. Por que Kruskal e não Prim para agrupar?**
O Kruskal une as arestas da mais curta para a mais longa. Se ele para quando restam K componentes, os K componentes já são os grupos. O Prim cresce uma árvore só, então seria preciso montar a árvore inteira e depois cortar as K − 1 arestas mais longas. Nos grafos dos tempos, os dois deram florestas com o mesmo peso (`tempos_f.md`).

**2. Por que pilha explícita no Tarjan?**
A DFS recursiva pode descer tão fundo quanto o número de vértices e estourar a pilha do programa num caminho longo. Com a pilha explícita isso não acontece. É o mesmo motivo do laço no `find` do Union-Find (`src/union_find.hpp`).

**3. Por que bootstrap?**
Os acertos são poucos (dezenas de looks) e a diferença entre duas versões depende de quais looks caíram no teste. O bootstrap sorteia os looks com reposição 1.000 vezes e mede a diferença em cada sorteio, sempre com as duas versões nos mesmos looks. Ele não exige supor uma distribuição normal.

**4. Por que o teste só rodou uma vez?**
Se o teste roda várias vezes e a gente escolhe pelo resultado, o teste vira validação e o número fica otimista. Os pesos da F foram fixados em `config_f.csv` na validação, e só depois a F foi ao teste. O teste já tinha sido usado para medir A a D, mas nenhum peso da F foi escolhido com base nele. As rodadas com outra ordem do catálogo (`avaliacao_d.md`) só mediram a fragilidade da D e não escolheram nada.

**5. Por que a F não usa os atributos?**
Na validação, nenhuma combinação com peso de atributo maior que 0 passou a F no Hit@10, que era o critério combinado antes. Eles melhoram o FITB, mas não o Hit@10 (seção 2).

**6. Quanto custa a KD-tree na prática e por que cresce mais que log n?**
De 1.000 para 100.000 peças, a busca kNN ficou 13 vezes mais lenta (0,0072 ms para 0,095 ms), enquanto log n cresce menos de 2 vezes. Muitas peças têm o mesmo valor de formalidade e de cor, e com muitos pontos iguais a busca precisa visitar mais galhos (`tempos_f.md`). Mesmo assim, com 100.000 peças ela é 14 vezes mais rápida que a força bruta.

**7. Como evitaram vazamento de teste?**
O grafo peça–look, a popularidade, as tabelas de pontos e os grupos usam só looks de treino. No ajuste, os looks de validação (semente 7) também saem de tudo isso. As peças escondidas são as mesmas em todas as versões e foram conferidas com `escondidas.csv`. Se um look de teste entrasse no grafo, a BFS acharia a peça escondida dentro do próprio look (`ARQUITETURA.md`).

**8. Por que os Hit@10 são tão baixos?**
Cada consulta tem em média 10.940 candidatas, e o sorteio acerta só 0,182% no Hit@10 (`tabela_final.md`). Além disso, em 72,6% dos looks de teste a peça escondida nunca apareceu no treino (`diagnostico.md`).

**9. Por que 10 ordens de desempate?**
Muitas peças têm a mesma nota, e quem vem antes no empate dependia da linha no arquivo. Com 10 ordens sorteadas e a média delas, o resultado deixa de depender da ordem do catálogo. A nota é calculada uma vez por consulta, e só o desempate muda.

**10. Os grupos do Kruskal valeram a pena?**
Para o acerto, não de forma mensurável. A F empata com a C + popularidade no teste, e na validação a diferença foi de 1 consulta. Os grupos foram mantidos pelo critério combinado antes. Eles servem para ver o grafo de estilos no Gephi e para a análise de articulações.

## 8. Últimas melhorias: gêmeas, peça nova e duas configurações

O maior problema é a peça nova. No teste, 4.242 das 5.842 peças escondidas (72,6%) não aparecem em nenhum look de treino. Nesta etapa, toda escolha usou só a validação, dividida em duas metades com semente fixa. A metade A escolhe e a metade B confirma. Uma variação só fica se ganhar da F nas duas metades.

### Peças gêmeas

O JSON da FashionVerse não tem código de produto, nome nem preço. A impressão digital testada foi categoria + as 3 cores arredondadas (passo 16) + legenda do BLIP. A junção usa uma tabela hash e o Union-Find. Ela criou 960 classes com mais de 1 peça. Na validação, as peças com alguma peça da classe no treino iriam de 26,0% para 27,2%. Pela imagem, só 24 dos 50 pares conferidos são a mesma peça, e a regra pedia 45. **As gêmeas não entraram.** Fonte: `resultados/gemeas.md`.

### Variações para a peça nova (validação)

Fonte: `resultados/novas_validacao.md`. Os números são o Hit@10 e o FITB nas metades A e B.

| Variação | Hit@10 A | Hit@10 B | FITB A | FITB B |
| --- | --- | --- | --- | --- |
| F atual | 1,123% | 1,312% | 30,39% | 31,13% |
| Grupos da forma 3 com recuo | 1,029% | 1,072% | 39,35% | 40,70% |
| Peso por grau (grupos do Kruskal) | 1,175% | 1,141% | 30,30% | 31,22% |
| Forma 3 + peso por grau | 1,630% | 0,995% | 32,32% | 35,09% |
| Regressão logística | 1,552% | 1,286% | 39,61% | 41,05% |

- **Hit@10:** nenhuma variação ganhou da F nas duas metades. As que subiram na metade A caíram na metade B. Por isso nenhuma configuração nova pelo Hit@10 foi ao teste.
- **FITB (critério acrescentado depois):** a de maior FITB na metade A foi forma 3 + peso por grau, com w = (1; 0; 0,5; 1) e a = 0,5. Ela ganhou da F no FITB nas duas metades (39,79% e 40,77%, contra 30,39% e 31,13%) e foi ao teste uma única vez com o nome "F2 (FITB)".
- **Regressão logística:** foi treinada por descida de gradiente, escrita no projeto, com pares da metade A. O maior peso foi o da coluna "peça nova" (2,14), seguido da cor (0,70), da popularidade (0,62) e da C (0,55).

### Teste (segundo uso, só a F2)

Fonte: `resultados/tabela_final_2.md`. A tabela da F (`tabela_final.md`) não mudou.

| Versão | Hit@1 | Hit@10 | FITB |
| --- | --- | --- | --- |
| Popularidade | 0,142% | 0,895% | 31,40% |
| C | 0,336% | 0,751% | 25,37% |
| F | 0,313% | 1,056% | 32,04% |
| F2 (FITB) | 0,308% | 1,048% | 41,64% |

F2 x F no FITB: +9,60 pp (de 8,48 a 10,70), diferença real. No Hit@10: −0,009 pp (de −0,022 a 0,002), pode ser sorte.

### Peça conhecida e peça nova (teste)

| Versão | Hit@10 conhecida | Hit@10 nova | FITB conhecida | FITB nova |
| --- | --- | --- | --- | --- |
| Aleatória (esperado) | 0,167% | 0,187% | 25,00% | 25,00% |
| Popularidade | 3,269% | 0,000% | 47,99% | 25,14% |
| A | 0,438% | 0,542% | 32,13% | 35,10% |
| C | 2,394% | 0,132% | 25,97% | 25,14% |
| D nova | 1,856% | 0,248% | 29,14% | 26,56% |
| F | 3,856% | 0,000% | 49,04% | 25,62% |
| F2 (FITB) | 3,825% | 0,000% | 53,54% | 37,15% |

- Na peça conhecida, a F acerta 23 vezes mais que o aleatório no Hit@10.
- Na peça nova, nenhuma versão com popularidade coloca a escondida no top 10. A peça nova tem grau 0, e qualquer peça conhecida fica na frente dela. Só a A e as duas D, que não usam popularidade, passam do aleatório (A: 2,9 vezes).
- No FITB, as 4 opções vêm da mesma categoria, então a peça nova compete com outras parecidas. Ali a F2 acerta 37,15% na peça nova, contra 25,62% da F.

### Perguntas novas que o professor pode fazer

**11. Por que as gêmeas não entraram?**
Só 24 de 50 pares juntados eram a mesma peça. A legenda do BLIP é genérica ("a black skirt"), então peças diferentes da mesma cor caem na mesma impressão digital. A regra combinada antes pedia 45.

**12. Por que a peça nova nunca entra no top 10?**
C e popularidade dão 0 para quem não tem look de treino. Na categoria inteira há milhares de peças conhecidas com nota maior. Só a nota por atributos e grupos poderia ajudar, mas nenhuma variação ganhou da F no Hit@10 nas duas metades da validação.

**13. Por que existem duas configurações?**
A F é a melhor pelo Hit@10, que é o critério principal. A F2 foi escolhida depois pelo FITB, e isso está registrado em `DECISOES.md`. As duas foram escolhidas só na validação, e o teste foi usado uma vez para cada uma.

**14. Por que dividir a validação em duas metades?**
Com tantas combinações testadas, alguma ganha só por sorte. A metade B não participa da escolha. A forma 3 com peso por grau mostra isso: subiu de 1,123% para 1,630% na metade A e caiu para 0,995% na metade B.
