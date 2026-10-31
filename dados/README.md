# Dados

A base completa não fica neste repositório.

## Como baixar

Enquanto a base tratada não está no Zenodo, gere os CSVs:

1. Baixe o `FashionVerse-MME.zip` do Mendeley Data e extraia o `Combined_Metadata.json`.
2. Rode `python3 preprocess/preprocessar.py --entrada Combined_Metadata.json --saida dados`.

Arquivos esperados:

```
dados/pecas.csv
dados/looks.csv
dados/pecas_1000.csv     dados/looks_1000.csv
dados/pecas_10000.csv    dados/looks_10000.csv
dados/pecas_100000.csv   dados/looks_100000.csv
```

## Formato

```
pecas.csv   id,categoria,L,a,b,variacao_cor,formalidade
looks.csv   look_id,peca_id,conjunto
```

O campo `conjunto` vale `treino` ou `teste`.

## Origem

A base original é a FashionVerse-MME (Mendeley Data, DOI 10.17632/6sx3pbsfvz.1). Os CSVs saem do script `preprocess/preprocessar.py`. Quem só vai rodar o projeto não precisa desse script.

## Dados de exemplo

A pasta `exemplo/` tem dados pequenos e inventados, usados nos testes.

- As peças 0 a 5 são os pontos A a F do guia de estudo, com x = formalidade e y = estampa.
- As peças 6 a 15 são as peças do exemplo da camisa preta. Os valores de x e y delas foram escolhidos para dar as distâncias 0,2 e 0,5 entre as camisas.
- Os looks 1 a 4 são os looks L1 a L4 do mesmo exemplo. O look 5 é um look de teste com a camisa preta de botão e a saia cinza.

A coluna `nome` só existe no exemplo. O programa aceita o arquivo com ou sem ela.
