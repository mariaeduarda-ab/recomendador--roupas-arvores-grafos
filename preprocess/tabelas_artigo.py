#!/usr/bin/env python3
# Gera as tabelas do artigo (LaTeX) a partir dos CSVs e md de resultados/, para nenhum numero ser copiado a mao.
# Uso: python3 preprocess/tabelas_artigo.py   (grava artigo/tabelas/*.tex)

import csv
import re
from pathlib import Path

RES = Path("resultados")
SAIDA = Path("artigo/tabelas")
# a F e a versao final. a F2 foi so experimento e fica fora das tabelas do artigo
VERSOES = ["Aleatória", "Popularidade", "A", "B", "C", "D", "D nova", "C + popularidade", "F"]
METRICAS = ["hit1", "hit5", "hit10", "fitb"]


def pct(x, casas=3):
    return f"{100 * x:.{casas}f}".replace(".", ",") + r"\%"


def pp(x, casas=3):
    s = f"{100 * x:.{casas}f}".replace(".", ",")
    return s.replace("-", "$-$")


def tabela(nome, legenda, rotulo, colunas, linhas, larga=False):
    amb = "table*" if larga else "table"
    corpo = "\n".join(" & ".join(l) + r" \\" for l in linhas)
    texto = (f"\\begin{{{amb}}}[t]\n\\centering\n\\caption{{{legenda}}}\n\\label{{{rotulo}}}\n\\footnotesize\n"
             f"\\begin{{tabular}}{{l{'r' * (len(colunas) - 1)}}}\n\\toprule\n{' & '.join(colunas)} \\\\\n\\midrule\n"
             f"{corpo}\n\\bottomrule\n\\end{{tabular}}\n\\end{{{amb}}}\n")
    (SAIDA / nome).write_text(texto, encoding="utf-8")
    print(SAIDA / nome)


def final():
    v = {}
    for r in csv.DictReader(open(RES / "tabela_final.csv", encoding="utf-8")):
        v[(r["versao"], r["metrica"])] = float(r["media"])
    linhas = [["Aleatória (esperado)"] + [pct(v[("Aleatória (esperado)", m)], 2 if m == "fitb" else 3) for m in METRICAS]]
    for nome in VERSOES[1:]:
        linhas.append([nome] + [pct(v[(nome, m)], 2 if m == "fitb" else 3) for m in METRICAS])
    tabela("final.tex", "Teste com 5.842 looks: média de 10 ordens de desempate (tabela\\_final.md).", "tab:final",
           ["Versão", "Hit@1", "Hit@5", "Hit@10", "FITB"], linhas)


def diferencas():
    quero = [("F", "Popularidade"), ("F", "C"), ("F", "C + popularidade")]
    d = {}
    for r in csv.DictReader(open(RES / "tabela_final_diferencas.csv", encoding="utf-8")):
        d.setdefault((r["x"], r["y"], r["metrica"]), r)
    linhas = []
    for x, y in quero:
        for m, rot in (("hit10", "Hit@10"), ("fitb", "FITB")):
            r = d[(x, y, m)]
            c = 2 if m == "fitb" else 3
            real = float(r["ic95_min"]) > 0 or float(r["ic95_max"]) < 0
            linhas.append([f"{x} x {y}", rot, pp(float(r["diferenca"]), c),
                           f"{pp(float(r['ic95_min']), c)} a {pp(float(r['ic95_max']), c)}", "real" if real else "pode ser sorte"])
    tabela("diferencas.tex", "Diferenças no teste, em pontos percentuais, com intervalo de 95\\% (bootstrap, 1.000 sorteios).",
           "tab:diferencas", ["Comparação", "Métrica", "Diferença", "Intervalo", "Conclusão"], linhas, larga=True)


def conhecida_nova():
    # as duas tabelas estao so no md da segunda rodada. as linhas das outras versoes sao iguais as da primeira
    texto = (RES / "tabela_final_2.md").read_text(encoding="utf-8")
    partes = re.split(r"### Peça (conhecida|nova)", texto)[1:]
    dados = {}
    for grupo, bloco in zip(partes[0::2], partes[1::2]):
        for linha in bloco.splitlines():
            col = [c.strip() for c in linha.strip("|").split("|")]
            if len(col) == 7 and col[0] in VERSOES + ["Aleatória (esperado)"]:
                limpa = lambda s: s.split(" (")[0].replace("%", r"\%")
                dados[(grupo, col[0])] = (limpa(col[3]), col[4], limpa(col[5]))
    linhas = []
    for nome in ["Aleatória (esperado)"] + VERSOES[1:]:
        k, n = dados[("conhecida", nome)], dados[("nova", nome)]
        linhas.append([nome, k[0], k[1], k[2], n[0], n[1], n[2]])
    tabela("conhecida_nova.tex",
           "Peça escondida conhecida (1.600 consultas) e nova (4.242), no teste (tabela\\_final\\_2.md). x = vezes o aleatório.",
           "tab:conhecida_nova",
           ["Versão", "Hit@10 conh.", "x", "FITB conh.", "Hit@10 nova", "x", "FITB nova"], linhas, larga=True)


def tempos():
    v = {}
    for f in ("kdtree", "union_find", "agm", "bfs"):
        for r in csv.DictReader(open(RES / "tempos_f" / f"{f}.csv", encoding="utf-8")):
            v[(r["medida"], int(r["n"]))] = float(r["media_ms"])
    nomes = [("construir_kdtree", "Construir a KD-tree"), ("knn_kdtree", "kNN na KD-tree (consulta)"),
             ("knn_forca_bruta", "kNN força bruta (consulta)"), ("uf_sem_tecnicas", "Union-Find sem técnicas"),
             ("uf_tamanho_e_compressao", "Union-Find com técnicas"), ("kruskal_ordenar_arestas", "Kruskal: ordenar"),
             ("kruskal_uniao", "Kruskal: uniões"), ("prim_heap", "Prim com heap"), ("bfs_2_passos", "BFS 2 passos (consulta)")]
    def fmt(x):
        if x >= 1000:
            return f"{x:,.0f}".replace(",", ".")
        # a BFS leva menos de 1 microssegundo: notacao cientifica nao fica bem na tabela
        return (f"{x:.6f}" if x < 0.001 else f"{x:.4g}").replace(".", ",")
    linhas = [[rot] + [fmt(v[(m, n)]) for n in (1000, 10000, 100000)] for m, rot in nomes]
    tabela("tempos.tex", "Tempos em ms, média de 5 repetições (tempos\\_f.md).", "tab:tempos",
           ["Medida", "$n=10^3$", "$n=10^4$", "$n=10^5$"], linhas)


def main():
    SAIDA.mkdir(parents=True, exist_ok=True)
    final()
    diferencas()
    conhecida_nova()
    tempos()


if __name__ == "__main__":
    main()
