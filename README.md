# Recomendador de roupas com árvores e grafos

Trabalho final da disciplina AEDS 2 (CEFET-MG). O sistema encontra peças de roupa parecidas com uma KD-tree e completa looks com um grafo de peças e looks. O programa é escrito em C++17 e usa só a biblioteca padrão.

A explicação completa está em [docs/ARQUITETURA.md](docs/ARQUITETURA.md). A ordem das tarefas está em [docs/ROTEIRO.md](docs/ROTEIRO.md).

## Como compilar

```
make
```

Precisa só de `g++` e `make`, no Ubuntu 24.04.

## Como baixar os dados

Veja [dados/README.md](dados/README.md).

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
```

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

Os scripts em Python precisam do que está em `preprocess/requirements.txt`.

Use `--ajuste` para dividir a contagem da BFS pela popularidade da peça. Use `--dados <pasta>` para escolher a pasta dos CSVs. O padrão é `dados/`. Para usar os dados pequenos de exemplo, escreva `--dados dados/exemplo`. Com outra pasta de dados, as tabelas vão para `resultados/<nome da pasta>/`, e as da base real não são sobrescritas.

## Testes

```
make test
```

