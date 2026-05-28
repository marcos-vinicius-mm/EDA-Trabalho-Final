/**
 * @file Dictionary.h
 * @author João Pedro Teófilo (example@alu.ufc.br)
 * @author Marcos V. de Morais Maniçoba (marcosufcvinicius@alu.ufc.br)
 * @brief Interface abstrata para estruturas de dicionario utilizadas no
 * Trabalho Final da disciplina de Estrutura de Dados Avancada - 2026.1.
 * 
 * Esta interface define operacoes fundamentais para estruturas de dados
 * baseadas em pares (chave, valor), incluindo insercao, remocao, busca,
 * atualizacao, metricas de desempenho e iteracao ordenada.
 * 
 * As implementacoes concretas podem utilizar diferentes estrategias,
 * como tabelas hash, arvores AVL, arvores Rubro-Negra, entre outras.
 * 
 * @version 0.1
 * @date 2026-05-28
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <string>
#include <vector>
#include <utility>
#include <functional>

// Estrutura de metricas coletadas por cada dicionario
struct Metrics {
    long long comparisons = 0;   // Comparacoes de chave
    long long rotations   = 0;   // Rotacoes (AVL / Rubro-Negra)
    long long collisions  = 0;   // Colisoes (Hash)
    long long reinsertions= 0;   // Re-insersoes (enderecamento aberto)
    double    elapsed_ms  = 0.0; // Tempo de construcao (ms)

    void reset() { comparisons = rotations = collisions = reinsertions = 0; elapsed_ms = 0.0; }
};

/**
* Classe-base abstrata (interface) Dictionary
* Todas as implementacoes herdam desta classe.
* K = tipo da chave   V = tipo do valor
*/
template <typename Key, typename Value>
class Dictionary {
public:
    virtual ~Dictionary() = default;
    
    /**
     * @brief Retorna o numero de elementos na tabela hash
     */
    virtual size_t size();
    
    /**
     * @brief Retorna um booleano indicando se a tabela esta vazia
     */
    virtual bool empty();
    
    /**
     * @brief Retorna o numero de slots na HashTable (o tamanho da tabela).
     * Um slot eh um campo na tabela hash para o qual 
     * os elementos sao atribuidos com base no valor de hashing de sua chave.
     * O numero de slots influencia diretamente no fator de carga da 
     * tabela hash (e, portanto, a probabilidade de colisao).
     * 
     * @return size_t := o numero de slots
     */
    virtual size_t bucket_count();
    
    /**
     * @brief Retorna o numero de elementos armazenados no slot n da tabela.
     * O valor de n deve ser tal que 0 <= n <= m_table_size - 1; 
     * caso contrario lanca uma std::out_of_range exception.
     * 
     * @param n := numero do slot
     * @return size_t := numero de elementos no slot n
     */
    virtual size_t bucket_size(size_t n);
    
    /**
     * @brief Retorna o numero do slot onde a chave k estaria localizada.
     * 
     * @param k := chave  
     * @return size_t := numero do slot
     */
    virtual size_t bucket(const Key& k);
    
    /**
     * @brief retorna o valor do fator de carga atual 
     * Atencao: cuidado com divisao inteira: voce deve retornar um float.
     */
    virtual float load_factor();
    
    /**
     * @brief retorna o maior valor que o fator de carga pode ter
     */
    virtual float max_load_factor();

    /**
     * @brief Todos os pares de (chave,valor) da tabela hash sao deletados: 
     * A funcao clear() de cada lista encadeada eh chamada e as listas sao esvaziadas, 
     * deixando a tabela hash com zero pares (m_number_of_elements == 0).
     */
    virtual void clear();
    
    /**
     * @brief Insere um novo elemento na tabela hash.
     * Se ((m_number_of_elements + 1) / m_table_size) > m_max_load_factor entao a funcao
     * invoca a funcao rehash() passando o dobro do tamanho atual da tabela.
     * O elemento eh inserido somente se a chave dele ja nao estiver presente
     * na tabela (numa tabela hash, as chaves sao unicas). 
     * Retorna false caso a insercao nao seja feita.
     * Caso a insercao seja feita, isso incrementa o 
     * numero de elementos da tabela em 1 unidade.
     * Retorna true se e somente se a insercao for feita.
     * 
     * @param k := chave
     * @param v := valor 
     */
    virtual bool add(const Key& k, const Value& v);
    
	/**
     * @brief Insere um novo elemento na tabela hash.
     * Se ((m_number_of_elements + 1) / m_table_size) > m_max_load_factor entao a funcao
     * invoca a funcao rehash() passando o dobro do tamanho atual da tabela.
     * O elemento eh inserido somente se a chave dele ja nao estiver presente
     * na tabela (numa tabela hash, as chaves sao unicas). 
     * Caso a insercao seja feita, isso incrementa o 
     * numero de elementos da tabela em 1 unidade.
     * Retorna o valor se a insercao for feita ou se a chave ja existir na tabela.
     * 
     * @param k := chave
     * @param v := valor 
     */
    virtual Value& insert(const Key& k, const Value& v);

	/**
     * @brief Retorna uma referencia para o valor associado a chave k.
     * Se k nao estiver na tabela, a funcao lanca uma out_of_range exception.
     * 
     * @param k := chave
     * @return V& := valor associado a chave
     */
    virtual V& at(const Key& k) = 0;
    
    /**
     * @brief Versao const da funcao at()
     * 
     * @param k 
     * @return const Value& 
     */
    virtual const V& at(const Key& k) const = 0;

    // Atualizacao: equivalente a insert quando chave ja existe
    virtual void update(const Key& k, const Value& v) = 0;

    /**
     * @brief Remove da tabela hash o elemento com chave k se ele existir.
     * Ao remover o elemento, o numero de elementos eh decrementado em 1 unidade.
     * Retorna um booleano indicando se a remocao foi realizada.
     * 
     * @param k := chave a ser removida
     */	
    virtual bool remove(const Key& k) = 0;

    /**
     * @brief Recebe como entrada uma chave k e retorna true 
     * se e somente se a chave k estiver presente na tabela hash.
     * 
     * @param k := chave a ser pesquisada
     */
    virtual bool contains(const Key& k) const = 0;
    
    /**
     * @brief Recebe um inteiro nao negativo m e faz com que o tamanho
     * da tabela seja um numero primo maior ou igual a m.
     * Se m for maior que o tamanho atual da tabela, um rehashing eh realizado.
     * Se m for menor que o tamanho atual da tabela, a funcao nao tem nenhum efeito.
     * Um rehashing eh uma operacao de reconstrucao da tabela:
     * Todos os elementos no container sao rearranjados de acordo 
     * com o seu valor de hashing dentro na nova tabela.
     * Isto pode alterar a ordem de iteracao dos elementos dentro do container.
     * Operacoes de rehashing sao realizadas automaticamente pelo container 
     * sempre que load_factor() ultrapassa o m_max_load_factor.
     * 
     * @param m := o novo tamanho da tabela hash
     */
    virtual void rehash(size_t m);


    // Iteracao in-order (chave, valor) em ordem de chave
    virtual std::vector<std::pair<Key,Value>> to_sorted_vector() const = 0;

    // Metricas acumuladas desde a criacao (ou ultimo reset)
    virtual const Metrics& metrics() const = 0;
    virtual void  reset_metrics()          = 0;

    // Impressao em stream (CSV: chave,frequencia)
    virtual void print_csv(std::ostream& os) const = 0;
};

#endif // DICTIONARY_H