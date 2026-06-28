/**
 * @file Main.cpp
 * @brief Contador de frequencia de palavras — QXD0115 EDA Projeto Final
 *
 * Uso:
 *   ./freq dictionary <estrutura> <arquivo.txt> [saida.csv]
 *
 * Estruturas disponiveis:
 *   avl         Arvore AVL completamente iterativa
 *   rb          Arvore Rubro-Negra completamente iterativa
 *   hash-chain  Hash com encadeamento exterior (ChainedHashTable)
 *   hash-open   Hash com endereçamento aberto (OpenAddressingHashTable)
 *   all         Executa as quatro estruturas e exibe comparativo de metricas
 *
 * Exemplos:
 *   ./freq dictionary avl dom-casmurro
 *   ./freq dictionary hash-chain sherlock_holmes
 *   ./freq dictionary all dom-casmurro
 *   ./freq --help
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
 *        Maiusculas acentuadas em UTF-8 tambem sao convertidas.
 */
static std::string to_lower(const std::string& s) {
    std::string r;
    r.reserve(s.size());

    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(s[i]);

        if (c >= 'A' && c <= 'Z') {
            r += static_cast<char>(c + 32);
        } else if (c == 0xC3 && i + 1 < s.size()) {
            unsigned char c2 = static_cast<unsigned char>(s[i + 1]);
            if (c2 >= 0x80 && c2 <= 0x9E) {
                r += static_cast<char>(c);
                r += static_cast<char>(c2 + 0x20);
                ++i;
            } else {
                r += static_cast<char>(c);
            }
        } else {
            r += static_cast<char>(c);
        }
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
 * @brief Substitui pontuacoes UTF-8 (como aspas) por espacos.
 */
static void normalize_utf8_punctuation(std::string& line) {
    std::string bad_chars[] = {"«", "»", "“", "”", "‘", "’", "—", "…", "•", "™"};
    
    for (const auto& bad : bad_chars) {
        size_t pos = 0;
        while ((pos = line.find(bad, pos)) != std::string::npos) {
            line.replace(pos, bad.length(), bad.length(), ' '); 
            pos += bad.length();
        }
    }
}


/**
 * @brief Le o arquivo de texto e retorna vetor de palavras normalizadas.
 *
 * Regras de tokenizacao:
 *  - Espacos e sinais de pontuacao sao separadores.
 *  - Hifen no meio de palavra eh mantido ("mostra-lo").
 *  - Hifen isolado ou no inicio/fim de token eh descartado.
 *  - Todas as letras sao convertidas para minusculas.
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
    bool first_line = true;

    while (std::getline(fin, line)) {
        if (first_line) {
            if (line.size() >= 3 && 
                (unsigned char)line[0] == 0xEF && 
                (unsigned char)line[1] == 0xBB && 
                (unsigned char)line[2] == 0xBF) {
                line.erase(0, 3); // Remove os 3 bytes do BOM
            }
            first_line = false;
        }

        normalize_utf8_punctuation(line);
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
                        sep = true;  
                    }
                }
            }

            if (!sep && !std::isalpha(c) && c < 128 && c != '-')
                sep = true;

            if (!sep) {
                token += static_cast<char>(c);
            } else {
                if (!token.empty()) {
                    std::string clean = strip_edges(token);
                    if (!clean.empty())
                        words.push_back(to_lower(clean));
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
 *        Usa insert_or_increment() para inserir ou incrementar em uma unica busca.
 *        Compativel com qualquer implementacao de Dictionary<string,int>.
 *
 * @param dict   dicionario a ser populado
 * @param words  vetor de palavras ja normalizadas
 */
static void build_frequency(Dictionary<std::string, int>& dict,
                             const std::vector<std::string>& words) {
    for (const auto& w : words) {
        dict.insert_or_increment(w, 1);
    }
}


// Resultado de uma execucao

struct RunResult {
    std::string type;
    std::string name;
    Metrics metrics;
    size_t unique_count = 0;
};


// Exibição de métricas

/**
 * @brief Imprime as métricas coletadas por uma estrutura de dados.
 *
 * @param name         nome da estrutura
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


static void print_comparison_table(const std::vector<RunResult>& results,
                                   size_t word_count)
{
    std::cout << "\n==================== COMPARATIVO ====================\n";
    std::cout << std::left << std::setw(30) << "Estrutura"
              << std::right
              << std::setw(14) << "Comparacoes"
              << std::setw(10) << "Rotac."
              << std::setw(10) << "Colis."
              << std::setw(12) << "Tempo(ms)"
              << "\n";
    std::cout << std::string(76, '-') << "\n";

    for (const auto& r : results) {
        std::cout << std::left << std::setw(30) << r.name
                  << std::right
                  << std::setw(14) << r.metrics.comparisons
                  << std::setw(10) << (r.metrics.rotations > 0 ? std::to_string(r.metrics.rotations) : "-")
                  << std::setw(10) << (r.metrics.collisions > 0 ? std::to_string(r.metrics.collisions) : "-")
                  << std::fixed << std::setprecision(3)
                  << std::setw(12) << r.metrics.elapsed_ms
                  << "\n";
    }

    std::cout << std::string(76, '-') << "\n";
    std::cout << "Tokens processados: " << word_count << "\n";
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
    std::filesystem::path path(out_path);
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }

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


static std::string resolve_csv_path(const std::string& output_dir,
                                    const std::string& csv_arg,
                                    const std::string& default_name) {
    if (csv_arg.empty()) {
        return output_dir + default_name;
    }
    // Caminho completo: usa direto; caso contrario, prefixo output_dir
    if (csv_arg.find('/') != std::string::npos ||
        csv_arg.find('\\') != std::string::npos) {
        return csv_arg;
    }
    return output_dir + csv_arg;
}


static std::string resolve_text_path(const std::string& txt_arg) {
    // Monta o caminho completo: se o usuario ja passou um caminho com
    // separador (/ ou \), usa direto; caso contrario, prefixo books_dir.
    if (txt_arg.find('/') != std::string::npos ||
        txt_arg.find('\\') != std::string::npos) {
        return txt_arg;
    }

    // Pasta onde os livros estao armazenados.
    const std::string books_dir = "include/test_books/";
    std::string txt_file = books_dir + txt_arg;
    // Adiciona .txt automaticamente se o usuario nao colocou
    if (txt_file.size() < 4 ||
        txt_file.substr(txt_file.size() - 4) != ".txt") {
        txt_file += ".txt";
    }
    return txt_file;
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
static RunResult run_structure(const std::string& type,
                               const std::vector<std::string>& words,
                               const std::string& out_csv,
                               bool write_output)
{
    RunResult result;
    result.type = type;
    result.name = friendly_name(type);

    DictPtr dict = make_dict(type);
    if (!dict) {
        std::cerr << "Estrutura desconhecida: " << type << "\n";
        return result;
    }

    // Mede o tempo de construcao da tabela de frequencias
    auto t0 = std::chrono::high_resolution_clock::now();
    build_frequency(*dict, words);
    auto t1 = std::chrono::high_resolution_clock::now();

    result.metrics = dict->metrics();
    result.metrics.elapsed_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    result.unique_count = dict->size();

    print_metrics(result.name, result.metrics, words.size(), result.unique_count);

    if (write_output && !out_csv.empty()) {
        write_csv(*dict, out_csv);
    }

    return result;
}


// Ajuda

static void print_help(const char* prog) {
    std::cout
        << "\nUso:\n"
        << "  " << prog << " dictionary <estrutura> <arquivo.txt> [saida.csv]\n\n"
        << "Estruturas disponiveis:\n"
        << "  avl         Arvore AVL completamente iterativa\n"
        << "  rb          Arvore Rubro-Negra completamente iterativa\n"
        << "  hash-chain  Hash com encadeamento exterior (ChainedHashTable)\n"
        << "  hash-open   Hash com enderecamento aberto  (OpenAddressingHashTable)\n"
        << "  all         Executa as quatro estruturas e exibe comparativo\n\n"
        << "Exemplos:\n"
        << "  " << prog << " dictionary avl        dom-casmurro\n"
        << "  " << prog << " dictionary hash-chain sherlock_holmes\n"
        << "  " << prog << " dictionary all        dom-casmurro\n\n"
        << "Opcoes:\n"
        << "  --help, -h   Exibe esta mensagem de ajuda\n\n"
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
        std::cerr << "\nUso: " << argv[0]
                  << " dictionary <estrutura> <arquivo.txt> [saida.csv]\n"
                  << "\nUse --help para mais informacoes.\n\n";
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

    std::string txt_file = resolve_text_path(txt_arg);

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
        std::vector<RunResult> results;
        results.reserve(all_types.size());

        for (const auto& t : all_types) {
            std::string out = output_dir + t + ".csv";
            results.push_back(run_structure(t, words, out, true));
        }

        print_comparison_table(results, words.size());
    } else {
        // Estrutura unica: usa o nome fornecido pelo usuario,
        // ou <estrutura>.csv como padrão, dentro de sheet_results/<livro>/.
        if (!make_dict(type)) {
            std::cerr << "Estrutura desconhecida: " << type << "\n";
            return 1;
        }
        std::string out = resolve_csv_path(output_dir, csv_file, type + ".csv");
        run_structure(type, words, out, true);
    }

    return 0;
}
