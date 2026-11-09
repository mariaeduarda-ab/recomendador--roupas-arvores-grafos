# Recomendador de roupas com árvores e grafos

Trabalho final da disciplina AEDS 2 (CEFET-MG). O sistema encontra peças de roupa parecidas com uma KD-tree e completa looks com um grafo de peças e looks. O programa é escrito em C++17 e usa só a biblioteca padrão.

A explicação completa está em [docs/ARQUITETURA.md](docs/ARQUITETURA.md). A versão final (F), a avaliação justa e os resultados finais estão em [docs/VERSAO_F.md](docs/VERSAO_F.md). As decisões tomadas durante o trabalho estão em [docs/DECISOES.md](docs/DECISOES.md). A ordem das tarefas está em [docs/ROTEIRO.md](docs/ROTEIRO.md).

## Como compilar e rodar um exemplo

Precisa só de `g++` (C++17) e `make`. Não precisa baixar nada para o exemplo.

```
make -j4        # compila (cerca de 45 s com 1 núcleo, menos com -j4)
make test       # 170 testes
make demo       # exemplo pequeno, cerca de 1 s depois de compilado
```

O `make demo` usa `dados/demo/`, um pedaço da base real que já vem no repositório: os 826 looks das 60 peças mais usadas, com 2.891 peças. Ele mostra peças parecidas, completa um look, roda a avaliação de todas as versões, as tabelas de pontos, as formas de agrupar e as articulações. As tabelas vão para `resultados/demo/`. Os números do exemplo não são os do artigo, porque a base é pequena e escolhida pelas peças mais usadas.

Testado em 09/11/2026 num clone limpo, seguindo só este README:

| Sistema | Compilador | make |
| --- | --- | --- |
| Ubuntu 22.04.5 LTS | g++ 11.4.0 | GNU Make 4.3 |

Para rodar outro comando no exemplo, use `--dados dados/demo`, por exemplo `./recomendador --dados dados/demo parecidas 5 3`.

## Base completa

Os números do artigo usam a base completa (102.370 peças), que não fica no repositório. Para gerar os CSVs, veja [dados/README.md](dados/README.md). Os scripts em Python precisam de Python 3 com `numpy` e `matplotlib` (`preprocess/requirements.txt`).

## Comandos

```
./recomendador parecidas <id_peca> <k>          # k peças parecidas da mesma categoria
./recomendador completar <id_look> <categoria>  # completa um look (versão C)
./recomendador avaliar                          # Hit@k das versões e tempos
./recomendador gephi <k>                        # grupos com esse k e arquivos do Gephi
./recomendador avaliar_d                        # escolhe a config da versão D na validação e compara todas (cerca de 30 min)
./recomendador reavaliar                        # popularidade, B, C e D com a config já escolhida
./recomendador completar_d <id_look> <categoria> # completa um look (versão D)
./recomendador montar <id_peca> [categorias]    # 3 melhores looks a partir de uma peça
./recomendador validacao                        # lista os looks de validação
./recomendador vetor_d                          # escolhe na validação o vetor da D com pecas_v4.csv
./recomendador estampa_d                        # valida a regra de estampa na D nova
./recomendador avaliar_e                        # escolhe os pesos da E e roda o teste
./recomendador avaliar_fitb                     # prova de múltipla escolha com todas as versões
./recomendador completar_e <id_look> <categoria> # completa um look (versão E)
./recomendador pontos                           # tabelas de pontos por atributo (pecas_v5.csv)
./recomendador grupos [K ...]                   # grupos com Kruskal por categoria (padrão: 20 50 100)
./recomendador grupos_formas                    # compara 3 formas de agrupar e grava o Gephi da escolhida
./recomendador ajustar_f                        # escolhe os pesos da F só na validação (cerca de 14 min)
./recomendador avaliar_justo [--validacao]      # todas as versões na categoria inteira, 10 desempates, bootstrap
./recomendador articulacoes                     # Tarjan: articulações e pontes no grafo entre grupos
./recomendador tempos_f                         # tempos das estruturas para 1 mil, 10 mil e 100 mil peças
make tempos_f                                   # refaz os tempos e os gráficos da versão F
```

`avaliar_justo` sem `--validacao` roda o teste. Ele já foi rodado uma vez para a tabela final (tag `v-final`). Rodar de novo só repete os mesmos números.

`completar_d`, `montar` e `reavaliar` usam `resultados/config_d.csv`. Sem esse arquivo, avisam e usam a configuração inicial.

## Sequência para refazer tudo

```
python3 preprocess/preprocessar.py --entrada Combined_Metadata.json --saida dados
python3 preprocess/atributos.py --entrada Combined_Metadata.json --saida dados
make
make test
./recomendador avaliar
./recomendador avaliar_d
python3 preprocess/ordem.py <pasta temporária>
python3 preprocess/graficos.py
```

Versão F (depois da sequência acima):

```
python3 preprocess/atributos.py --entrada Combined_Metadata.json --saida <pasta temporária>
python3 preprocess/atributos_finais.py --texto <pasta temporária> --ids dados/pecas.csv --saida dados   # gera pecas_v4.csv
python3 preprocess/grupos_cor.py --saida dados/pecas_v5.csv
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

Os scripts em Python precisam do que está em `preprocess/requirements.txt`.

Use `--ajuste` para dividir a contagem da BFS pela popularidade da peça. Use `--dados <pasta>` para escolher a pasta dos CSVs. O padrão é `dados/`. Para usar os dados pequenos de exemplo, escreva `--dados dados/exemplo`. Com outra pasta de dados, as tabelas vão para `resultados/<nome da pasta>/`, e as da base real não são sobrescritas.

## Testes

```
make test
```

