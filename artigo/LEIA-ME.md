# Pasta do artigo

Modelo IEEE de conferência (`\documentclass[conference]{IEEEtran}`) para o Overleaf.

## Como abrir no Overleaf

1. Compacte a pasta: `cd artigo && zip -r ../artigo.zip .`
2. No Overleaf: New Project → Upload Project → escolha o zip.
3. Compilador: pdfLaTeX. Arquivo principal: `main.tex`.

Não foi possível compilar nesta máquina (não há LaTeX instalado). O arquivo usa só pacotes que o Overleaf já tem: IEEEtran, babel, booktabs, graphicx e tikz. Se der erro, me mande a mensagem.

## O que está pronto

- `figuras/fluxograma.tex`: fluxograma da metodologia (TikZ).
- `figuras/kdtree.png`, `union_find.png`, `agm.png`, `bfs.png`: gráficos de tempo, copiados de `resultados/tempos_f/`.
- `figuras/grupos_articulacoes.png`: desenho provisório do grafo entre grupos, feito por `preprocess/desenho_grupos.py`. **Troque pela imagem do Gephi.**
- `tabelas/*.tex`: geradas por `python3 preprocess/tabelas_artigo.py` a partir de `resultados/`. Para atualizar, rode o script de novo. Não edite os números à mão.

## O que falta

- **Imagem do Gephi.** Abra `resultados/gephi_articulacoes/q2_nodes.csv` e `q2_edges.csv` (Data Laboratory → Import Spreadsheet), use o layout ForceAtlas 2, cor pela coluna `destaque` e tamanho pela coluna `Pecas`. Exporte em PNG para `figuras/gephi.png` e troque o nome no `main.tex`.
- **Seções.** Seguem o enunciado do trabalho final: Resumo (até 250 palavras), Introdução, Referencial Teórico (Estado da Arte), Metodologia (com o fluxograma, que é obrigatório), Resultados e Discussão, Conclusão e Próximas Etapas. A antiga "Modelagem Experimental" virou a subseção "Configuração experimental" da Metodologia.
- **Autores.** Alunos por contribuição decrescente (se igual, ordem alfabética). O último autor é sempre o professor Michel Pires da Silva.
- **Referências.** `referencias.bib` tem 19 obras conferidas pelo DOI (Crossref e DataCite, 12/11/2026), com o BibTeX vindo do próprio doi.org. Mesmo assim, confira cada uma no scholar.google.com: referência inventada tira 50% da nota, e site ou blog tira 20%. Só aparece na lista o que for citado com `\cite` no texto. As entradas `repositorio` e `zenodo` são exigidas pelo enunciado; o DOI do Zenodo entra depois de publicar.
- **Ubuntu 24.04.** O enunciado exige que o código rode no Ubuntu 24.04 sem configuração especial. Esta máquina é 22.04; testar antes da entrega.
- Texto de todas as seções.
