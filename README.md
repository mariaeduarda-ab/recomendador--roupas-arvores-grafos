# Recomendador de roupas com árvores e grafos

Trabalho final da disciplina AEDS 2 (CEFET-MG). O sistema encontra peças de roupa parecidas com uma KD-tree e completa looks com um grafo de peças e looks. O programa é escrito em C++17 e usa só a biblioteca padrão.

A explicação completa está em [docs/ARQUITETURA.md](docs/ARQUITETURA.md). A ordem das tarefas está em [docs/ROTEIRO.md](docs/ROTEIRO.md).

## Como compilar

```
make
```

Precisa só de `g++` e `make`, no Ubuntu 24.04.

## Como baixar os dados

A base tratada fica no Zenodo. O link ainda é um TODO. Veja [dados/README.md](dados/README.md).

## Comandos

```
./recomendador parecidas <id_peca> <k>          # k peças parecidas da mesma categoria
./recomendador completar <id_look> <categoria>  # completa um look (versão C)
./recomendador avaliar                          # Hit@k das versões e tempos
./recomendador gephi <k>                        # grupos com esse k e arquivos do Gephi
```

Use `--dados <pasta>` para escolher a pasta dos CSVs. O padrão é `dados/`. Para usar os dados pequenos de exemplo, escreva `--dados dados/exemplo`.

## Testes

```
make test
```

Os testes das estruturas falham até as estruturas serem implementadas. Isso é esperado.
