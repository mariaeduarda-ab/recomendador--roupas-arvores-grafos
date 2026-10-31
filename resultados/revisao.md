# Revisão da versão D

Revisão independente, feita em 31/10/2026 no branch `versao-knn-por-categoria`, a partir do commit `fc03bb3`. As checagens rodaram numa cópia fora do repositório. Nenhum código de `src/` e nenhum dado foi alterado.

## Conclusão

**Nenhum problema invalida os números.** Os valores de Hit se repetiram por inteiro. Não há vazamento. D é pior que B, C e popularidade, e os textos dizem isso.

Duas ressalvas mudam como os números devem ser lidos:

1. O resultado de D depende da ordem das peças no CSV (Hit@10 de 6 a 12). Veja o item 3.
2. A causa principal do resultado ruim pode ser a regra de candidatas, e não a nota. Veja a seção "Decisão para vocês".

## Resumo

| Item | Situação |
| --- | --- |
| 1. Vazamento de teste e validação | OK |
| 2. KD-trees por categoria x força bruta | OK nas distâncias, com ressalva nos ids |
| 3. Números batem com nova execução | OK no Hit e nos tempos. PROBLEMA: falta avisar a sensibilidade à ordem |
| 4. Mesmo teste em todas as versões | OK |
| 5. Compila e roda do zero | OK no código. PROBLEMA no README dos dados, no README principal e no `requirements.txt` |
| 6. Honestidade | OK nas conclusões. PROBLEMAS pequenos de texto e de gráfico |
| Extra. Comandos `completar_d` e `montar` | PROBLEMA |

## 1. Vazamento: OK

Leitura do código:

- O grafo ignora look que não é de treino (`src/grafo_looks.cpp:10`).
- O bônus de grupo e as contagens também (`src/versao_d.cpp:127`).
- A popularidade sai do grafo (`src/recomendador.cpp:32`).
- A validação sorteia só looks de treino (`src/avaliacao_d.cpp:180`) e os tira do grafo e do bônus (`src/avaliacao_d.cpp:192-195`).
- `escolher_config` não recebe os looks de teste como consulta e roda antes de eles serem sorteados (`src/avaliacao_d.cpp:296` e `:308`).
- As normalizações usam só as peças, sem looks (`preprocess/preprocessar.py:115`, `:51`). Os atributos vêm do texto da peça.

Testes práticos:

| Teste | Resultado |
| --- | --- |
| Trocar as peças dos looks de teste entre si: `bonus_grupo`, em 2 milhões de duplas sorteadas | 0 diferenças |
| Mesma troca: `nota` em 200 mil sorteios e BFS e `num_looks` de todas as peças | 0 diferenças |
| Trocar as peças dos looks de validação entre si, na base sem validação | 0 diferenças |
| Controle: validação de volta no treino | 1.763.166 bônus mudam, então o teste consegue detectar |
| Rodar a validação inteira com os looks de teste trocados | `validacao_d.csv` e `config_d.csv` idênticos byte a byte |
| Treino por look, de novo em Python | 25.066 looks de treino, 6.267 de teste, 101.408 linhas de treino e 0 looks mistos |
| Ordem das peças x look (risco no desempate) | Distância mediana de 35.237 no mesmo look contra 30.222 ao acaso. Sem relação |

No teste final, os looks de validação voltam ao treino. Isso é correto: nenhum look de teste entra.

## 2. KD-tree x força bruta: OK, com ressalva

1.000 buscas aleatórias na base real, com peça, categoria e k (1, 5, 10 ou 20) sorteados, contra uma força bruta escrita à parte:

| Busca | Distâncias iguais | Mesmos ids |
| --- | --- | --- |
| Grupo 1 | 1.000 de 1.000 | **68 de 1.000** |
| Atributos misturados | 1.000 de 1.000 | 995 de 1.000 |

A busca está certa. Os ids diferem porque há muitos empates: as duas devolvem peças à mesma distância, mas nem sempre as mesmas peças. `estruturas_d.md` diz corretamente "mesmas distâncias". Na apresentação, não digam "mesmo resultado" sem explicar isso.

## 3. Nova execução: OK nos números, PROBLEMA na sensibilidade

Rodei `./recomendador avaliar && ./recomendador avaliar_d` numa cópia limpa.

- `hit_d.csv`, `hit_k.csv`, `diferencas_d.csv`, `validacao_d.csv`, `config_d.csv` e `escondidas.csv` ficaram **idênticos byte a byte**.
- Os números de `avaliacao_d.md` batem com esses arquivos. Conferi Hit, cobertura, 111 contra 38 e os intervalos.
- Os tempos mudaram, como esperado. Arestas com 100 mil peças: 132 s aqui contra 93 s na tabela. Nesta rodada havia outras checagens na mesma máquina, por isso a duração total foi de 40 min, e não 28 min.
- A KD-tree continuou cerca de 22 vezes mais rápida que a força bruta com 100 mil peças (0,065 ms contra 1,47 ms).
- Os dados de `dados/` também foram refeitos do zero e ficaram idênticos (item 5).

**PROBLEMA (médio): repetir o comando dá o mesmo número, mas mudar a ordem das linhas do catálogo muda D.** Refiz o teste embaralhando as linhas de `pecas.csv` e `pecas_v2.csv`, com as mesmas escondidas e a mesma configuração:

| Versão, acertos @1, @5, @10 | Original | Ordem 1 | Ordem 2 | Ordem 3 |
| --- | --- | --- | --- | --- |
| D | 0, 5, 10 | 3, 6, 12 | 0, 2, 6 | 2, 3, 6 |
| D, peças alcançadas | 111 | 129 | 134 | 116 |
| C | 21, 35, 37 | 21, 34, 37 | 16, 35, 36 | 20, 33, 37 |
| B | 22, 34, 35 | 22, 33, 35 | 17, 33, 34 | 22, 32, 35 |
| Popularidade | 10, 33, 52 | 9, 34, 52 | 8, 31, 49 | 7, 34, 52 |
| A | 3, 11, 30 | 3, 11, 30 | 3, 11, 30 | 3, 11, 30 |

Causa: os desempates dependem do índice da peça (`src/kdtree.cpp:47-48`, `src/recomendador.cpp:10`).

Consequências:

- A conclusão "D é pior" continua de pé. O melhor caso de D (12) ainda fica bem abaixo de B (34), C (36) e popularidade (49).
- As diferenças pequenas entre variações de D não podem ser interpretadas. "D sem bônus" (8), D (10) e "pesos iniciais" (11) estão dentro dessa variação.
- O intervalo de 95% não mede esse efeito.
- A escolha da configuração também é frágil. Ela saiu de 5 acertos em 2.332, e 4 combinações empatam em 5. Não refiz a validação nas ordens embaralhadas.

Correção sugerida: acrescentar a tabela acima a `avaliacao_d.md`, com a frase de que variações de poucos acertos não têm significado. Se houver tempo, repetir com 5 a 10 ordens e dar a faixa no artigo.

## 4. Mesmo teste: OK

- `rodar` recebe a mesma lista `testes` em todas as linhas de `hit_d.csv`, incluindo popularidade e as variações (`src/avaliacao_d.cpp:308`, `:327-372`). `avaliar_hit` usa a mesma função e a mesma semente (`src/avaliacao.cpp:110`).
- As 5.842 escondidas geradas de novo são iguais às de `escondidas.csv` (0 diferenças).
- O look restante é o look menos a escondida (`src/avaliacao_d.cpp:63-67`). A aleatória usa o mesmo (`:111`).
- Em todos os testes e em todas as candidatas de popularidade, A, B, C e D: 0 peças do look na lista, 0 de outra categoria, 0 repetidas.
- A escondida nunca fica repetida no resto do look (0 casos).

Observação: em 36 testes, o resto do look só tem peças da categoria pedida. D não tem aresta dentro da categoria, então nunca responde nesses casos. É parte da definição de D, e o efeito é pequeno.

## 5. Compilar e rodar do zero: OK no código, PROBLEMA na documentação

- `make clean && make && make test` na cópia e num `git clone` limpo: 0 warnings com `-Wall -Wextra`, 52 OK e 0 FALHOU.
- Ambiente: Ubuntu 22.04 com g++ 11.4. O README cita 24.04. Não testei em 24.04.
- Sem dados, `./recomendador avaliar` termina com mensagem clara.
- `preprocess/preprocessar.py` e `preprocess/atributos.py` refeitos a partir de `Combined_Metadata.json`: todos os CSVs de `dados/` e `amostra_atributos.md` ficaram idênticos byte a byte.

Problemas:

- **Médio.** `dados/README.md` não cita `pecas_v2` nem `atributos.py` (linhas 7-19). Quem seguir o README não consegue rodar `avaliar_d`. Correção: acrescentar o passo `python3 preprocess/atributos.py --entrada Combined_Metadata.json --saida dados` e os arquivos `pecas_v2*.csv`, com o formato.
- **Médio.** `README.md:21-27` não lista `avaliar_d`, `completar_d` e `montar`.
- **Pequeno.** `preprocess/requirements.txt` só tem `numpy`, mas `preprocess/graficos.py:8` precisa de `matplotlib`.
- **Pequeno.** Com `--dados dados/exemplo`, `avaliar` e `avaliar_d` gravam por cima de `resultados/`, porque a pasta de saída é fixa (`src/main.cpp:18`). Isso apagaria as tabelas reais. Correção: avisar no README ou pular a gravação nesse caso.

## 6. Honestidade: OK nas conclusões, problemas pequenos

OK:

- **D foi pior.** `avaliacao_d.md:48` diz "A versão D piorou", no nível da escolha aleatória. Confirmado: 10 acertos contra 10,6 esperados.
- **27%.** `diagnostico.md:7` e `docs/ARQUITETURA.md:85` tratam o número como teto. Refiz: 1.600 de 5.842. O acerto real de C é 0,63%. O número segue como "27% de acerto" só no texto do pedido, e não deve ir para o artigo.
- **Intervalos pareados.** `src/avaliacao_d.cpp:154-175` usa a diferença look a look. Refiz os 9 intervalos a partir de "só x acerta" e "só y acerta", e todos bateram. Todos continuam fora do 0 com correção para 9 comparações. O caso mais apertado é D x popularidade em Hit@1 (0 contra 10).
- **"D sem regras".** `avaliacao_d.md:34` explica a igualdade com b = c = 0.
- **Teto de D.** A cobertura de 1,90% aparece na tabela e no texto.
- **`acertos.png` e `tempos.png`.** Abri as duas. Os valores batem com os CSVs, e os eixos e legendas estão certos.
- **Rodada única.** `hit_d.csv` tem um só commit, e os horários dos arquivos vêm depois do último código. Não dá para provar por arquivo que não houve outra rodada, mas nada indica isso.

Problemas:

- **Médio.** `tempos.png` compara "Recomendacao C" com "Recomendacao D" em tarefas diferentes. C usa uma peça só como look, na própria categoria da peça (`src/avaliacao.cpp:69`). D usa looks reais de teste (`src/avaliacao_d.cpp:275`). Correção: tirar a comparação direta ou medir C nos mesmos looks.
- **Pequeno.** Na linha de 1.000 peças, "uma recomendação" é a média de só 4 looks. O texto diz "até 100" (`avaliacao_d.md:62`). Com 10 mil são 327 looks disponíveis, cortados em 100. Correção: informar o número.
- **Pequeno.** `avaliacao_d.md:15` diz que as regras "não ajudaram", mas não diz que quase não foram testadas. A estampa é "desconhecido" em 85,7% das peças e "lisa" em só 22 das 102.370. Então a regra quase nunca dá ponto. Correção: acrescentar essa frase.
- **Pequeno.** `avaliacao_d.md:8` diz "2.332 looks, ou seja, 10%". Foram retirados 2.506 looks (10%). Destes, 2.332 têm 2 peças ou mais e viraram consulta (9,3%).
- **Pequeno.** `avaliacao_d.md:51` diz "todos os atributos", mas a busca misturada não usa comprimento (`src/versao_d.cpp:76-77`). A mesma linha compara 25 com 10 sem intervalo. Calculei: 24 só a misturada, 9 só D, diferença de +0,257%, intervalo de +0,064% a +0,449%. A misturada não difere de forma clara de C (24 contra 36, intervalo de −0,465% a +0,054%) nem de A (23 contra 28, intervalo de −0,325% a +0,154%).
- **Pequeno.** "D com pesos iniciais" mantém k = 20 e "basta um lado". O nome da linha não diz isso.
- **Pequeno.** `acertos.png` mostra "D sem regras" sem a explicação da igualdade. Os dois gráficos estão sem acentos ("Pecas", "Recomendacao", "mutuo"). Correção: acentuar em `preprocess/graficos.py` e pôr a explicação na legenda do artigo.
- **Pequeno.** `docs/ARQUITETURA.md:459` ainda mostra popularidade com Hit@10 de 0,0087. O valor atual é 0,0089. `diagnostico.md` traz 51 acertos, de antes da correção, e `avaliacao_d.md:66` explica isso. Basta atualizar a arquitetura.

## Extra: comandos da versão D

**PROBLEMA (médio).** `completar_d` e `montar` usam `ConfigD{}`, ou seja, k = 5, mútuo e pesos 1 (`src/main.cpp:116`). Não usam a configuração escolhida em `config_d.csv`. Com k = 5 e mútuo, 90,9% das peças ficam sem aresta (conferi: 242.060 arestas), então esses comandos muitas vezes não devolvem nada. Eles também não são a versão da tabela. Correção: ler `config_d.csv`, ou usar k = 20, basta um lado, a = 0,5, b = 0, c = 0, d = 1. Não afeta as tabelas.

## Decisão para vocês

D só avalia peças ligadas por aresta a uma peça do look (`src/versao_d.cpp:165-170`, decisão registrada em `estruturas_d.md:37`). O pedido diz "para cada candidata da categoria pedida", o que também pode ser lido como a categoria inteira. Essa regra limita a cobertura a 1,90%. Ela também deixa de fora as peças que dividem look com o resto, que são justamente as que B acerta.

Medi a outra leitura **só na validação**, sem tocar no teste, somando a mesma nota em toda a categoria (b = c = 0):

| Validação (2.332 looks) | @1 | @5 | @10 |
| --- | --- | --- | --- |
| D escolhida (só arestas) | 2 | 5 | 5 |
| Categoria inteira, a = 0,5, d = 1 | 12 | 19 | 22 |
| Categoria inteira, a = 1, d = 5 | 14 | 20 | 23 |
| C | 14 | 19 | 19 |
| B | 14 | 17 | 17 |
| Popularidade | 2 | 7 | 16 |

Isso é só um indício. A diferença para C é de poucos looks, sem intervalo, e essa variante não passou por grade nem pelo teste. Mas ela mostra duas coisas:

- A frase "D piorou" vale para D como foi implementada. Ela não prova que a soma de notas é ruim.
- A explicação de `avaliacao_d.md:48-50` está incompleta. Os empates pesam, mas a regra de candidatas parece pesar mais.

Se quiserem testar, isso vira uma nova versão: ajustar só na validação, rodar o teste uma vez e declarar no artigo que o teste já tinha sido visto antes. Não substituam a linha D atual.
