# Recomendador de roupas com árvores e grafos

[![Validação Ubuntu 24.04](https://github.com/mariaeduarda-ab/recomendador--roupas-arvores-grafos/actions/workflows/ci-ubuntu24.yml/badge.svg)](https://github.com/mariaeduarda-ab/recomendador--roupas-arvores-grafos/actions/workflows/ci-ubuntu24.yml)

O sistema completa looks. Ele recebe um look sem uma peça e a categoria da peça que falta, dá uma nota a todas as peças dessa categoria e devolve a lista ordenada. A peça certa deve ficar entre as primeiras. Tudo é feito com estruturas de dados clássicas, sem rede neural e sem treino, e cada nota pode ser explicada pelo caminho que a produziu.

O programa é escrito em C++17 e usa só a biblioteca padrão.

## Principais resultados

Base FashionVerse-MME, com 102.370 peças e 31.333 looks. Teste com 5.842 looks e, em média, 10.940 candidatas por consulta.

| Versão | Hit@10 | FITB |
| --- | --- | --- |
| Aleatória | 0,182% | 25,00% |
| Popularidade | 0,895% | 31,40% |
| C (KD-tree e looks em comum) | 0,751% | 25,37% |
| **F (versão final)** | **1,056%** | **32,04%** |

- A versão F acerta a peça certa entre as 10 primeiras **5,8 vezes** mais que o sorteio.
- Quando a peça já apareceu em looks de treino, o acerto sobe para **3,856%**, **23 vezes** o sorteio.
- A análise mostra um **viés de popularidade estrutural**. A informação dos looks traz o acerto, mas favorece as peças que já têm histórico. Esse viés nasce da forma do grafo, e não de um aprendizado.
- Os tempos medidos seguem a teoria. Em 100 mil peças, a busca na KD-tree é **14 vezes** mais rápida que a força bruta, e o Union-Find com união por posto e compressão de caminho é **37 mil vezes** mais rápido que a versão sem as técnicas.

Todos os números saem dos arquivos da pasta [`resultados/`](resultados/).

## Como funciona

```
FashionVerse-MME (JSON)
  → pré-processamento: junta registros repetidos, converte a cor para Lab, divide treino, validação e teste
  → atributos das peças (texto e CLIP) e 12 grupos de cor com k-means
  → estruturas: KD-tree, grafo de peças e looks, Union-Find, Kruskal e tabelas de pontos
  → versões A a F: nota para todas as peças da categoria
  → validação escolhe os pesos, teste roda uma vez
  → tabelas, gráficos de tempo, Gephi e articulações
```

| Estrutura | Papel no sistema | Custo |
| --- | --- | --- |
| KD-tree | Acha as peças mais parecidas em cor Lab e formalidade, sem comparar com todas | Construção O(n log n), busca O(log n) em média |
| Grafo bipartido de peças e looks com BFS de 2 passos | Conta os looks em comum entre duas peças | O(V + E) |
| Union-Find (posto e compressão de caminho) | Testa se duas peças já estão no mesmo grupo | O(α(n)) por operação |
| Kruskal | Forma os grupos de estilo e para em K grupos por categoria | O(E log E) |
| Tarjan | Acha as articulações e as pontes entre grupos | O(V + E) |

### Versões

| Versão | Nota da candidata |
| --- | --- |
| A | Semelhança com o vetor médio do look (KD-tree) |
| B | Looks em comum com as peças do look (BFS de 2 passos) |
| C | Looks em comum com as peças do look e com as 5 vizinhas de cada uma na KD-tree |
| D | Atributos do texto e bônus dos looks |
| F | Nota da C, pontos entre grupos e popularidade, com pesos escolhidos na validação |

A explicação completa de cada estrutura e de cada versão está em [docs/ARQUITETURA.md](docs/ARQUITETURA.md) e [docs/VERSAO_F.md](docs/VERSAO_F.md).

## Como compilar e rodar

Precisa só de `g++` com C++17 e `make`. O exemplo não precisa baixar nada.

```
make -j4        # compila
make test       # roda os 170 testes
make demo       # roda o exemplo pequeno com a base real, cerca de 1 s
```

O `make demo` usa a pasta `dados/demo/`, que já vem no repositório: 826 looks reais com 2.891 peças. Ele mostra peças parecidas, completa um look, avalia as versões, gera as tabelas de pontos, os grupos e as articulações. As tabelas vão para `resultados/demo/`. Os números do exemplo não são os do artigo, porque a base é menor.

| Sistema | Compilador | make |
| --- | --- | --- |
| Ubuntu 24.04 | g++ do sistema | GNU Make |
| Ubuntu 22.04.5 LTS | g++ 11.4.0 | GNU Make 4.3 |

O Ubuntu 24.04 é conferido a cada envio para o repositório pelo workflow [`ci-ubuntu24.yml`](.github/workflows/ci-ubuntu24.yml), que compila, roda os testes e roda o exemplo.

## Comandos principais

```
./recomendador parecidas <id_peca> <k>           # k peças parecidas da mesma categoria
./recomendador completar <id_look> <categoria>   # completa um look
./recomendador montar <id_peca> [categorias]     # 3 melhores looks a partir de uma peça
./recomendador avaliar_justo [--validacao]       # todas as versões na categoria inteira, 10 desempates e bootstrap
./recomendador articulacoes                      # Tarjan: articulações e pontes no grafo entre grupos
./recomendador tempos_f                          # tempos das estruturas para 1 mil, 10 mil e 100 mil peças
```

Use `--dados <pasta>` para escolher a pasta dos CSVs. O padrão é `dados/`. Para o exemplo, use `--dados dados/demo`, como em `./recomendador --dados dados/demo parecidas 5 3`.

<details>
<summary>Todos os comandos</summary>

```
./recomendador avaliar                          # Hit@k das versões e tempos
./recomendador avaliar_d                        # escolhe a configuração da versão D na validação (cerca de 30 min)
./recomendador reavaliar                        # popularidade, B, C e D com a configuração escolhida
./recomendador completar_d <id_look> <categoria> # completa um look com a versão D
./recomendador vetor_d                          # escolhe na validação o vetor da D
./recomendador estampa_d                        # valida a regra de estampa da D
./recomendador avaliar_e                        # escolhe os pesos da E e roda o teste
./recomendador completar_e <id_look> <categoria> # completa um look com a versão E
./recomendador avaliar_fitb                     # prova de múltipla escolha com todas as versões
./recomendador validacao                        # lista os looks de validação
./recomendador pontos                           # tabelas de pontos por atributo
./recomendador grupos [K ...]                   # grupos com Kruskal por categoria (padrão: 20 50 100)
./recomendador grupos_formas                    # compara 3 formas de agrupar e grava os arquivos do Gephi
./recomendador gephi <k>                        # grupos com esse k e arquivos do Gephi
./recomendador exportar_grupos                  # grava os grupos de cada peça
./recomendador ajustar_f                        # escolhe os pesos da F na validação (cerca de 14 min)
./recomendador ajustar_novas                    # testa variações para a peça nova na validação
make tempos_f                                   # refaz os tempos e os gráficos
```

`avaliar_justo` sem `--validacao` roda o teste, que foi usado uma vez para a tabela final (tag `v-final`).

</details>

## Base completa e reprodução

Os números do artigo usam a base completa (102.370 peças), que não fica no repositório. Para gerar os CSVs a partir do `Combined_Metadata.json`, veja [dados/README.md](dados/README.md). Os scripts em Python precisam do que está em [`preprocess/requirements.txt`](preprocess/requirements.txt).

```
python3 preprocess/preprocessar.py --entrada Combined_Metadata.json --saida dados
python3 preprocess/atributos.py --entrada Combined_Metadata.json --saida <pasta temporária>
python3 preprocess/atributos_finais.py --texto <pasta temporária> --ids dados/pecas.csv --saida dados
python3 preprocess/grupos_cor.py --saida dados/pecas_v5.csv
make
./recomendador vetor_d
./recomendador pontos
./recomendador grupos
./recomendador grupos_formas
./recomendador ajustar_f
./recomendador avaliar_justo --validacao
./recomendador avaliar_justo
./recomendador articulacoes
make tempos_f
```

O pacote com a base tratada para o Zenodo é gerado por `preprocess/zenodo.py`. Ele traz os dados gerados pelo projeto e o identificador original de cada peça, sem as imagens. A base de origem usa a licença CC BY 4.0, mas as imagens da Polyvore podem exigir outra permissão.

## Organização do repositório

| Pasta | Conteúdo |
| --- | --- |
| `src/` | Código em C++ das estruturas, das versões e das avaliações |
| `testes/` | Os 170 testes |
| `preprocess/` | Scripts em Python de preparo dos dados, atributos e gráficos |
| `dados/` | Exemplos pequenos, a base do `make demo` e as 400 peças de referência rotuladas pela imagem |
| `resultados/` | Tabelas, gráficos e relatórios de cada experimento |
| `docs/` | Arquitetura, versão final, decisões e roteiro |
| `artigo/` | Artigo em LaTeX (modelo IEEE) |
| `zenodo/` | Instruções do pacote da base tratada |

## Documentação

- [docs/ARQUITETURA.md](docs/ARQUITETURA.md): cada estrutura, com exemplos pequenos.
- [docs/VERSAO_F.md](docs/VERSAO_F.md): a versão final, a avaliação justa e os resultados.
- [docs/DECISOES.md](docs/DECISOES.md): cada decisão do projeto, o motivo e se foi testada na validação.
- [docs/ROTEIRO.md](docs/ROTEIRO.md): a ordem das tarefas.

## Autores

Maria Eduarda Alves Barbosa e Igor Moreira Lopes

