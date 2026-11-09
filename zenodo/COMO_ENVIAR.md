# Como enviar ao Zenodo

Nada foi enviado. O envio é feito com a sua conta.

## Antes de enviar

1. Envie o branch e as tags ao GitHub, para o link do código funcionar: `git push origin versao-f --tags` (ou junte `versao-f` no `main` antes).
2. Gere a pasta de novo se algum dado mudou:
   ```
   ./recomendador exportar_grupos
   python3 preprocess/zenodo.py --entrada Combined_Metadata.json
   ```
3. Compacte: `cd zenodo && zip -r ../fashionverse_atributos_grupos.zip README.md LICENCA_ORIGEM.md *.csv tabelas_pontos`

## No site

1. Entre em https://zenodo.org com a sua conta e clique em "New upload".
2. Envie o zip (cerca de 27 MB descompactado).
3. Preencha os campos:

| Campo | O que pôr |
| --- | --- |
| Resource type | Dataset |
| Title | Atributos, grupos e divisão de looks para a FashionVerse-MME |
| Creators | o seu nome e o da dupla, com a instituição (CEFET-MG) |
| Description | o primeiro parágrafo do `README.md` + a lista de arquivos |
| License | Creative Commons Attribution 4.0 International (CC BY 4.0) |
| Keywords | recomendação de roupas, KD-tree, grafos, Polyvore, FashionVerse-MME |
| Related works | "Is derived from" → DOI 10.17632/6sx3pbsfvz.1 (FashionVerse-MME). "Is supplemented by" → link do repositório no GitHub |
| Version | v-final-2 |
| Language | Portuguese |

4. Clique em "Preview", confira e só depois em "Publish". Depois de publicado, os arquivos não podem ser trocados (só uma versão nova).
5. Copie o DOI do Zenodo para o README do repositório e para o artigo.
