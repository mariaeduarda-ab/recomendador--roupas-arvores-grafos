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
- **Seções.** Os títulos seguem o artigo modelo do professor (`Documentação_AEDS.pdf`: Introdução, Fundamentação Teórica, Metodologia, Modelagem Experimental, Resultados e Discussão, Conclusões). Não achei o enunciado do trabalho final com as seções pedidas. Confiram com ele.
- Nomes dos autores, texto de todas as seções e as referências.
