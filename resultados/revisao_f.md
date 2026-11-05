# Revisão da versão F

Revisão feita em 05/11 no branch `f-revisao`, criado de `versao-f` em `d1e0547` (tag `v-final`). Nenhum arquivo de código ou de resultado foi mudado. Os números abaixo saem dos arquivos versionados ou das execuções desta revisão.

## Resumo

| Item | Resultado |
| --- | --- |
| 1. Nenhum dado de teste nas tabelas, grupos, popularidade e ajustes | OK |
| 2. Os números se repetem | OK. Os 4 comandos geraram arquivos iguais byte a byte. |
| 3. Mesmo teste e mesma avaliação para todas as versões | OK |
| 4. Compila e roda num clone limpo | OK. README desatualizado: PROBLEMA. |
| 5. A F diante da popularidade | OK no teste. A validação diz "ganha" no Hit@10 sem aviso: PROBLEMA pequeno. Textos antigos em `docs/ARQUITETURA.md` contradizem a tabela final: PROBLEMA. |
| 6. Números dos .md contra os CSV | 24 números conferidos, todos batem. Uma frase arredonda demais. |

## 1. Vazamento de dados de teste

**OK.** Nenhuma conta que escolhe ou pontua usa um look de teste. A regra comum é a flag `treino` de cada look. Toda contagem pula os looks com `treino == false`:

| Onde | O que conta | Evidência |
| --- | --- | --- |
| Grafo peça–look (B, C, D, popularidade) | só looks de treino | `src/grafo_looks.cpp:10` (`if (!base.looks[l].treino) continue;`) |
| Popularidade da avaliação justa | `grafo.num_looks`, do grafo acima | `src/avaliacao_justa.cpp:360` |
| Popularidade da F (termo e desempate) | `log(1 + num_looks)`, do mesmo grafo | `src/versao_f.cpp:96` |
| Tabelas de pontos por atributo | só looks de treino | `src/pontos_atributos.cpp:227` |
| Pontos dos grupos | só looks de treino | `src/grupos_kruskal.cpp:251` |
| Formação dos grupos (Kruskal e forma 3) | só vetores das peças, sem looks | `montar_grafo_knn` e `agrupar` em `src/versao_f.cpp:205`, `src/grupos_formas.cpp:364-371` |
| `grupo_cor` (k-means) | só a cor das peças | `preprocess/grupos_cor.py:3`, `resultados/grupos_cor.md:3` |

**Validação.** `sortear_validacao` (`src/avaliacao_d.cpp:175-184`) sorteia 20% dos looks que já são de treino, com semente 7. Então os looks de teste nunca entram na validação. `base_sem_validacao` (`src/avaliacao_justa.cpp:158-162`) marca esses looks como `treino = false`. Com isso eles também saem do grafo, das tabelas e dos grupos.

**`ajustar_f`.** Usa só `b = base_sem_validacao(base)` e `consultas_validacao(base)` (`src/versao_f.cpp:323-332`). O grafo, a C, as tabelas (`PontosAtributos pa(b, val)`), os 9 tipos de grupo (`PontosGrupos pg(b, o.gr)`, linha 352) e a popularidade são montados com `b`. A função nunca chama `consultas_teste`. O comando no `main` só repassa a base (`src/main.cpp:166`).

**`avaliar_justo --validacao`.** Usa `b = base_sem_validacao(base)` para tudo e não monta o teste (`src/avaliacao_justa.cpp:334-335`). A F é montada com o mesmo `b` (`VersoesF::montar`, `src/versao_f.cpp:192-209`).

**`vetor_d` e `estampa_d`.** Escolhem na validação e testam depois. O `vetor_d` escolhe entre as 8 combinações com `sem_val` e grava `vetor_d.csv` antes de montar o teste (`src/avaliacao_d.cpp:466-503`). O teste vem depois (linha 506 em diante). O `estampa_d` só roda o teste se a regra ganhar na validação (`src/avaliacao_d.cpp:588-591`). Ela perdeu (`validacao_estampa_d.csv`: 7 contra 5 acertos), e o teste não rodou.

**Vazamentos sutis procurados:**

- *Popularidade que conta o look da própria consulta.* Não acontece. No teste a consulta é um look com `treino = false`. Na validação a consulta é um look de validação, que fica `treino = false` em `b`. Há um teste para isso: `F: popularidade = log(1 + grau), sem o look de teste` (saída do `make test`).
- *Perguntas do FITB.* As erradas são sorteadas com a flag "aparece no treino". Na validação essa flag sai da base sem validação (`src/avaliacao_justa.cpp:165-166`). No teste ela sai da base com a validação dentro, como no `avaliar_fitb` (`src/avaliacao_fitb.cpp:248`). Não usa o teste.
- *Tabelas e grupos no teste final com os looks de validação dentro.* Acontece e é esperado. `avaliar_justo` sem `--validacao` usa a base inteira (`src/avaliacao_justa.cpp:334`). Está documentado em `docs/DECISOES.md:84` e em `resultados/tabelas_pontos.md:3`.
- *Ordem do trabalho.* No git, `config_f.csv` e `escolha_final.md` entram em `87407e8`, e `tabela_final.csv` só entra em `d1e0547`, depois. Não há resultado de teste da F versionado antes da escolha.

## 2. Repetição dos números

**OK.** Rodei cada comando uma vez, um de cada vez, no worktree. Depois de cada um, `git status --short` ficou vazio: os arquivos foram regravados (horário novo) com o mesmo conteúdo, byte a byte. Não foi preciso desfazer nada.

| Comando | Arquivos regravados | Tempo | Pico de memória | Resultado |
| --- | --- | --- | --- | --- |
| `./recomendador avaliar_justo --validacao` | `validacao_final.csv`, `.md`, `_diferencas.csv` | 2 min 30 s | 1,21 GB | iguais |
| `./recomendador avaliar_justo` | `tabela_final.csv`, `.md`, `_diferencas.csv` | 3 min 10 s | 1,21 GB | iguais |
| `./recomendador pontos` | `tabelas_pontos.md` e os 6 CSV de `tabelas_pontos/` | 1 s | 115 MB | iguais |
| `./recomendador grupos_formas` | `grupos_formas.md`, `gephi_grupos/escolhida_*` | 2 s | 157 MB | iguais |

Saídas de conferência:

- Teste: `Escondidas: as 5842 escondidas batem com resultados/escondidas.csv`. F: `Hit@10 = 0.0105615, FITB = 0.320353`, igual a `tabela_final.csv`.
- Validação: F: `Hit@10 = 0.0121784, FITB = 0.30759`, igual a `validacao_final.csv` e à linha da F em `grade_f.csv`.

## 3. Mesmo teste e mesma avaliação

**OK.**

- Todas as versões da tabela final rodam numa só chamada, sobre o mesmo objeto `Consultas c` (`src/avaliacao_justa.cpp:335` e `:352`). Então as escondidas, as 10 ordens de desempate (sementes 1 a 10, linhas 56-62) e as 3 erradas do FITB (semente 13 no teste e 11 na validação, linhas 25-26) são as mesmas para todas.
- As 5.842 escondidas batem com `resultados/escondidas.csv`, gravado pelo `avaliar_d` em 31/10 (saída acima). `sortear_escondidas` usa a semente 42 (`src/avaliacao.cpp:16` e `:80`) em todos os comandos.
- O bootstrap usa a semente 2026 em toda comparação (`src/avaliacao_justa.cpp:287`), então todas usam as mesmas 1.000 amostras.
- As versões antigas repetem a avaliação anterior. Sem as linhas da F e da C + popularidade, `tabela_final.csv` é igual a `tabela_corrigida.csv`, e `validacao_final.csv` é igual a `validacao_corrigida.csv` (`diff` sem saída nos dois casos).
- As perguntas do FITB são as do `avaliar_fitb` (mesma função `montar_perguntas` e mesmas sementes, `src/avaliacao_fitb.cpp:21-22`). A popularidade dá 31,409% no `fitb.csv` e 31,40% (média das 10 ordens) na tabela final.

## 4. Compilação num clone limpo

**OK.** Clone local do branch em `/tmp/claude-1000/.../scratchpad/clone`, sem `dados/*.csv`:

```
make clean && make && make test
```

- Ubuntu 22.04, `g++ (Ubuntu 11.4.0-1ubuntu1~22.04.3) 11.4.0`, flags `-std=c++17 -O2 -Wall -Wextra`.
- 0 avisos nos 21 arquivos. Saída final do teste: `Resultado: 155 OK e 0 FALHOU`.
- `./recomendador avaliar_justo --validacao --dados dados/exemplo` roda no clone e avisa que falta `config_f.csv` para a base de exemplo.

**PROBLEMA: o README não documenta os comandos novos.** A lista de comandos (`README.md:21-30`) e a sequência para refazer tudo (`README.md:36-45`) param na versão D. Faltam:

- `avaliar_justo` e `avaliar_justo --validacao`
- `ajustar_f`
- `pontos`
- `grupos`
- `grupos_formas`
- `tempos_f` (e `make tempos_f`)
- `vetor_d`
- `estampa_d`
- também `avaliar_e`, `avaliar_fitb`, `completar_e` e `validacao`, que o `main` aceita (`src/main.cpp:123-128`)
- na sequência: `python3 preprocess/grupos_cor.py` (gera `pecas_v5.csv`), o preparo de `pecas_v4.csv` e a ordem `vetor_d`, `ajustar_f`, `avaliar_justo --validacao`, `avaliar_justo`

O README também diz "no Ubuntu 24.04" (`README.md:13`). A compilação aqui foi no 22.04 e funcionou.

## 5. A F ganha da popularidade?

**No teste (`tabela_final.md`), em frases simples:**

- **Ganha de verdade:** no Hit@1, a F acerta 0,313% contra 0,142% da popularidade (+0,171 pp, intervalo de 0,024 a 0,330 pp). No FITB, 32,04% contra 31,40% (+0,63 pp, intervalo de 0,14 a 1,11 pp). Contra a C, ganha de verdade no Hit@10 (+0,305 pp) e no FITB (+6,67 pp).
- **Pode ser sorte:** no Hit@5 (+0,147 pp, intervalo de −0,058 a 0,359 pp) e no Hit@10 (+0,161 pp, intervalo de −0,068 a 0,383 pp). O intervalo passa pelo zero. Não dá para dizer que a F é melhor que a popularidade no Hit@10.
- **Não ganha da C + popularidade em nada.** No Hit@10 a F fica até um pouco abaixo (1,056% contra 1,058%), e no Hit@5 também (0,716% contra 0,746%). Todas as diferenças podem ser sorte. Ou seja, os grupos não trouxeram ganho medível no teste. A vantagem da F na validação era de 1 consulta em 2.332, e só 2 das 623 combinações da grade chegaram a 1,218% de Hit@10 (`grade_f.csv`). Esse ganho sumiu no teste.
- **Perde (sem intervalo calculado):** no Hit@1 a F (0,313%) fica abaixo da B (0,373%) e da C (0,336%). No FITB fica abaixo da A (34,29%). A tabela não traz o intervalo da F contra a A ou contra a B, então não dá para dizer se essas diferenças são reais.

**Textos do repositório.** Nenhum .md em `resultados/` ou `docs/` diz que a F é melhor que a popularidade no Hit@10 do teste. `tabela_final.md` diz "pode ser sorte" no Hit@5 e no Hit@10. `docs/DECISOES.md:90` diz que a diferença para a C + popularidade "pode ser sorte".

**PROBLEMA pequeno:** `validacao_final.md` e `escolha_final.md` dizem "F ganha de Popularidade (diferença real)" no Hit@10 (+0,532 pp, intervalo de 0,142 a 1,003 pp). Isso vale só na validação, onde a F foi escolhida. Esse número é otimista e não aparece no teste. Nenhum dos dois arquivos avisa isso. Se o artigo citar a validação, precisa dizer.

**PROBLEMA: textos antigos contradizem a avaliação justa.** Eles vêm da avaliação antiga, que não dava nota à categoria inteira:

- `docs/ARQUITETURA.md:431`: "Ela [a D] ficou pior que B, C e popularidade." Na tabela final a D ganha da popularidade no Hit@1 (diferença real) e as diferenças no Hit@5 e no Hit@10 podem ser sorte. Contra a C, só o FITB separa, e a favor da D.
- `docs/ARQUITETURA.md:478`: "A versão C só ganha da B em Hit@5 e Hit@10". Na tabela final a B fica acima da C nas 3 métricas de Hit (B x C: Hit@1 +0,038 pp, diferença real; Hit@5 +0,014 pp e Hit@10 +0,021 pp, pode ser sorte). A tabela de `ARQUITETURA.md:470-475` usa os números antigos de `hit_k.csv`.
- `docs/ARQUITETURA.md:433`: "Uma versão que não supera a popularidade não ajuda." Pela regra do próprio documento, a F não supera a popularidade no Hit@5 e no Hit@10 do teste. O documento não fala da F nem da avaliação justa.
- `resultados/avaliacao_d.md:46` e `:65` e `resultados/revisao.md:7` dizem "D é pior que B, C e popularidade". Valia na avaliação antiga. Hoje não vale no Hit@10.

## 6. Números dos .md contra os CSV

Conferi 24 números. Todos batem, a não ser pelo arredondamento indicado.

| Texto | Número no .md | Fonte | Confere |
| --- | --- | --- | --- |
| `tabela_final.md` | F Hit@10 1,056% | `tabela_final.csv`: 0.0105615 | OK |
| `tabela_final.md` | C + pop Hit@10 1,058% | 0.0105786 | OK |
| `tabela_final.md` | F x Pop Hit@10 0,161 pp (−0,068 a 0,383) | `tabela_final_diferencas.csv`: 0.00160904, −0.000684697, 0.0038343 | OK |
| `tabela_final.md` | F x Pop FITB 0,63 pp (0,14 a 1,11) | 0.00633345, 0.00142075, 0.0110921 | OK |
| `escolha_final.md` | F Hit@10 1,218%, Hit@5 0,858% | `grade_f.csv`: 0.0121784, 0.00857633 | OK |
| `escolha_final.md` | C + pop Hit@10 1,175% | `grade_f.csv`: 0.0117496 | OK |
| `escolha_final.md` | 623 combinações | `grade_f.csv`: 623 linhas | OK |
| `config_f.csv` | F = (1; 0; 0,5; 1), K = 100, estampa 0,1 | igual ao `escolha_final.md` | OK |
| `DECISOES.md:10-11` | vetor 7 contra 6; estampa 7 para 5 | `validacao_vetor_d.csv`, `validacao_estampa_d.csv` | OK |
| `DECISOES.md` e `grupos_formas.md` | maior grupo 17,6%, 1255 grupos, 9,1% de 1 peça | média da coluna forma 3: 17,635%; soma 1255; 114/1255 = 9,08% | OK |
| `tabelas_pontos.md:48` | 109 pares com tabela própria | `tabelas_pontos/cor.csv`: 109 pares de categoria | OK |
| `tabelas_pontos.md` | bag rosa x shoes rosa: n 47, esperado 3,2, pontos 2,43 | `cor.csv`: 47, 3.23555, 2.42769 | OK |
| `tabelas_pontos.md` | outerwear bege x skirt cinza 1: −2,40 | `cor.csv`: −2.4022 | OK |
| `tabelas_pontos.md` | bag x shoes animal print: 22, 5,6, 1,25 | `estampa.csv`: 22, 5.57255, 1.25259 | OK |
| `grupos_cor.md` | preto 26384, branco 24096, bege 11400, total 102370 | contagem em `dados/pecas_v5.csv` | OK |
| `grupos_kruskal.md` | K = 20: 400 grupos, maior 16897 (bag, 99%), 19 de 20 | `grupos_kruskal.csv` | OK |
| `grupos_kruskal.md` | K = 100: 2000 grupos, maior 14952 (bag, 88%), 16 de 20 | `grupos_kruskal.csv` | OK |
| `tempos_f.md` | KD-tree 19 vezes mais rápida em 100 mil | 1,99076 / 0,106758 = 18,6 | OK |
| `tempos_f.md` | kNN cresce 13 vezes | 0,106758 / 0,0082956 = 12,9 | OK |
| `tempos_f.md` | Union-Find: 35 mil vezes | 35239,4 / 0,987 = 35.700 | OK |
| `tempos_f.md` | Kruskal 1,3 a 1,5 vezes mais rápido que Prim | 1,30; 1,48; 1,52 | OK |
| `tempos_f.md` | pesos 45,56; 252,11; 1.381,19 e 6,5 arestas por peça | `agm_conferencia.csv`; 649640/100000 | OK |
| `tempos_f.md` | desvio da BFS até 30%, Union-Find 10% | 30,3% e 9,97% | OK |
| `tempos_f.md` | "Ordenar as arestas custa de 7 a 8 vezes mais que o laço" | 6,7; 8,3; 7,8 | arredonda demais. Seria "de 7 a 8 vezes, em média" ou "de 6,7 a 8,3 vezes". |

**Frases que exageram ou confundem:**

- Na tabela de diferenças, uma diferença de exatamente zero aparece como "pode ser sorte" (`tabela_final.md`, F x C + popularidade, Hit@1: "0,000 pp | 0,000 a 0,000 pp"). As duas listas acertam as mesmas consultas. O certo seria "sem diferença".
- `docs/DECISOES.md`, linha do peso da estampa (seção "Grupos com Kruskal"), ainda diz "Ainda não. Ela pediu um teste com 3 valores na validação." O teste foi feito no `ajustar_f`. Em K = 100, os pesos 0,1 e 0,2 empataram em tudo (`grade_f.csv`: Hit@10 0,0121784 e Hit@5 0,00857633 nos dois). O 0,1 ficou porque vem antes na ordem da grade (`src/versao_f.cpp:345`, comparação estrita na linha 436). Isso não está escrito.
- `validacao_final.md:3` diz que os looks de validação "saem do grafo, do bônus e da popularidade". Eles também saem das tabelas de pontos e dos grupos da F. Está certo no código, só falta no texto.
- `grupos_kruskal.md` usa ponto como separador decimal ("0.10", "1.38 s", "-0.001"). Os outros textos usam vírgula.
- `DECISOES.md:10` diz que o vetor só com ocasião "ganhou das 8 combinações". Ele é uma das 8.

## Problemas por gravidade

| Gravidade | Problema | Onde | O que fazer |
| --- | --- | --- | --- |
| Alta | O README não traz os comandos da E e da F nem a sequência nova para refazer os resultados. | `README.md:21-45` | Listar os comandos do item 4 e a ordem `grupos_cor.py`, `vetor_d`, `ajustar_f`, `avaliar_justo --validacao`, `avaliar_justo`, `pontos`, `grupos`, `grupos_formas`, `make tempos_f`. |
| Média | `docs/ARQUITETURA.md` traz conclusões da avaliação antiga que a tabela final contradiz (D pior que todas; C ganha da B no Hit@5 e no Hit@10) e não fala da avaliação justa nem da F. | `docs/ARQUITETURA.md:431`, `:433`, `:470-478` | Marcar como avaliação antiga ou trocar pelos números de `tabela_final.md`. |
| Média | A validação diz "F ganha de Popularidade (diferença real)" no Hit@10 sem avisar que a F foi escolhida nessas mesmas consultas. | `validacao_final.md`, `escolha_final.md` | Dizer no artigo que o número da validação é otimista e que, no teste, o Hit@10 pode ser sorte. |
| Baixa | Frases antigas "D é pior que B, C e popularidade". | `avaliacao_d.md:46`, `:65`; `revisao.md:7` | Avisar que valem para a avaliação antiga. |
| Baixa | Linha do peso da estampa desatualizada e empate 0,1 x 0,2 sem registro. | `docs/DECISOES.md`, seção "Grupos com Kruskal" | Atualizar a linha. |
| Baixa | Diferença zero aparece como "pode ser sorte". | `src/avaliacao_justa.cpp:99-103`, `src/versao_f.cpp:299-303` | Só no texto do artigo. Mudar o código mudaria os .md gerados. |
| Baixa | "De 7 a 8 vezes" arredonda 6,7 a 8,3. Ponto decimal em `grupos_kruskal.md`. README cita só o Ubuntu 24.04. | `tempos_f.md`, `grupos_kruskal.md`, `README.md:13` | Ajustar o texto. |

Nenhum problema muda um número de acerto. Não achei vazamento, e os resultados se repetem.

## Comandos para refazer esta revisão

```
for f in <repo>/dados/*.csv <repo>/dados/*.json; do ln -sf "$f" dados/; done
make
./recomendador avaliar_justo --validacao && git status --short
./recomendador avaliar_justo && git status --short
./recomendador pontos && git status --short
./recomendador grupos_formas && git status --short
diff <(grep -v "^F,\|^C + popularidade," resultados/tabela_final.csv) resultados/tabela_corrigida.csv
diff <(grep -v "^F,\|^C + popularidade," resultados/validacao_final.csv) resultados/validacao_corrigida.csv
git clone --branch f-revisao <worktree> /tmp/clone && cd /tmp/clone && make clean && make && make test
```
