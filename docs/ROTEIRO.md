# Roteiro de implementação

As tarefas estão na ordem em que cada parte depende da anterior. Prazo de entrega: 22/11/2026.

Para ver o que já passa, rode `make test`.

## 1. Conferir a base

- [ ] Abrir uma amostra da FashionVerse e conferir o formato e a qualidade
- Responsável:
- Arquivos: nenhum. Anotar os nomes dos campos para a tarefa 2.
- Pronto quando: 50 peças forem conferidas à mão. Os campos e a licença (CC BY 4.0) já foram conferidos.

## 2. Pré-processamento

- [x] Completar o `preprocessar.py` e gerar os CSVs
- Responsável:
- Arquivos: `preprocess/preprocessar.py`, `dados/`
- Pronto quando: `pecas.csv`, `looks.csv` e os subconjuntos existirem e `./recomendador parecidas 0 5` ler a base sem erro de leitura.

## 3. Union-Find

- [x] Union-Find + testes
- Responsável:
- Arquivos: `src/union_find.hpp`
- Pronto quando: os testes `Union-Find` passarem.

## 4. KD-tree

- [x] KD-tree (construção, vizinho mais próximo, kNN, força bruta) + testes
- Responsável:
- Arquivos: `src/kdtree.hpp`, `src/kdtree.cpp`
- Pronto quando: os testes `KD-tree` passarem, inclusive o teste contra a força bruta.

## 5. Grafo de similaridade

- [x] Grafo kNN mútuo + grupos + exportação Gephi
- Responsável:
- Arquivos: `src/grafo_similaridade.cpp`, `src/gephi.cpp`
- Pronto quando: os testes `kNN mútuo` passarem e `./recomendador gephi 10` gerar arquivos que abrem no Gephi.

## 6. Grafo peça–look

- [x] Grafo peça–look + BFS de 2 passos + testes
- Responsável:
- Arquivos: `src/grafo_looks.cpp`
- Pronto quando: os testes `BFS` passarem.

## 7. Recomendação

- [x] Versões A, B, C e popularidade
- Responsável:
- Arquivos: `src/recomendador.cpp`
- Pronto quando: os testes `Peso` e `Versão C` passarem e `./recomendador completar 5 saia --dados dados/exemplo` listar saia cinza, saia jeans e saia preta, nessa ordem.

## 8. Avaliação

- [x] Avaliação (Hit@k) e testes de tempo
- Responsável:
- Arquivos: `src/avaliacao.cpp`, `resultados/`
- Pronto quando: `./recomendador avaliar` gravar `resultados/hit_k.csv` e `resultados/tempos.csv` com valores da base real.

## 9. Artigo

- [ ] Gráficos e tabelas para o artigo
- Responsável:
- Arquivos: `resultados/`, Overleaf
- Pronto quando: a tabela de Hit@k, o gráfico de tempo, a tabela de grupos e o desenho do Gephi estiverem no artigo.

## 10. Publicação

- [ ] Publicar a base tratada no Zenodo e colocar os links no artigo
- Responsável:
- Arquivos: `README.md`, `dados/README.md`
- Pronto quando: a licença estiver confirmada, os TODO de link estiverem preenchidos e outra pessoa conseguir baixar e rodar com os passos do README.
