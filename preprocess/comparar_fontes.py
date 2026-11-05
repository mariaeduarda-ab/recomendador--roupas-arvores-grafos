#!/usr/bin/env python3
# Compara texto, CLIP e as combinacoes com a referencia rotulada pela imagem.
# Uso: python3 preprocess/comparar_fontes.py --texto <pasta com pecas_v2.csv das listas atuais>
#   --entrada Combined_Metadata.json e opcional: com ele a formalidade 0,5 com palavra (empate)
#   nao conta como desconhecido. Sem ele, todo 0,5 vira desconhecido.

import argparse
import csv
import math
from pathlib import Path

from referencia import NA

ATRS = ["estampa", "comprimento", "modelagem", "estilo", "ocasiao", "formalidade"]
DESC = "desconhecido"
FORA = (NA, "incerto")
MINIMO = 0.70

ESTAMPA = {"lisa": "lisa", "listrada": "listrada", "xadrez": "xadrez", "floral": "floral", "poa": "poá",
           "animal": "animal print", "outra": "outra estampa"}
COMPRIMENTO = {"curto": "curto", "medio": "médio", "longo": "longo"}
ESTILO = {"casual": "casual", "classico": "clássico", "esportivo": "esportivo", "romantico": "romântico",
          "moderno": "moderno"}
OCASIAO = {"dia": "dia a dia", "trabalho": "trabalho", "festa": "festa", "esporte": "esporte", "praia": "praia"}
FORMAL = ["baixa", "média", "alta"]

FONTES = ["texto", "clip", "comb", "lisa"]
NOME_FONTE = {"texto": "texto", "clip": "CLIP", "comb": "combinação", "lisa": "texto+lisa"}


def ler(caminho):
    with open(caminho, encoding="utf-8-sig", newline="") as arq:
        return {int(l["id"]): l for l in csv.DictReader(arq)}


def wilson(k, n, z=1.96):
    if n == 0:
        return 0.0, 0.0
    p = k / n
    centro = (p + z * z / (2 * n)) / (1 + z * z / n)
    meio = z * math.sqrt(p * (1 - p) / n + z * z / (4 * n * n)) / (1 + z * z / n)
    return max(0.0, centro - meio), min(1.0, centro + meio)


def pct(x):
    return f"{100 * x:.1f}".replace(".", ",")


def num(x):
    return f"{x:.3f}".replace(".", ",")


def fmt(k, n):
    if n == 0:
        return "sem peças"
    lo, hi = wilson(k, n)
    return f"{pct(k / n)}% ({pct(lo)} a {pct(hi)}; {k}/{n})"


def palavras_formalidade(entrada, ids):
    # conta as palavras da mesma forma que o atributos.py, so para saber se o 0,5 e empate ou falta de palavra
    if not entrada or not Path(entrada).exists():
        print("sem --entrada: toda formalidade 0,5 conta como desconhecido.")
        return None
    import re
    from atributos import limpar
    from preprocessar import PALAVRAS_CASUAIS, PALAVRAS_FORMAIS, carregar_pecas
    pecas = carregar_pecas(entrada)
    saida = {}
    for i in ids:
        if i >= len(pecas):
            continue
        p = pecas[i]
        ps = re.findall(r"[a-z]+(?:-[a-z]+)*", limpar(p["descricao"], p["categoria"]))
        saida[i] = sum(w in PALAVRAS_FORMAIS or w in PALAVRAS_CASUAIS for w in ps)
    return saida


def tem_palavra_formal(i, linha, contagem):
    f = float(linha["formalidade"])
    if f != 0.5:
        return True
    return bool(contagem and contagem.get(i, 0) > 0)


def formal_texto(f, cortes):
    c1, c2 = cortes
    return "baixa" if f < c1 else ("alta" if f >= c2 else "média")


def escolher_cortes(pares):
    # pares: (valor do texto, rotulo). busca exaustiva nos pontos medios, empate fica com o par mais perto de 1/3 e 2/3
    vals = sorted({f for f, _ in pares})
    cand = [0.0] + [(a + b) / 2 for a, b in zip(vals, vals[1:])] + [1.0001]
    melhor = None
    for i, c1 in enumerate(cand):
        for c2 in cand[i:]:
            k = sum(formal_texto(f, (c1, c2)) == r for f, r in pares)
            chave = (k, -abs(c1 - 1 / 3) - abs(c2 - 2 / 3))
            if melhor is None or chave > melhor[0]:
                melhor = (chave, (c1, c2))
    return melhor[1], melhor[0][0]


def multi(linha, prefixo, mapa):
    return [mapa[v] for v in mapa if linha[f"{prefixo}_{v}"] == "1"]


def resposta_texto(atr, i, linha, cortes, contagem):
    # devolve a lista de valores marcados (vazia = desconhecido)
    if atr == "estampa":
        v = linha["estampa"]
        return [ESTAMPA[v]] if v in ESTAMPA else []
    if atr == "comprimento":
        v = linha["comprimento"]
        return [COMPRIMENTO[v]] if v in COMPRIMENTO else []
    if atr == "modelagem":
        v = linha["modelagem"]
        return [v] if v != DESC else []
    if atr == "estilo":
        return multi(linha, "estilo", ESTILO)
    if atr == "ocasiao":
        return multi(linha, "ocasiao", OCASIAO)
    if not tem_palavra_formal(i, linha, contagem):
        return []
    return [formal_texto(float(linha["formalidade"]), cortes)]


def resposta_clip(atr, linha):
    if linha is None:
        return []
    v = linha[atr]
    return [] if v in FORA or v == "" else [v]


def respostas(atr, i, texto, clip, cortes, contagem):
    t = resposta_texto(atr, i, texto, cortes, contagem)
    c = resposta_clip(atr, clip)
    r = {"texto": t, "clip": c, "comb": t or c}
    if atr == "estampa":
        r["lisa"] = t or ["lisa"]
    return r


def contar(lista):
    # lista de (marcados, ref). total, acertos solto, acertos estrito, respondidas, varios marcados
    c = {"n": 0, "ok": 0, "estrito": 0, "resp": 0, "varios": 0}
    for marcados, ref in lista:
        c["n"] += 1
        if marcados:
            c["resp"] += 1
        if len(marcados) > 1:
            c["varios"] += 1
        if ref in marcados:
            c["ok"] += 1
            if len(marcados) == 1:
                c["estrito"] += 1
    return c


def avaliar(ids, ref, texto, clip, cortes, contagem):
    res = {}
    for atr in ATRS:
        por_fonte = {}
        for i in ids:
            r = ref[i][atr]
            if r in FORA:
                continue
            for f, m in respostas(atr, i, texto[i], clip.get(i), cortes, contagem).items():
                por_fonte.setdefault(f, []).append((m, r))
        res[atr] = {f: contar(l) for f, l in por_fonte.items()}
    return res


def tabela_principal(res, fontes, s):
    s.append("| atributo | n | " + " | ".join(NOME_FONTE[f] for f in fontes) + " | "
             + " | ".join(f"% desc. {NOME_FONTE[f]}" for f in fontes) + " |")
    s.append("|" + " --- |" * (2 + 2 * len(fontes)))
    for atr in ATRS:
        linha = res[atr]
        n = next(iter(linha.values()))["n"] if linha else 0
        cel = [fmt(linha[f]["ok"], linha[f]["n"]) if f in linha else "-" for f in fontes]
        des = [pct(1 - linha[f]["resp"] / linha[f]["n"]) + "%" if f in linha and linha[f]["n"] else "-" for f in fontes]
        s.append(f"| {atr} | {n} | " + " | ".join(cel) + " | " + " | ".join(des) + " |")


def tabela_responde(res, fontes, s):
    s.append("| atributo | " + " | ".join(NOME_FONTE[f] for f in fontes) + " |")
    s.append("|" + " --- |" * (1 + len(fontes)))
    for atr in ATRS:
        linha = res[atr]
        s.append(f"| {atr} | " + " | ".join(fmt(linha[f]["ok"], linha[f]["resp"]) if f in linha else "-"
                                             for f in fontes) + " |")


def tabela_multi(res, s):
    s.append("| atributo | fonte | acerto total (solto) | acerto total (estrito) | peças com vários valores |")
    s.append("| --- | --- | --- | --- | --- |")
    for atr in ("estilo", "ocasiao"):
        for f in ("texto", "comb"):
            c = res[atr][f]
            s.append(f"| {atr} | {NOME_FONTE[f]} | {fmt(c['ok'], c['n'])} | {fmt(c['estrito'], c['n'])} | "
                     f"{c['varios']} de {c['n']} ({pct(c['varios'] / c['n'])}%) |")


def melhor_fonte(linha):
    f = max(linha, key=lambda x: (linha[x]["ok"] / linha[x]["n"] if linha[x]["n"] else 0, x == "texto"))
    return f, (linha[f]["ok"] / linha[f]["n"] if linha[f]["n"] else 0.0)


def concordancia(caminho, ref, texto, clip, cortes, contagem, s):
    revisao = ler(caminho)
    ok = {i: r for i, r in revisao.items() if r.get("confirmada", "").strip() == "sim"}
    s.append(f"Arquivo lido: `{caminho}`. Peças: {len(revisao)}. Confirmadas: {len(ok)}. "
             "Só as confirmadas entram.")
    s.append("")
    faltam = [i for i in ok if i not in ref]
    if faltam:
        s.append(f"Ids da revisão fora da referência, ignorados: {faltam}.")
        s.append("")
    s.append("### Rótulos dos agentes contra a revisão")
    s.append("")
    s.append("Conta só peça em que a revisão não diz \"não se aplica\" nem \"incerto\".")
    s.append("")
    s.append("| atributo | concordância |")
    s.append("| --- | --- |")
    for atr in ATRS:
        k = n = 0
        for i, r in ok.items():
            if i not in ref or r[atr] in FORA:
                continue
            n += 1
            k += ref[i][atr] == r[atr]
        s.append(f"| {atr} | {fmt(k, n)} |")
    s.append("")
    s.append("A revisão abriu com os rótulos dos agentes já preenchidos. Quem revisa tende a manter o valor que já está "
             "na tela. Então a concordância real deve ser menor que a da tabela.")
    s.append("")
    s.append("### Formalidade das fontes contra a revisão")
    s.append("")
    s.append("| fonte | acerto total | acerto quando responde |")
    s.append("| --- | --- | --- |")
    lista = {}
    for i, r in ok.items():
        if i not in texto or r["formalidade"] in FORA:
            continue
        for f, m in respostas("formalidade", i, texto[i], clip.get(i), cortes, contagem).items():
            lista.setdefault(f, []).append((m, r["formalidade"]))
    for f in ("texto", "clip", "comb"):
        c = contar(lista.get(f, []))
        s.append(f"| {NOME_FONTE[f]} | {fmt(c['ok'], c['n'])} | {fmt(c['ok'], c['resp'])} |")
    s.append("")
    s.append("As 50 peças vêm das duas metades. Os cortes da formalidade do texto saíram da metade de ajuste, "
             "então parte delas já entrou na escolha dos cortes.")


REGRAS = """## Regras da comparação

1. Peças com `categoria_errada=sim` ficam fora de toda conta. A lista está abaixo.
2. Valores "não se aplica" e "incerto" na referência não contam.
3. Todas as fontes são comparadas nas mesmas peças: a metade de teste. As frases do CLIP foram ajustadas na metade de ajuste. As fontes só de texto também aparecem nas 400 peças, numa tabela separada.
4. Cada fonte tem dois números. "Acerto total" conta a falta de resposta ("desconhecido") como erro. "Acerto quando responde" conta só as peças com resposta. A regra dos 70% usa o acerto total, porque o atributo precisa servir para todas as peças do sistema.
5. Todo acerto vem com o intervalo de Wilson de 95% e com a contagem.
6. Valores do texto: estampa lisa, listrada, xadrez, floral, poa vira poá, animal vira animal print, outra vira outra estampa. Comprimento medio vira médio. Modelagem fica igual. Estilo e ocasião no texto podem ter vários valores marcados. Se o texto marca um só, acerta quando bate. Se marca vários, acerta quando a referência está entre eles (versão solta). A versão estrita, em que vários valores contam como erro, está numa tabela à parte. Formalidade no texto é um número de 0 a 1. Dois cortes, escolhidos só na metade de ajuste, viram baixa, média e alta. O valor 0,5 sem nenhuma palavra conta como desconhecido.
7. Fontes: (a) texto; (b) CLIP; (c) combinação, que usa o texto quando ele achou palavra e o CLIP nos outros casos; (d) só na estampa, texto+lisa, que usa o texto e dá "lisa" quando não há palavra de estampa.
8. Um atributo entra só se a melhor fonte chega a 70% de acerto total na metade de teste.
9. A formalidade do CLIP é "não se aplica" em joias e acessórios por decisão do ajuste. No acerto total isso conta como desconhecido."""


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--texto", required=True)
    parser.add_argument("--clip", default="dados/atributos_clip.csv")
    parser.add_argument("--ref", default="dados/referencia")
    parser.add_argument("--entrada")
    parser.add_argument("--revisao", default="revisao/revisao_50.csv")
    parser.add_argument("--saida", default="resultados/comparacao_fontes.md")
    args = parser.parse_args()

    pasta = Path(args.ref)
    ref = {**ler(pasta / "referencia_300.csv"), **ler(pasta / "referencia_extra_100.csv")}
    metades = ler(pasta / "metades.csv")
    clip = ler(args.clip)
    errados = sorted(i for i, l in metades.items() if l["categoria_errada"] == "sim")
    validos = [i for i in sorted(ref) if i in metades and metades[i]["categoria_errada"] == "nao"]
    ajuste = [i for i in validos if metades[i]["metade"] == "ajuste"]
    teste = [i for i in validos if metades[i]["metade"] == "teste"]

    # le so as linhas que importam: o pecas_v2.csv tem 100 mil
    quero = set(ref) | set(clip)
    texto = {}
    with open(Path(args.texto) / "pecas_v2.csv", encoding="utf-8") as arq:
        for l in csv.DictReader(arq):
            i = int(l["id"])
            if i in quero:
                texto[i] = l
    sem_texto = [i for i in ref if i not in texto]
    if sem_texto:
        raise SystemExit(f"ids da referência sem linha no texto: {sem_texto[:10]}")
    sem_clip = [i for i in validos if i not in clip]
    for i in validos:
        if texto[i]["categoria"] != ref[i]["categoria"]:
            raise SystemExit(f"id {i}: categoria {texto[i]['categoria']} no texto e {ref[i]['categoria']} na referência")

    contagem = palavras_formalidade(args.entrada, sorted(quero))
    pares = [(float(texto[i]["formalidade"]), ref[i]["formalidade"]) for i in ajuste
             if ref[i]["formalidade"] not in FORA and tem_palavra_formal(i, texto[i], contagem)]
    cortes, k_cortes = escolher_cortes(pares)

    res_teste = avaliar(teste, ref, texto, clip, cortes, contagem)
    res_todos = avaliar(validos, ref, texto, clip, cortes, contagem)

    s = ["# Comparação das fontes de atributo", ""]
    s.append("Gerado por `preprocess/comparar_fontes.py`. A pergunta é qual fonte de cada atributo é boa o bastante "
             "para entrar no sistema.")
    s.append("")
    s.append(REGRAS)
    s.append("")
    s.append("## Dados")
    s.append("")
    s.append(f"- Referência: {len(ref)} peças rotuladas pela imagem. Válidas: {len(validos)}. "
             f"Ajuste: {len(ajuste)}. Teste: {len(teste)}.")
    s.append(f"- Texto: `pecas_v2.csv` gerado com as listas atuais de `palavras_chave.py` em `{args.texto}`.")
    s.append(f"- CLIP: `{args.clip}`. Peças válidas sem linha do CLIP: {len(sem_clip)} (contam como desconhecido).")
    if contagem is None:
        s.append("- Formalidade: rodado sem o JSON. Todo 0,5 conta como desconhecido, mesmo o empate entre palavras.")
    else:
        empates = sum(1 for i in validos if float(texto[i]["formalidade"]) == 0.5 and contagem.get(i, 0) > 0)
        s.append(f"- Formalidade: o JSON foi lido para separar o empate (0,5 com palavra) da falta de palavra. "
                 f"Empates entre as válidas: {empates}.")
    s.append("")
    s.append("### Categoria errada na base")
    s.append("")
    s.append(f"{len(errados)} peças mostram na imagem outra coisa que a categoria da base. Elas ficam fora das contas. "
             "Isso é um problema da base, não das fontes.")
    s.append("")
    mostra = {}
    if (pasta / "categoria_errada.csv").exists():
        mostra = {i: l.get("o_que_a_imagem_mostra", "") for i, l in ler(pasta / "categoria_errada.csv").items()}
    s.append("| id | categoria | metade | o que a imagem mostra |")
    s.append("| --- | --- | --- | --- |")
    for i in errados:
        s.append(f"| {i} | {metades[i]['categoria']} | {metades[i]['metade']} | {mostra.get(i, '')} |")
    s.append("")
    s.append("## Cortes da formalidade do texto")
    s.append("")
    baixa = f"baixa abaixo de {num(cortes[0])}" if cortes[0] > 0 else "nenhum valor vira baixa"
    alta = f"alta a partir de {num(cortes[1])}" if cortes[1] <= 1 else "nenhum valor vira alta"
    s.append(f"Escolhidos na metade de ajuste, com {len(pares)} peças que têm palavra: {baixa}, {alta}, "
             f"média no resto. No ajuste esses cortes acertam {k_cortes} de {len(pares)}.")
    s.append("")
    s.append("Valor do texto contra o rótulo, na metade de ajuste (só peças com palavra):")
    s.append("")
    s.append("| valor do texto | baixa | média | alta |")
    s.append("| --- | --- | --- | --- |")
    for f in sorted({f for f, _ in pares}):
        s.append(f"| {num(f)} | " + " | ".join(str(sum(1 for x, r in pares if x == f and r == v)) for v in FORMAL) + " |")
    s.append("")
    s.append("## Resultado na metade de teste")
    s.append("")
    s.append("Acerto total, com intervalo de 95% e contagem, e a parte de desconhecido de cada fonte. "
             "Em estilo e ocasião vale a versão solta.")
    s.append("")
    tabela_principal(res_teste, FONTES, s)
    s.append("")
    s.append("### Acerto quando responde (teste)")
    s.append("")
    tabela_responde(res_teste, FONTES, s)
    s.append("")
    s.append("### Estilo e ocasião: versão solta e estrita (teste)")
    s.append("")
    tabela_multi(res_teste, s)
    s.append("")
    s.append("## Só texto, nas 400 peças (linha secundária)")
    s.append("")
    s.append("As fontes de texto não foram ajustadas na referência, então podem ser medidas nas duas metades. "
             "Exceção: os cortes da formalidade saíram do ajuste, e aqui o ajuste entra junto.")
    s.append("")
    so_texto = {a: {f: c for f, c in l.items() if f in ("texto", "lisa")} for a, l in res_todos.items()}
    tabela_principal(so_texto, ["texto", "lisa"], s)
    s.append("")
    tabela_responde(so_texto, ["texto", "lisa"], s)
    s.append("")

    s.append("## Decisão")
    s.append("")
    s.append("| atributo | melhor fonte | acerto total no teste | entra? |")
    s.append("| --- | --- | --- | --- |")
    decisao = {}
    for atr in ATRS:
        f, a = melhor_fonte(res_teste[atr])
        c = res_teste[atr][f]
        decisao[atr] = (f, a >= MINIMO)
        s.append(f"| {atr} | {NOME_FONTE[f]} | {fmt(c['ok'], c['n'])} | {'sim' if a >= MINIMO else 'não'} |")
    s.append("")
    for atr in ATRS:
        f, entra = decisao[atr]
        c = res_teste[atr][f]
        lo, hi = wilson(c["ok"], c["n"])
        if entra:
            nota = f"- **{atr}** entra com a fonte {NOME_FONTE[f]}."
            if lo < MINIMO:
                nota += f" O limite de baixo do intervalo ({pct(lo)}%) fica abaixo de 70%, então a margem é pequena."
        else:
            nota = (f"- **{atr}** fica fora. A melhor fonte ({NOME_FONTE[f]}) acerta {pct(c['ok'] / c['n'])}% "
                    f"no total, abaixo de 70%. Vai para o artigo como limitação.")
            if c["resp"] and c["ok"] / c["resp"] >= MINIMO:
                nota += (f" Quando responde, acerta {pct(c['ok'] / c['resp'])}%. O problema é a falta de resposta, "
                         "não o erro.")
        s.append(nota)
    s.append("")
    if contagem is not None:
        s.append("Na formalidade, o 0,5 de empate não conta como desconhecido. Na tabela, isso só muda o texto e a "
                 "combinação.")
        s.append("")
    fora_g1 = [a for a in ("estilo", "ocasiao", "formalidade") if not decisao[a][1]]
    if fora_g1:
        s.append("### Efeito na versão D")
        s.append("")
        s.append("O vetor de busca da versão D é o grupo 1: 5 colunas de estilo, 5 de ocasião e a formalidade. "
                 f"Ficam fora: {', '.join(fora_g1)}. No arquivo final, estilo e ocasião fora ficam com todas as "
                 "colunas em 0 e a formalidade fora fica em 0,5. O código da D já lê esses valores.")
        if len(fora_g1) == 3:
            s.append("")
            s.append("Com os três fora, o vetor de busca fica igual para todas as peças. A KD-tree deixa de separar "
                     "peças e a parte \"a\" da nota (proximidade) vale 1 para todo par. A D passa a depender só "
                     "da cor, da estampa e do bônus dos looks. Isso precisa ser decidido antes de rodar a D com o "
                     "arquivo final.")
        s.append("")

    s.append("## Revisão manual (50 peças)")
    s.append("")
    if Path(args.revisao).exists():
        concordancia(args.revisao, ref, texto, clip, cortes, contagem, s)
    else:
        s.append(f"Pendente. O arquivo `{args.revisao}` ainda não existe. Quando a revisão for exportada, basta rodar "
                 "o script de novo. Ele mede a concordância dos rótulos dos agentes com a revisão e compara a "
                 "formalidade do texto e do CLIP com as 50 peças.")
    s.append("")

    texto_md = "\n".join(s)
    Path(args.saida).write_text(texto_md, encoding="utf-8")
    print(texto_md)


if __name__ == "__main__":
    main()
