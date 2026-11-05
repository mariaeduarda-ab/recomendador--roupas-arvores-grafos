# Tempos das estruturas (versao F)

Comando que refaz tudo: `make tempos_f` (roda `nice -n 0 ./recomendador tempos_f` e depois `python3 preprocess/graficos_tempos_f.py`).
CSVs e graficos em `resultados/tempos_f/`.

## Como foi medido

- Dados reais: `pecas_N.csv` e `looks_N.csv` (N = 1.000, 10.000 e 100.000). Nao usamos vetores aleatorios. O vetor da peca tem 5 numeros (L, a, b, variacao de cor, formalidade).
- Cada medida roda 5 vezes. A tabela mostra a media e o desvio padrao das 5.
- kNN e BFS: tempo por consulta, media de 1.000 consultas sorteadas com semente fixa.
- Kruskal e Prim usam o grafo kNN de todas as pecas (k = 10), com peso igual a distancia entre os vetores.
- Maquina com 8 nucleos e outros programas rodando. Carga (media de 1 minuto) no inicio: 1,94. No fim: 2,02 (`carga.csv`).
- Uma rodada anterior, com carga de 0,94 a 2,17, deu tempos ate 40% menores em parte das medidas (por exemplo, construir a KD-tree) e quase iguais em outras (Kruskal e Prim em n = 100.000). Os valores absolutos variam com a carga. A forma das curvas nao muda.

## Tabela (ms, media de 5 repeticoes)

| Medida | n = 1.000 | n = 10.000 | n = 100.000 | Curva esperada |
| --- | ---: | ---: | ---: | --- |
| Construir a KD-tree | 0,352 | 3,49 | 68,2 | n log n |
| kNN na KD-tree (por consulta) | 0,0083 | 0,0190 | 0,107 | log n no caso medio |
| kNN na forca bruta (por consulta) | 0,0172 | 0,142 | 1,99 | n |
| Union-Find sem as tecnicas | 4,09 | 389 | 35.239 | n² nesta sequencia |
| Union-Find so com tamanho | 0,0096 | 0,0893 | 0,887 | n |
| Union-Find so com compressao | 0,0150 | 0,145 | 1,44 | n |
| Union-Find com tamanho e compressao | 0,0103 | 0,0985 | 0,987 | n |
| Montar o grafo kNN (k = 10) | 9,05 | 208 | 10.217 | n log n no caso medio |
| Kruskal: ordenar as arestas | 0,495 | 6,37 | 65,3 | n log n |
| Kruskal: laco com Union-Find | 0,074 | 0,766 | 8,34 | n |
| Prim: montar a lista de adjacencia | 0,414 | 5,26 | 87,7 | n |
| Prim com heap | 0,737 | 10,6 | 112 | n log n |
| BFS de 2 passos (por consulta) | 0,000023 | 0,000085 | 0,00041 | nao depende de n |

Os desvios estao nos CSVs. Os maiores desvios relativos sao os da BFS (ate 30% da media, porque cada consulta leva menos de 1 microssegundo) e do Union-Find sem as tecnicas em n = 100.000 (10%).

## O que os numeros mostram

- **KD-tree contra forca bruta.** Em n = 100.000 a KD-tree responde 19 vezes mais rapido. Em n = 1.000 a diferenca e de so 2 vezes. A busca na KD-tree cresce mais que log n (13 vezes de 1.000 para 100.000). O motivo e que muitas pecas tem o mesmo valor de formalidade e de cor, e a busca precisa visitar mais galhos quando ha muitos pontos iguais.
- **Union-Find.** A sequencia une a peca 0 com a 1, depois a 0 com a 2, e assim por diante, e depois faz n buscas sorteadas. Sem as duas tecnicas, a raiz antiga sempre fica embaixo da peca nova. A peca 0 fica no fim de uma corrente de n elementos e cada busca anda a corrente inteira. O tempo multiplica por 100 quando n multiplica por 10, como n². Com qualquer uma das tecnicas a corrente nao se forma e o tempo cresce como n. Em n = 100.000 a diferenca e de 35 mil vezes. A classe do projeto (`union_find.hpp`, por posto e compressao) fica em 0,76 ms nesse n.
- **Kruskal.** Ordenar as arestas custa de 7 a 8 vezes mais que o laco com o Union-Find. A ordenacao e a parte cara, como diz a analise O(E log E).
- **Prim contra Kruskal.** As duas florestas tem o mesmo peso nos 3 tamanhos (45,56; 252,11; 1.381,19) e o mesmo numero de arestas (`agm_conferencia.csv`). Kruskal (ordenar + laco) foi 1,3 a 1,5 vezes mais rapido que Prim com heap, sem contar a lista de adjacencia. O grafo kNN e esparso (cerca de 6,5 arestas por peca), e nesse caso os dois tem o mesmo custo O(E log E).
- **Componentes.** O grafo kNN das 3 bases saiu conexo (1 componente). O reinicio do Prim em cada componente so e exercitado nos testes, com um grafo desconexo.
- **Montar o grafo kNN.** E a parte mais cara de tudo (10 s em n = 100.000), porque faz uma busca kNN por peca. Cresce mais que n log n pelo mesmo motivo da busca na KD-tree.
- **BFS de 2 passos.** O custo depende do numero de looks da peca e do tamanho desses looks, nao de n. Nos subconjuntos ele cresce porque as pecas sao sorteadas: em n = 1.000 quase todo look de treino tem so 1 peca da amostra, e em n = 100.000 os looks ficam com 3 a 5 pecas. Mesmo assim cada consulta leva menos de 1 microssegundo.

## Arquivos

- `kdtree.csv`, `union_find.csv`, `agm.csv`, `bfs.csv`: colunas n, medida, media_ms, desvio_ms.
- `agm_conferencia.csv`: arestas do grafo, peso e arestas da floresta de Kruskal e de Prim, numero de componentes.
- `carga.csv`: carga da maquina no inicio e no fim (`/proc/loadavg`).
- `kdtree.png`, `union_find.png`, `agm.png`, `bfs.png`: tempo por n em escala log-log. A linha tracejada de cada serie e a curva teorica, ajustada para passar pelo primeiro ponto.
