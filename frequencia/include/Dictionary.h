/**
 * @file Dictionary.h
 * @author João Pedro Teófilo (joaopedroteofilo@alu.ufc.br)
 * @author Marcos V. de Morais Maniçoba (marcosufcvinicius@alu.ufc.br)
 * @brief Interface abstrata para estruturas de dicionario utilizadas no
 * Trabalho Final da disciplina de Estrutura de Dados Avancada - 2026.1.
 * * Esta interface define operacoes fundamentais para estruturas de dados
 * baseadas em pares (chave, valor), incluindo insercao, remocao, busca,
 * atualizacao, metricas de desempenho e iteracao ordenada.
 * * As implementacoes concretas podem utilizar diferentes estrategias,
 * como tabelas hash, arvores AVL, arvores Rubro-Negra, entre outras.
 * 
 * @version 0.1
 * @date 2026-05-28
 * @copyright Copyright (c) 2026
 * */

#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <string>
#include <vector>
#include <utility>
#include <functional>
#include <ostream>

// Estrutura de metricas coletadas por cada dicionario
struct Metrics {
    long long comparisons = 0;   // Comparacoes de chave
    long long rotations   = 0;   // Rotacoes (AVL / Rubro-Negra)
    long long collisions  = 0;   // Colisoes (Hash)
    long long reinsertions= 0;   // Re-insersoes (enderecamento aberto)
    double elapsed_ms = 0.0;     // Tempo gasto em milissegundos

    void reset() { 
        comparisons = rotations = collisions = reinsertions = 0; 
        elapsed_ms = 0.0;
    }
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
     * @brief Retorna o numero de elementos no dicionario
     */
    virtual size_t size() const = 0;
    

    /**
     * @brief Retorna um booleano indicando se o dicionario esta vazio
     */
    virtual bool empty() const = 0;
    

    /**
     * @brief Todos os pares de (chave,valor) do dicionario sao deletados
     */
    virtual void clear() = 0;
    
    /**
     * @brief Insere um novo elemento no dicionario.
     * O elemento eh inserido somente se a chave dele ja nao estiver presente
     * (chaves sao unicas). 
     * Caso a insercao seja feita, isso incrementa o 
     * numero de elementos da tabela em 1 unidade.
     * Retorna true se insercao for feita. Retorna false se a chave ja existir.
     * 
     * @param k := chave
     * @param v := valor 
     */
    virtual bool insert(const Key& k, const Value& v) = 0;


    /**
     * @brief Retorna uma referencia para o valor associado a chave k.
     * Se k nao estiver na tabela, a funcao lanca uma out_of_range exception.
     * 
     * @param k := chave
     * @return Value& := valor associado a chave
     */
    virtual Value& at(const Key& k) = 0;
    
    
    /**
     * @brief Versao const da funcao at()
     * 
     * @param k := chave
     * @return const Value& := valor associado a chave
     */
    virtual const Value& at(const Key& k) const = 0;


    /**
     * @brief Atualiza o valor associado a chave k para v.
     * 
     * @param k := chave
     * @param v := novo valor
     */
    virtual void update(const Key& k, const Value& v) = 0;


    /**
     * @brief Remove o elemento com chave k se ele existir.
     * Ao remover o elemento, o numero de elementos eh decrementado em 1 unidade.
     * Retorna um booleano indicando se a remocao foi realizada.
     * 
     * @param k := chave a ser removida
     */ 
    virtual bool remove(const Key& k) = 0;


    /**
     * @brief Recebe como entrada uma chave k e retorna true 
     * se e somente se a chave k estiver presente no dicionario.
     * 
     * @param k := chave a ser pesquisada
     */
    virtual bool contains(const Key& k) const = 0;
    
    /**
     * @brief Retorna um vetor de pares (chave, valor) contendo todos os elementos do dicionario
     */
    virtual std::vector<std::pair<Key,Value>> to_sorted_vector() const = 0;

    // Metricas acumuladas desde a criacao (ou ultimo reset)
    virtual const Metrics& metrics() const = 0;
    virtual void reset_metrics() = 0;

    // Impressao em stream (CSV: chave,frequencia)
    virtual void print_csv(std::ostream& os) const = 0;
};

#endif // DICTIONARY_H