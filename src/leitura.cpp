#include "leitura.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace {

std::vector<std::string> separar_colunas(std::string linha) {
    if (!linha.empty() && linha.back() == '\r') linha.pop_back();
    std::vector<std::string> colunas;
    std::stringstream fluxo(linha);
    std::string coluna;
    while (std::getline(fluxo, coluna, ',')) colunas.push_back(coluna);
    return colunas;
}

int posicao_coluna(const std::vector<std::string>& cabecalho, const std::string& nome) {
    const auto it = std::find(cabecalho.begin(), cabecalho.end(), nome);
    return it == cabecalho.end() ? -1 : static_cast<int>(it - cabecalho.begin());
}

std::string erro_na_linha(const std::string& caminho, int numero) {
    return "Linha " + std::to_string(numero) + " com formato errado em " + caminho;
}

}  // namespace

bool ler_pecas(const std::string& caminho, Base& base, std::string& erro) {
    std::ifstream arquivo(caminho);
    if (!arquivo) {
        erro = "Não foi possível abrir " + caminho;
        return false;
    }

    std::string linha;
    if (!std::getline(arquivo, linha)) {
        erro = "Arquivo vazio: " + caminho;
        return false;
    }
    const std::vector<std::string> cabecalho = separar_colunas(linha);
    const int col_id = posicao_coluna(cabecalho, "id");
    const int col_categoria = posicao_coluna(cabecalho, "categoria");
    const int col_nome = posicao_coluna(cabecalho, "nome");
    if (col_id < 0 || col_categoria < 0) {
        erro = "Faltam as colunas id e categoria em " + caminho;
        return false;
    }

    int numero = 1;
    while (std::getline(arquivo, linha)) {
        numero++;
        if (linha.empty() || linha == "\r") continue;
        const std::vector<std::string> colunas = separar_colunas(linha);
        if (colunas.size() != cabecalho.size()) {
            erro = erro_na_linha(caminho, numero);
            return false;
        }
        Peca peca;
        try {
            peca.id = std::stoi(colunas[col_id]);
            peca.categoria = colunas[col_categoria];
            if (col_nome >= 0) peca.nome = colunas[col_nome];
            for (int i = 0; i < static_cast<int>(colunas.size()); i++) {
                if (i != col_id && i != col_categoria && i != col_nome) {
                    peca.v.push_back(std::stod(colunas[i]));
                }
            }
        } catch (const std::exception&) {
            erro = erro_na_linha(caminho, numero);
            return false;
        }
        base.indice_da_peca[peca.id] = static_cast<int>(base.pecas.size());
        base.pecas.push_back(peca);
    }
    return true;
}

bool ler_looks(const std::string& caminho, Base& base, std::string& erro) {
    std::ifstream arquivo(caminho);
    if (!arquivo) {
        erro = "Não foi possível abrir " + caminho;
        return false;
    }

    std::string linha;
    if (!std::getline(arquivo, linha)) {
        erro = "Arquivo vazio: " + caminho;
        return false;
    }
    const std::vector<std::string> cabecalho = separar_colunas(linha);
    const int col_look = posicao_coluna(cabecalho, "look_id");
    const int col_peca = posicao_coluna(cabecalho, "peca_id");
    const int col_conjunto = posicao_coluna(cabecalho, "conjunto");
    if (col_look < 0 || col_peca < 0 || col_conjunto < 0) {
        erro = "Faltam as colunas look_id, peca_id e conjunto em " + caminho;
        return false;
    }

    int numero = 1;
    while (std::getline(arquivo, linha)) {
        numero++;
        if (linha.empty() || linha == "\r") continue;
        const std::vector<std::string> colunas = separar_colunas(linha);
        if (colunas.size() != cabecalho.size()) {
            erro = erro_na_linha(caminho, numero);
            return false;
        }
        int look_id = 0;
        int peca_id = 0;
        try {
            look_id = std::stoi(colunas[col_look]);
            peca_id = std::stoi(colunas[col_peca]);
        } catch (const std::exception&) {
            erro = erro_na_linha(caminho, numero);
            return false;
        }
        const auto peca = base.indice_da_peca.find(peca_id);
        if (peca == base.indice_da_peca.end()) continue;

        auto look = base.indice_do_look.find(look_id);
        if (look == base.indice_do_look.end()) {
            Look novo;
            novo.id = look_id;
            novo.treino = colunas[col_conjunto] == "treino";
            look = base.indice_do_look.emplace(look_id, static_cast<int>(base.looks.size())).first;
            base.looks.push_back(novo);
        }
        base.looks[look->second].pecas.push_back(peca->second);
    }
    return true;
}

bool ler_base(const std::string& pasta, Base& base, std::string& erro) {
    namespace fs = std::filesystem;
    const fs::path raiz(pasta);
    fs::path pecas = raiz / "pecas.csv";
    fs::path looks = raiz / "looks.csv";
    if (!fs::exists(pecas) && fs::exists(raiz / "pecas_exemplo.csv")) {
        pecas = raiz / "pecas_exemplo.csv";
        looks = raiz / "looks_exemplo.csv";
    }
    if (!fs::exists(pecas)) {
        erro = "Não achei pecas.csv em " + pasta + ". Veja dados/README.md ou use --dados dados/exemplo.";
        return false;
    }
    return ler_pecas(pecas.string(), base, erro) && ler_looks(looks.string(), base, erro);
}
