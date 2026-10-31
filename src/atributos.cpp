#include "atributos.hpp"

#include <filesystem>
#include <fstream>

namespace {

const std::string CABECALHO =
    "id,categoria,estilo_casual,estilo_classico,estilo_esportivo,estilo_romantico,estilo_moderno,"
    "ocasiao_dia,ocasiao_trabalho,ocasiao_festa,ocasiao_esporte,ocasiao_praia,formalidade,"
    "L,a,b,cor_neutra,estampa,comprimento,modelagem";
constexpr int N_G1 = 11;
constexpr int N_COLUNAS = 20;

}  // namespace

bool ler_atributos(const std::string& caminho, const Base& base, std::vector<Atrib>& attr, std::string& erro) {
    std::ifstream arquivo(caminho);
    if (!arquivo) {
        erro = "Não foi possível abrir " + caminho;
        return false;
    }
    std::string linha;
    std::getline(arquivo, linha);
    if (!linha.empty() && linha.back() == '\r') linha.pop_back();
    if (linha != CABECALHO) {
        erro = "Cabeçalho inesperado em " + caminho;
        return false;
    }

    attr.assign(base.pecas.size(), Atrib{});
    std::vector<char> lida(base.pecas.size(), 0);
    int numero = 1;
    while (std::getline(arquivo, linha)) {
        numero++;
        if (linha.empty() || linha == "\r") continue;
        std::vector<std::string> col = separar_colunas(linha);
        if ((int)col.size() != N_COLUNAS) {
            erro = "Linha " + std::to_string(numero) + " com formato errado em " + caminho;
            return false;
        }
        try {
            auto it = base.indice_da_peca.find(std::stoi(col[0]));
            if (it == base.indice_da_peca.end()) continue;
            Atrib& a = attr[it->second];
            for (int i = 0; i < N_G1; i++) a.g1.push_back(std::stod(col[2 + i]));
            for (int i = 0; i < 3; i++) a.lab.push_back(std::stod(col[13 + i]));
            a.neutra = col[16] == "1";
            a.estampa = col[17];
            a.comprimento = col[18];
            a.modelagem = col[19];
            lida[it->second] = 1;
        } catch (const std::exception&) {
            erro = "Linha " + std::to_string(numero) + " com formato errado em " + caminho;
            return false;
        }
    }
    // peca sem atributo quebraria a distancia sem avisar
    for (int i = 0; i < (int)lida.size(); i++) {
        if (!lida[i]) {
            erro = "Peça " + std::to_string(base.pecas[i].id) + " sem atributos em " + caminho;
            return false;
        }
    }
    return true;
}

bool ler_atributos_da_pasta(const std::string& pasta, const Base& base, std::vector<Atrib>& attr, std::string& erro) {
    namespace fs = std::filesystem;
    fs::path p = fs::path(pasta) / "pecas_v2.csv";
    if (!fs::exists(p)) p = fs::path(pasta) / "pecas_v2_exemplo.csv";
    return ler_atributos(p.string(), base, attr, erro);
}
