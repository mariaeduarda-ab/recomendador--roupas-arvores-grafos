# Tempos das estruturas (versao F)

Comando que refaz tudo: `make tempos_f` (roda `nice -n 0 ./recomendador tempos_f` e depois `python3 preprocess/graficos_tempos_f.py`).
CSVs e graficos em `resultados/tempos_f/`.

## Como foi medido

- Dados reais: `pecas_N.csv` e `looks_N.csv` (N = 1.000, 10.000 e 100.000). Nao usamos vetores aleatorios. O vetor da peca tem 5 numeros (L, a, b, variacao de cor, formalidade).
- Cada medida roda 5 vezes. A tabela mostra a media e o desvio padrao das 5.
- kNN e BFS: tempo por consulta, media de 1.000 consultas sorteadas com semente fixa.
- Kruskal e Prim usam o grafo kNN de todas as pecas (k = 10), com peso igual a distancia entre os vetores.
- Maquina com 8 nucleos, Ubuntu 22.04, g++ 11.4. Rodada de 09/11, so com o navegador aberto. Carga (media de 1 minuto) no inicio: 0,94. No fim: 1,22 (`carga.csv`).
- A rodada de 05/11, com carga perto de 2, deu tempos de 10% a 40% maiores. Os valores absolutos variam com a carga. A forma das curvas nao muda.

## Tabela (ms, media de 5 repeticoes)

| Medida | n = 1.000 | n = 10.000 | n = 100.000 | Curva esperada |
| --- | ---: | ---: | ---: | --- |
| Construir a KD-tree | 0,231 | 2,88 | 49,0 | n log n |
| kNN na KD-tree (por consulta) | 0,0072 | 0,0130 | 0,0948 | log n no caso medio |
| kNN na forca bruta (por consulta) | 0,0117 | 0,0787 | 1,34 | n |
| Union-Find sem as tecnicas | 3,46 | 298 | 29.204 | n² nesta sequencia |
| Union-Find so com tamanho | 0,0077 | 0,0694 | 0,715 | n |
| Union-Find so com compressao | 0,0127 | 0,107 | 1,12 | n |
| Union-Find com tamanho e compressao | 0,0091 | 0,0792 | 0,781 | n |
| Montar o grafo kNN (k = 10) | 6,57 | 147 | 8.012 | n log n no caso medio |
| Kruskal: ordenar as arestas | 0,354 | 4,33 | 51,7 | n log n |
| Kruskal: laco com Union-Find | 0,050 | 0,498 | 6,52 | n |
| Prim: montar a lista de adjacencia | 0,310 | 3,53 | 65,1 | n |
| Prim com heap | 0,558 | 8,53 | 106 | n log n |
| BFS de 2 passos (por consulta) | 0,000017 | 0,000067 | 0,00037 | nao depende de n |

Os desvios estao nos CSVs. Os maiores desvios relativos sao os da BFS (ate 30% da media, porque cada consulta leva menos de 1 microssegundo) e do Union-Find sem as tecnicas em n = 100.000 (10%).

## O que os numeros mostram

- **KD-tree contra forca bruta.** Em n = 100.000 a KD-tree responde 14 vezes mais rapido. Em n = 1.000 a diferenca e de so 1,6 vez. A busca na KD-tree cresce mais que log n (13 vezes de 1.000 para 100.000). O motivo e que muitas pecas tem o mesmo valor de formalidade e de cor, e a busca precisa visitar mais galhos quando ha muitos pontos iguais.
- **Union-Find.** A sequencia une a peca 0 com a 1, depois a 0 com a 2, e assim por diante, e depois faz n buscas sorteadas. Sem as duas tecnicas, a raiz antiga sempre fica embaixo da peca nova. A peca 0 fica no fim de uma corrente de n elementos e cada busca anda a corrente inteira. O tempo multiplica por 100 quando n multiplica por 10, como n². Com qualquer uma das tecnicas a corrente nao se forma e o tempo cresce como n. Em n = 100.000 a diferenca e de 37 mil vezes. A classe do projeto (`union_find.hpp`, por posto e compressao) fica em 0,63 ms nesse n.
- **Kruskal.** Ordenar as arestas custa de 7,1 a 8,7 vezes mais que o laco com o Union-Find. A ordenacao e a parte cara, como diz a analise O(E log E).
- **Prim contra Kruskal.** As duas florestas tem o mesmo peso nos 3 tamanhos (45,56; 252,11; 1.381,19) e o mesmo numero de arestas (`agm_conferencia.csv`). Kruskal (ordenar + laco) foi 1,4 a 1,8 vez mais rapido que Prim com heap, sem contar a lista de adjacencia. O grafo kNN e esparso (cerca de 6,5 arestas por peca), e nesse caso os dois tem o mesmo custo O(E log E).
- **Componentes.** O grafo kNN das 3 bases saiu conexo (1 componente). O reinicio do Prim em cada componente so e exercitado nos testes, com um grafo desconexo.
- **Montar o grafo kNN.** E a parte mais cara de tudo (8 s em n = 100.000), porque faz uma busca kNN por peca. Cresce mais que n log n pelo mesmo motivo da busca na KD-tree.
- **BFS de 2 passos.** O custo depende do numero de looks da peca e do tamanho desses looks, nao de n. Nos subconjuntos ele cresce porque as pecas sao sorteadas: em n = 1.000 quase todo look de treino tem so 1 peca da amostra, e em n = 100.000 os looks ficam com 3 a 5 pecas. Mesmo assim cada consulta leva menos de 1 microssegundo.

## Arquivos

- `kdtree.csv`, `union_find.csv`, `agm.csv`, `bfs.csv`: colunas n, medida, media_ms, desvio_ms.
- `agm_conferencia.csv`: arestas do grafo, peso e arestas da floresta de Kruskal e de Prim, numero de componentes.
- `carga.csv`: carga da maquina no inicio e no fim (`/proc/loadavg`).
- `kdtree.png`, `union_find.png`, `agm.png`, `bfs.png`: tempo por n em escala log-log. A linha tracejada de cada serie e a curva teorica, ajustada para passar pelo primeiro ponto.
