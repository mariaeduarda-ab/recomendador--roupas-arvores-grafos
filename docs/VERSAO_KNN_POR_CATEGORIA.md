# Versão D: kNN por categoria e notas

Este documento explica a versão D e o resultado dela. Todos os números vêm dos arquivos da pasta `resultados/`.

**Resumo.** A versão D ficou pior que as versões B e C e pior que a popularidade. No Hit@10, ela ficou no mesmo nível da escolha aleatória.

## 1. O problema da versão antiga

As versões antigas são A (só semelhança), B (só grafo) e C (grafo + semelhança). O teste tem 5.842 looks. Cada look tem uma peça escondida, e o sistema precisa achar essa peça entre as peças da categoria dela.

**O número 27% não é acerto.** Em 1.600 dos 5.842 looks (27,4%), a peça escondida aparece em algum look de treino. Esse é o teto das versões B e C. Elas não têm como passar disso. A frase "27% de acerto" não deve ir para o artigo. O acerto real da versão C no Hit@10 é 0,633%.

| Medida | Valor |
| --- | --- |
| Peças da base | 102.370 |
| Peças em só 1 look | 87.617 (85,6%) |
| Peças escondidas sem look de treino | 4.242 de 5.842 (72,6%) |
| Peças escondidas que a versão B alcança | 35 |
| Peças escondidas que a versão C alcança | 38 |
| Testes em que a versão B fica sem candidatas | 3.651 (62,5%) |
| Testes em que a versão C fica sem candidatas | 267 (4,6%) |

O problema principal é alcançar a peça, e não ordenar a lista. Das 35 peças que a versão B alcança, as 35 entram no top 10. Na versão C, entram 37 de 38.

A ideia da versão D foi usar mais atributos para criar mais ligações entre as peças.

## 2. A nova ideia passo a passo

Os valores abaixo são inventados, mas as contas saíram do código real (`src/versao_d.cpp`). O exemplo usa todos os pesos iguais a 1 para mostrar cada parte da nota. A configuração final da versão D é outra (seção 5).

### Passo 1. As peças

| Peça | Estilo | Ocasião | Formalidade | Cor neutra | Estampa |
| --- | --- | --- | --- | --- | --- |
| camisa | casual | dia a dia | 0,50 | não | listrada |
| saia 1 | casual | dia a dia | 0,50 | sim | lisa |
| saia 2 | casual | dia a dia | 0,25 | não | floral |
| saia 3 | clássico | trabalho | 0,75 | sim | desconhecida |
| sapato 1 | casual | dia a dia | 0,25 | sim | desconhecida |
| sapato 2 | casual | dia a dia | 0,50 | não | desconhecida |
| sapato 3 | clássico | trabalho | 0,75 | sim | desconhecida |

Os looks de treino são:

- L1: camisa, saia 1, sapato 1
- L2: saia 2, sapato 2
- L3: saia 3, sapato 3

### Passo 2. A fórmula da nota

```
nota(p, q) = a · proximidade + b · regra_cor + c · regra_estampa + d · bônus
```

- **proximidade** = 1 − raiz(soma das diferenças ao quadrado / 11). O vetor do grupo 1 tem 11 números.
- **regra_cor** = 0,5 se pelo menos uma das peças tem cor neutra. Se não, 0.
- **regra_estampa** = −1 se as duas têm estampa, +1 se uma é lisa e a outra tem estampa, 0 nos outros casos. Estampa desconhecida sempre dá 0.
- **bônus** = looks em que p e q aparecem juntas / raiz(looks de q). Se esse valor não existir, o código usa o bônus de grupo.
- **bônus de grupo** = vezes que os dois grupos aparecem juntos / raiz(aparições do grupo de p × aparições do grupo de q). Um grupo é categoria + primeiro estilo + tipo de estampa.

### Passo 3. A conta de uma dupla

Camisa e saia 2:

- As duas diferem só na formalidade, 0,50 e 0,25. A soma das diferenças ao quadrado é 0,0625.
- proximidade = 1 − raiz(0,0625 / 11) = 0,9246
- regra_cor = 0, porque nenhuma é neutra.
- regra_estampa = −1, porque as duas têm estampa.
- bônus = 0, porque elas não aparecem juntas e os grupos delas também não.
- nota = 0,9246 + 0 − 1 + 0 = −0,0754

### Passo 4. As notas de cada dupla

| Dupla | Proximidade | Cor | Estampa | Bônus | Nota |
| --- | --- | --- | --- | --- | --- |
| camisa e saia 1 | 1,0000 | 0,5 | +1 | 1,0000 (peça) | 3,5000 |
| camisa e saia 2 | 0,9246 | 0 | −1 | 0 | −0,0754 |
| camisa e saia 3 | 0,3923 | 0,5 | 0 | 0 | 0,8923 |
| camisa e sapato 1 | 0,9246 | 0,5 | 0 | 1,0000 (peça) | 2,4246 |
| camisa e sapato 2 | 1,0000 | 0 | 0 | 0,7071 (grupo) | 1,7071 |
| camisa e sapato 3 | 0,3923 | 0,5 | 0 | 0 | 0,8923 |
| saia 1 e sapato 1 | 0,9246 | 0,5 | 0 | 1,0000 (peça) | 2,4246 |
| saia 1 e sapato 2 | 1,0000 | 0,5 | 0 | 0,7071 (grupo) | 2,2071 |
| saia 1 e sapato 3 | 0,3923 | 0,5 | 0 | 0 | 0,8923 |
| saia 2 e sapato 1 | 1,0000 | 0,5 | 0 | 0,7071 (grupo) | 2,2071 |
| saia 2 e sapato 2 | 0,9246 | 0 | 0 | 1,0000 (peça) | 1,9246 |
| saia 2 e sapato 3 | 0,3784 | 0,5 | 0 | 0 | 0,8784 |
| saia 3 e sapato 1 | 0,3784 | 0,5 | 0 | 0 | 0,8784 |
| saia 3 e sapato 2 | 0,3923 | 0,5 | 0 | 0 | 0,8923 |
| saia 3 e sapato 3 | 1,0000 | 0,5 | 0 | 1,0000 (peça) | 2,5000 |

O bônus 0,7071 de "camisa e sapato 2" vem do grupo. Sapato 1 e sapato 2 estão no mesmo grupo. Esse grupo aparece 2 vezes nos looks, e o grupo da camisa aparece 1 vez. Eles aparecem juntos 1 vez. Então 1 / raiz(1 × 2) = 0,7071.

### Passo 5. Completar um look

O look tem só a camisa e falta a saia. Com uma peça no look, a soma é a própria nota da dupla.

| Candidata | Soma |
| --- | --- |
| saia 1 | 3,5000 |
| saia 3 | 0,8923 |
| saia 2 | −0,0754 |

A resposta é a saia 1. Com mais peças no look, somamos a nota da candidata com cada peça.

### Passo 6. Montar um look inteiro

O ponto de partida é a camisa. Temos 3 saias × 3 sapatos, ou seja, 9 combinações. A soma de cada look usa as 3 duplas: camisa e saia, camisa e sapato, saia e sapato.

| Look | Conta | Soma |
| --- | --- | --- |
| saia 1, sapato 1 | 3,5000 + 2,4246 + 2,4246 | **8,3492** |
| saia 1, sapato 2 | 3,5000 + 1,7071 + 2,2071 | **7,4142** |
| saia 1, sapato 3 | 3,5000 + 0,8923 + 0,8923 | **5,2846** |
| saia 2, sapato 1 | −0,0754 + 2,4246 + 2,2071 | 4,5563 |
| saia 3, sapato 3 | 0,8923 + 0,8923 + 2,5000 | 4,2846 |
| saia 3, sapato 1 | 0,8923 + 2,4246 + 0,8784 | 4,1953 |
| saia 2, sapato 2 | −0,0754 + 1,7071 + 1,9246 | 3,5563 |
| saia 3, sapato 2 | 0,8923 + 1,7071 + 0,8923 | 3,4917 |
| saia 2, sapato 3 | −0,0754 + 0,8923 + 0,8784 | 1,6953 |

Os 3 looks em negrito são os que o programa devolve.

## 3. Por que separamos os atributos em dois grupos

- **Grupo 1, devem ser parecidos:** estilo, ocasião e formalidade. Só eles entram na busca kNN. Uma peça casual procura outras peças casuais.
- **Grupo 2, devem combinar:** cor, estampa e comprimento. Eles ficam fora da busca. Cor e estampa só mudam a nota. Comprimento foi extraído, mas nenhuma regra da nota usa esse atributo.

Motivo: se a estampa entrasse na busca, uma peça listrada traria outras peças listradas. A regra de combinação diz o contrário.

O teste não confirmou essa separação. A variação com os atributos misturados acertou mais que a D: 25 contra 10 no Hit@10. O intervalo de 95% da diferença vai de +0,064% a +0,449%.

## 4. Como cada estrutura é usada

| Estrutura | Uso |
| --- | --- |
| KD-tree por categoria | Existe uma árvore para cada categoria. Para cada peça, cada árvore devolve as k peças mais próximas daquela categoria. |
| kNN mútuo | Liga X e Y só se um está entre as k do outro, nos dois sentidos. A versão também aceita a regra "basta um lado". |
| Grafo peça–look | Guarda quais peças aparecem em quais looks de treino. |
| BFS de 2 passos | Vai da peça para os looks dela e deles para as outras peças. Dá a contagem usada no bônus. |
| Heap | No comando `montar`, guarda os 3 melhores looks. O topo é o pior dos 3. |

A candidata de um teste é uma peça da categoria pedida ligada por aresta a pelo menos uma peça do look.

Comandos:

```
./recomendador completar_d <id_look> <categoria>
./recomendador montar <id_peca> [categorias]
./recomendador avaliar
./recomendador avaliar_d
python3 preprocess/graficos.py
```

## 5. Resultados

A configuração foi escolhida só com a validação: 2.332 looks e 96 combinações. A escolhida foi k = 20, basta um lado, a = 0,5, b = 0, c = 0, d = 1.

b = 0 e c = 0 significam que a validação descartou as regras de cor e estampa. A melhor combinação acertou só 5 looks, então essa escolha é muito sensível ao acaso. As regras também quase não foram testadas: a estampa é desconhecida em 85,7% das peças e só 22 peças estão marcadas como lisas.

O teste rodou uma única vez, com 5.842 looks.

| Versão | Acertos @10 | Hit@1 | Hit@5 | Hit@10 | Cobertura |
| --- | --- | --- | --- | --- | --- |
| Aleatória (esperado) | 10,6 | 0,018% | 0,091% | 0,182% | 100% |
| Aleatória (sorteio) | 11 | 0,034% | 0,103% | 0,188% | 100% |
| Popularidade | 52 | 0,171% | 0,565% | 0,890% | 100% |
| A | 30 | 0,051% | 0,188% | 0,514% | 100% |
| B | 35 | 0,377% | 0,582% | 0,599% | 0,60% |
| C | 37 | 0,359% | 0,599% | 0,633% | 0,65% |
| **D** | **10** | **0%** | **0,086%** | **0,171%** | **1,90%** |
| D sem regras de cor e estampa | 10 | 0% | 0,086% | 0,171% | 1,90% |
| D sem bônus de looks | 8 | 0% | 0,103% | 0,137% | 1,90% |
| D com pesos 1 (mesmo k e regra) | 11 | 0,017% | 0,068% | 0,188% | 1,90% |
| D com kNN mútuo | 5 | 0,034% | 0,051% | 0,086% | 0,15% |
| D com atributos misturados | 25 | 0,068% | 0,240% | 0,428% | 3,54% |

"D sem regras" é igual a D porque a configuração escolhida já tem b = c = 0.

**O que a tabela significa:**

- **Hit@10** é a porcentagem de testes em que a peça escondida ficou entre as 10 primeiras.
- **Cobertura** é a porcentagem de testes em que a peça escondida apareceu entre as candidatas, em qualquer posição.
- **D é pior que B, C e popularidade.** Os intervalos de 95% da diferença no Hit@10 não passam pelo 0: D x B vai de −0,653% a −0,203%, D x C vai de −0,692% a −0,232% e D x popularidade vai de −0,982% a −0,455%.
- **D empatou com a escolha aleatória.** Foram 10 acertos contra 10,6 esperados.
- **A cobertura subiu, mas o acerto não.** D alcança 111 peças escondidas, contra 38 da C. Só 10 dessas 111 entram no top 10.

**Tempos da versão D:**

| Peças | KD-trees por categoria | Arestas | Uma recomendação |
| --- | --- | --- | --- |
| 1.000 | 0,32 ms | 67 ms | 0,010 ms |
| 10.000 | 3,1 ms | 1.880 ms | 0,016 ms |
| 100.000 | 39,5 ms | 92.907 ms | 0,165 ms |

Com 1.000 peças, sobram em média só 4 looks de teste. Esse tempo de recomendação é menos confiável.

## 6. Limitações

- **Empates.** A busca tem só 637 vetores diferentes. Em 95,1% das buscas com k = 5, a 5ª e a 6ª vizinhas têm a mesma distância. O corte escolhe entre peças empatadas pela posição delas.
- **Ordem do catálogo.** Só por mudar a ordem das linhas, D variou de 6 a 13 acertos no Hit@10. Em todas as ordens, D ficou abaixo de B, C e popularidade. Diferenças pequenas entre variações de D não têm significado.
- **Poucas arestas no mútuo.** Com busca no grupo 1, mútuo e k = 5, 90,9% das peças ficam sem nenhuma aresta.
- **Cobertura.** A cobertura de D é 1,90%. Ela limita o acerto, porque só uma peça candidata pode ser recomendada.
- **Atributos com muito desconhecido.** Estampa tem 85,7%, comprimento tem 89,1% e modelagem tem 92,1%. A ocasião separa pouco: festa aparece em 76,9% das peças.
- **Atributos vêm de texto gerado por modelos.** A extração pode errar. As 50 peças de `resultados/amostra_atributos.md` ainda precisam de conferência manual.
- **A nota não foi testada na categoria inteira.** D avalia só peças ligadas por aresta. Na validação, a mesma nota na categoria inteira teve 22 a 23 acertos, contra 5 da D, 19 da C, 17 da B e 16 da popularidade. Isso é só um indício, com poucos looks de diferença e sem intervalo. Não foi medido no teste.
- **Variação misturada.** Ela usa 15 números, acima do limite de 12, e não se separa de forma clara de A nem de C. Foi comparada só no teste, então não pode virar a escolha sem outra validação.

## 7. Perguntas que o professor pode fazer

**1. Por que a nova versão piorou?**
As medições apontam três fatores. Os atributos da busca têm muitos empates, com só 637 vetores diferentes. A escolha de vizinhas depende da posição da peça, e isso muda o resultado de 6 a 13 acertos. A lista de candidatas fica presa às arestas, com cobertura de 1,90%. Não provamos o peso de cada fator.

**2. Por que manter a versão antiga?**
Porque ela teve resultado melhor e é a base de comparação. Sem ela, não teríamos como dizer que a D piorou.

**3. O que é vazamento de dados?**
É usar a resposta do teste durante a preparação. Se um look de teste entrasse no grafo, a BFS acharia a peça escondida dentro dele. A revisão embaralhou os looks de teste e a validação e a configuração saíram iguais, então eles não influenciaram o ajuste.

**4. Por que uma KD-tree por categoria?**
Para buscar direto as peças de uma categoria. Em uma árvore única, as peças das outras categorias não entram no heap, e a poda corta menos lados.

**5. O sistema acerta 27%?**
Não. 27,4% é o teto das versões B e C, ou seja, os testes em que a peça escondida existe no treino. O Hit@10 da C é 0,633%.

**6. Por que os acertos são tão baixos em todas as versões?**
O teste exige a peça exata entre cerca de 10.940 candidatas em média. Uma peça parecida conta como erro. Além disso, 85,6% das peças estão em só 1 look.

**7. Por que a validação desligou as regras de cor e estampa?**
Porque elas não ajudaram na validação. Isso não prova que as regras são ruins. A melhor combinação acertou só 5 looks, e a estampa é desconhecida em 85,7% das peças.

**8. Por que não usar kNN mútuo, como foi planejado?**
Com busca no grupo 1 e k = 5, 90,9% das peças ficam sem aresta. A validação escolheu "basta um lado" com k = 20. No teste, a variação mútua teve 5 acertos contra 10 da D.

**9. Como vocês garantem que a comparação é justa?**
Todas as versões usam a mesma divisão e as mesmas 5.842 peças escondidas, gravadas em `escondidas.csv`. Os pesos foram ajustados só na validação, e o teste rodou uma vez para gerar a tabela. Uma nova execução deu tabelas de acerto idênticas. O teste foi repetido depois só para medir a sensibilidade à ordem, e essa medida não escolheu nada.

**10. O que vocês fariam depois?**
Primeiro, testar uma versão nova com candidatas da categoria inteira, ajustada só na validação. Depois, buscar atributos com menos desconhecidos e menos empates. Hoje isso é só uma hipótese, porque o único indício vem da validação.
