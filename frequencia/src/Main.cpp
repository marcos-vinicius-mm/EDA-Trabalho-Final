/**
 * @file main.cpp
 * @brief Contador de frequencia de palavras — QXD0115 EDA Projeto Final
 *
 * Uso:
 *   .\Main.exe dictionary <estrutura> <arquivo.txt>
 *
 * Estruturas disponiveis:
 *   avl         Arvore AVL completamente iterativa
 *   rb          Arvore Rubro-Negra completamente iterativa
 *   hash-chain  Hash com encadeamento exterior (ChainedHashTable)
 *   hash-open   Hash com endereçamento aberto (OpenAddressingHashTable)
 *   all         Executa as quatro estruturas e exibe comparativo de metricas
 *
 * Exemplos:
 *   .\Main.exe dictionary avl livro.txt
 *   .\Main.exe dictionary hash-chain livro.txt
 *   .\Main.exe dictionary all livro.txt
 *   .\Main.exe --help
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <memory>
#include <cctype>
#include <filesystem>

#include "../include/Dictionary.h"
#include "../include/AVLTree.h"
#include "../include/RBTree.h"
#include "../include/ChainedHashTable.h"
#include "../include/OpenAddressingHashTable.h"


// Tokenizacao do arquivo de entrada

/**
 * @brief Converte letras maiusculas ASCII para minusculas.
 *        Caracteres acentuados (> 127) sao mantidos intactos.
 */
static std::string to_lower_ascii(const std::string& s) {
    std::string r = s;
    for (auto& c : r) {
        if (c >= 'A' && c <= 'Z')
            c = static_cast<char>(c + 32);
    }
    return r;
}

/**
 * @brief Remove pontuacao das extremidades de um token.
 *        Hifen no interior e mantido (ex.: "mostra-lo").
 */
static std::string strip_edges(const std::string& tok) {
    size_t l = 0, r = tok.size();
    while (l < r) {
        unsigned char c = tok[l];
        if (std::isalpha(c) || c > 127) break;
        ++l;
    }
    while (r > l) {
        unsigned char c = tok[r - 1];
        if (std::isalpha(c) || c > 127) break;
        --r;
    }
    return tok.substr(l, r - l);
}

/**
 * @brief Le o arquivo de texto e retorna vetor de palavras normalizadas.
 *
 * Regras de tokenizacao:
 *  - Espacos e sinais de pontuacao sao separadores.
 *  - Hifen no meio de palavra eh mantido ("mostra-lo").
 *  - Hifen isolado ou no inicio/fim de token eh descartado.
 *  - Todas as letras sao convertidas para minusculas (ASCII).
 *
 * @param filename caminho para o arquivo .txt
 * @return std::vector<std::string>  palavras normalizadas
 */
static std::vector<std::string> tokenize(const std::string& filename) {
    std::ifstream fin(filename);
    if (!fin.is_open()) {
        std::cerr << "Erro: nao foi possivel abrir '" << filename << "'\n";
        return {};
    }

    std::vector<std::string> words;
    std::string line;

    while (std::getline(fin, line)) {
        std::string token;

        for (size_t i = 0; i <= line.size(); ++i) {
            unsigned char c   = (i < line.size()) ? static_cast<unsigned char>(line[i]) : '\0';
            bool sep = (i == line.size()) || std::isspace(c);

            // Tratamento especial do hifen
            if (!sep && c == '-') {
                if (token.empty()) {
                    sep = true;   // hifen isolado no inicio: descarta
                } else {
                    // Hifen composto: mantem se o proximo char for letra
                    bool next_is_alpha = (i + 1 < line.size()) &&
                                        (std::isalpha(line[i + 1]) ||
                                         (unsigned char)line[i + 1] > 127);
                    if (next_is_alpha) {
                        token += static_cast<char>(c);
                        continue;
                    } else {
                        sep = true;  // hifen de dialogo ou no fim: descarta
                    }
                }
            }

            // Pontuacao ASCII nao-letra e nao-hifen: separador
            if (!sep && !std::isalpha(c) && c < 128 && c != '-')
                sep = true;

            if (!sep) {
                token += static_cast<char>(c);
            } else {
                if (!token.empty()) {
                    std::string clean = strip_edges(token);
                    if (!clean.empty())
                        words.push_back(to_lower_ascii(clean));
                    token.clear();
                }
            }
        }
    }
    return words;
}


// Construcao da tabela de frequencias

/**
 * @brief Percorre o vetor de palavras e popula o dicionario com as frequencias.
 *        Usa insert() para a primeira ocorrencia e at() para incrementar.
 *        Compativel com qualquer implementacao de Dictionary<string,int>.
 *
 * @param dict   dicionario a ser populado
 * @param words  vetor de palavras ja normalizadas
 */
static void build_frequency(Dictionary<std::string, int>& dict,
                             const std::vector<std::string>& words) {
    for (const auto& w : words) {
        if (!dict.insert(w, 1)) {   // insert retorna false se chave ja existia
            dict.at(w)++;           // então incrementa o contador
        }
    }
}


// Exibição de métricas

/**
 * @brief Imprime as métricas coletadas por uma estrutura de dados.
 *
 * @param name         nome amigavel da estrutura
 * @param m            metricas coletadas
 * @param word_count   total de tokens processados
 * @param unique_count total de palavras unicas inseridas
 */
static void print_metrics(const std::string& name,
                           const Metrics& m,
                           size_t word_count,
                           size_t unique_count) 
{
    std::cout << "\n+------------------------------------------+\n";
    std::cout << "| Estrutura : " << std::left << std::setw(29) << name << "|\n";
    std::cout << "+------------------------------------------+\n";
    std::cout << "| Palavras processadas  : " << std::setw(17) << word_count    << "|\n";
    std::cout << "| Palavras unicas       : " << std::setw(17) << unique_count  << "|\n";
    std::cout << "| Comparacoes de chave  : " << std::setw(17) << m.comparisons << "|\n";
    if (m.rotations > 0)
        std::cout << "| Rotacoes              : " << std::setw(17) << m.rotations   << "|\n";
    if (m.collisions > 0)
        std::cout << "| Colisoes (hash)       : " << std::setw(17) << m.collisions  << "|\n";
    if (m.reinsertions > 0)
        std::cout << "| Reinsercoes (rehash)  : " << std::setw(17) << m.reinsertions<< "|\n";
    std::cout << "| Tempo de construcao   : " << std::setw(13) << std::fixed
              << std::setprecision(3) << m.elapsed_ms << " ms |\n";
    std::cout << "+------------------------------------------+\n";
}


// Escrita do CSV de saida

/**
 * @brief Grava o CSV com cabeçalho "palavra,frequencia" em ordem alfabetica.
 *
 * @param dict dicionario populado
 * @param out_path  caminho do arquivo de saida
 * @return true se gravado com sucesso
 */
static bool write_csv(Dictionary<std::string, int>& dict,
                      const std::string& out_path) {
    std::ofstream fout(out_path);
    if (!fout.is_open()) {
        std::cerr << "Erro: nao foi possivel criar '" << out_path << "'\n";
        return false;
    }
    fout << "palavra,frequencia\n";
    dict.print_csv(fout);
    std::cout << "CSV gerado: " << out_path << "\n";
    return true;
}


// Fabrica de dicionarios e nomes amigaveis

using DictPtr = std::unique_ptr<Dictionary<std::string, int>>;

/**
 * @brief Instancia a estrutura de dados correspondente ao tipo solicitado.
 *
 * @param type  "avl" | "rb" | "hash-chain" | "hash-open"
 * @return DictPtr  ponteiro para Dictionary, ou nullptr se tipo inválido
 */
static DictPtr make_dict(const std::string& type) {
    if (type == "avl")        return std::make_unique<AVLTree<std::string, int>>();
    if (type == "rb")         return std::make_unique<RBTree<std::string, int>>();
    if (type == "hash-chain") return std::make_unique<ChainedHashTable<std::string, int>>();
    if (type == "hash-open")  return std::make_unique<OpenAddressingHashTable<std::string, int>>();
    return nullptr;
}

static std::string friendly_name(const std::string& type) {
    if (type == "avl")        return "AVL (iterativa)";
    if (type == "rb")         return "Rubro-Negra (iterativa)";
    if (type == "hash-chain") return "Hash - Encadeamento Exterior";
    if (type == "hash-open")  return "Hash - Enderecamento Aberto";
    return type;
}


// Execução de uma estrutura

/**
 * @brief Cria o dicionario, popula com as frequencias, mede o tempo,
 *        exibe as metricas e (opcionalmente) grava o CSV.
 *
 * @param type tipo da estrutura
 * @param words vetor de palavras tokenizadas
 * @param out_csv caminho do CSV de saida (vazio = nao grava)
 * @param write_output true se deve gravar o CSV
 */
static void run_structure(const std::string& type,
                           const std::vector<std::string>& words,
                           const std::string& out_csv,
                           bool write_output) 
{
    DictPtr dict = make_dict(type);
    if (!dict) {
        std::cerr << "Estrutura desconhecida: " << type << "\n";
        return;
    }

    // Mede o tempo de construcao da tabela de frequencias
    auto t0 = std::chrono::high_resolution_clock::now();
    build_frequency(*dict, words);
    auto t1 = std::chrono::high_resolution_clock::now();

    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    const_cast<Metrics&>(dict->metrics()).elapsed_ms = ms;

    print_metrics(friendly_name(type), dict->metrics(), words.size(), dict->size());

    if (write_output && !out_csv.empty())
        write_csv(*dict, out_csv);
}


// Ajuda

static void print_help(const char* prog) {
    std::cout
        << "Uso:\n"
        << "  " << prog << " dictionary <estrutura> <arquivo.txt> [saida.csv]\n\n"
        << "Estruturas disponiveis:\n"
        << "  avl         Arvore AVL completamente iterativa\n"
        << "  rb          Arvore Rubro-Negra completamente iterativa\n"
        << "  hash-chain  Hash com encadeamento exterior (ChainedHashTable)\n"
        << "  hash-open   Hash com enderecamento aberto  (OpenAddressingHashTable)\n"
        << "  all         Executa as quatro estruturas e exibe comparativo\n\n"
        << "Exemplos:\n"
        << "  " << prog << " dictionary avl        livro.txt\n"
        << "  " << prog << " dictionary hash-chain livro.txt\n"
        << "  " << prog << " dictionary all        livro.txt\n\n"
        << "Opcoes:\n"
        << "  --help   Exibe esta mensagem de ajuda\n\n"
        << "Formato do CSV de saida:\n"
        << "  palavra,frequencia   (cabecalho)\n"
        << "  <palavra>,<n>        (uma linha por palavra, em ordem alfabetica)\n\n";
}


// main

int main(int argc, char* argv[]) {

    // Verifica --help
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--help" || std::string(argv[i]) == "-h") {
            print_help(argv[0]);
            return 0;
        }
    }

    // Valida numero minimo de argumentos
    if (argc < 4) {
        std::cerr << "Uso: " << argv[0]
                  << " dictionary <estrutura> <arquivo.txt> [saida.csv]\n"
                  << "Use --help para mais informacoes.\n\n";
        return 1;
    }

    std::string subcmd   = argv[1];
    std::string type     = argv[2];
    std::string txt_arg  = argv[3];   // nome do livro passado pelo usuario
    std::string csv_file = (argc >= 5) ? argv[4] : "";

    if (subcmd != "dictionary") {
        std::cerr << "Subcomando desconhecido: '" << subcmd << "'\n"
                  << "Use: " << argv[0] << " dictionary ...\n";
        return 1;
    }

    // Pasta onde os livros estao armazenados.
    const std::string books_dir = "../../include/test_books/";

    // Monta o caminho completo: se o usuario ja passou um caminho com
    // separador (/ ou \), usa direto; caso contrario, prefixo books_dir.
    std::string txt_file = txt_arg;
    if (txt_arg.find('/') == std::string::npos &&
        txt_arg.find('\\') == std::string::npos) {
        txt_file = books_dir + txt_arg;
        // Adiciona .txt automaticamente se o usuario nao colocou
        if (txt_file.size() < 4 ||
            txt_file.substr(txt_file.size() - 4) != ".txt") {
            txt_file += ".txt";
        }
    }

    // Le e tokeniza o arquivo uma unica vez (compartilhado por todas as estruturas)
    std::cout << "Lendo arquivo: " << txt_file << " ...\n";
    std::vector<std::string> words = tokenize(txt_file);

    if (words.empty()) {
        std::cerr << "Nenhuma palavra encontrada ou arquivo invalido.\n\n";
        return 1;
    }
    std::cout << "Total de tokens lidos: " << words.size() << "\n";

    // Extrai o nome do livro a partir do caminho do arquivo .txt
    // Ex.: "livros/dom_casmurro.txt" => "dom_casmurro"
    std::filesystem::path txt_path(txt_file);
    std::string book_name = txt_path.stem().string();   // nome sem extensão

    // Pasta de saida: sheet_results/<nome_do_livro>/
    // Criada automaticamente se não existir
    const std::string output_dir = "sheet_results/" + book_name + "/";
    std::filesystem::create_directories(output_dir);

    if (type == "all") {
        // Executa as quatro estruturas em sequencia.
        // Cada estrutura gera seu proprio CSV em sheet_results/<livro>/.
        // Ex.: sheet_results/dom_casmurro/avl.csv
        const std::vector<std::string> all_types = {"avl", "rb", "hash-chain", "hash-open"};
        for (const auto& t : all_types) {
            std::string out = output_dir + t + ".csv";
            run_structure(t, words, out, true);
        }
    } else {
        // Estrutura unica: usa o nome fornecido pelo usuario,
        // ou <estrutura>.csv como padrão, dentro de sheet_results/<livro>/.
        std::string out = output_dir + (csv_file.empty() ? type + ".csv" : csv_file);
        run_structure(type, words, out, true);
    }

    return 0;
}