/**
 * @file OpenAddressingHashTable.h
 * @author João Pedro Teófilo (joaopedroteofilo@alu.ufc.br)
 * @brief Uma tabela hash com tratamento de colisao por enderecamento aberto - Estrutura de dados avancada - 2026.1
 * @version 0.2
 * @date 2026-06-04
 * @copyright Copyright (c) 2026
 * */

#ifndef OPEN_ADDRESSING_HASHTABLE_H
#define OPEN_ADDRESSING_HASHTABLE_H

#include <iostream>
#include <vector>
#include <stdexcept>
#include <functional>
#include <algorithm>
#include <utility>

#include "Dictionary.h"

/**
 * @brief Status de cada slot na tabela hash
 */
enum class Status {
    EMPTY,
    ACTIVE,
    DELETED
};

/**
 * @brief Classe que implementa uma tabela hash com tratamento de
 * colisao por enderecamento aberto (open addressing) usando sondagem linear.
 * 
 * @tparam Key key type
 * @tparam Value value type
 * @tparam Hash hash function type
 */
template <typename Key, typename Value, typename Hash = std::hash<Key>>
class OpenAddressingHashTable : public Dictionary<Key, Value> {
private:
    struct HashNode {
        Key key;
        Value value;
        Status status = Status::EMPTY;
    };

    // quantidade de pares (chave,valor) ativos
    size_t m_number_of_elements;

    // quantidade de elementos removidos
    size_t m_deleted_elements;

    // tamanho atual da tabela
    size_t m_table_size;

    // O maior valor que o fator de carga pode ter.
    float m_max_load_factor;

    // tabela
    std::vector<HashNode> m_table;

    // referencia para a funcao de codificacao
    Hash m_hashing;

    // metrica mutavel para permitir incremento em funcoes const
    mutable Metrics m_metrics;


    /**
     * @brief Retorna o menor numero primo maior ou igual a x. 
     * 
     * @param x := um numero nao negativo
     * @return size_t := um numero primo
     */
    size_t get_next_prime(size_t x) {
        if(x <= 2) return 3;
        x = (x % 2 == 0) ? x + 1 : x;
        while(true) {
            bool prime = true;
            for(size_t i = 2; i <= x/i; ++i) {
                if(x % i == 0) { prime = false; break; }
            }
            if(prime) return x;
            x += 2;
        }
    }


    /**
     * @brief Retorna um inteiro no intervalo [0 ... m_table_size-1].
     * 
     * @param k := um valor de chave do tipo Key
     * @return size_t := um inteiro no intervalo [0 ... m_table_size-1]
     */
    size_t hash_code(const Key& k) const {
        return m_hashing(k) % m_table_size;
    }


    /**
     * @brief Calcula a posicao de sondagem linear na tabela.
     * h(k, i) = (h'(k) + i) % m_table_size
     * 
     * @param hash_inicial := valor do hash_code original
     * @param i := tentativa de sondagem
     * @return size_t := indice calculado
     */
    size_t linear_probe(size_t hash_inicial, size_t i) const {
        return (hash_inicial + i) % m_table_size;
    }


    /**
     * @brief Retorna um booleano indicando se a tabela hash precisa ser redimensionada.
     */
    bool needs_rehash() const {
        float current_load = static_cast<float>(m_number_of_elements + m_deleted_elements + 1) / m_table_size;
        return current_load > m_max_load_factor;
    }


    /**
     * @brief Funcao auxiliar de busca em enderecamento aberto.
     * 
     * @param k := chave a ser pesquisada
     * @return long long := indice do elemento se encontrado, -1 caso contrario
     */
    long long aux_hash_search(const Key& k) const {
        size_t i = 0;
        size_t h_inicial = hash_code(k);

        while (i < m_table_size) {
            size_t j = linear_probe(h_inicial, i);

            if (m_table[j].status == Status::EMPTY) {
                return -1; 
            }

            if (m_table[j].status == Status::ACTIVE) {
                m_metrics.comparisons++;
                if (m_table[j].key == k) {
                    return j;
                }
            }
            i++;
        }
        return -1;
    }


    /**
     * @brief Operacao de reconstrucao da tabela hash.
     * 
     * @param new_size := o novo tamanho base
     */
    void rehash(size_t new_size) {
        size_t new_prime_size = get_next_prime(new_size);
        std::vector<HashNode> old_table = m_table;
        
        m_table.assign(new_prime_size, HashNode());
        m_table_size = new_prime_size;
        m_number_of_elements = 0;
        m_deleted_elements = 0;

        for (const auto& node : old_table) {
            if (node.status == Status::ACTIVE) {
                m_metrics.reinsertions++;
                insert(node.key, node.value);
            }
        }
    }


public:
    /**
     * @brief Construtor: cria uma tabela hash com um numero primo de slots.
     * 
     * @param tableSize := o numero de slots da tabela. 
     * @param maxLoadFactor := fator de carga maximo
     */
    OpenAddressingHashTable(size_t tableSize = 19, float maxLoadFactor = 0.5f) {
        if(maxLoadFactor <= 0 || maxLoadFactor >= 1.0f) {
            throw std::invalid_argument("invalid load factor");
        }
        m_max_load_factor = maxLoadFactor;
        m_table_size = get_next_prime(tableSize);
        m_table.assign(m_table_size, HashNode());
        m_number_of_elements = 0;
        m_deleted_elements = 0;
    }


    /**
     * @brief Retorna o numero de elementos na tabela hash
     */
    size_t size() const override { 
        return m_number_of_elements; 
    }


    /**
     * @brief Retorna um booleano indicando se a tabela esta vazia
     */
    bool empty() const override { 
        return m_number_of_elements == 0; 
    }


    /**
     * @brief Todos os pares de (chave,valor) da tabela hash sao deletados.
     */
    void clear() override {
        m_table.assign(m_table_size, HashNode());
        m_number_of_elements = 0;
        m_deleted_elements = 0;
    }


    /**
     * @brief Insere um novo elemento na tabela hash.
     * Caso a insercao seja feita, isso incrementa o 
     * numero de elementos da tabela em 1 unidade.
     * Retorna true se e somente se a insercao for feita. Retorna false se a chave ja existir.
     * 
     * @param k := chave
     * @param v := valor 
     */
    bool insert(const Key& k, const Value& v) override {
        if (needs_rehash()) {
            rehash(2 * m_table_size);
        }

        size_t i = 0;
        size_t h_inicial = hash_code(k);
        long long index_disponivel = -1;

        while (i < m_table_size) {
            size_t j = linear_probe(h_inicial, i);

            if (m_table[j].status == Status::ACTIVE) {
                m_metrics.comparisons++;
                if (m_table[j].key == k) {
                    return false;
                }
                m_metrics.collisions++;
            } 
            else if (m_table[j].status == Status::EMPTY) {
                if (index_disponivel == -1) index_disponivel = j;
                break;
            } 
            else if (m_table[j].status == Status::DELETED) {
                if (index_disponivel == -1) index_disponivel = j;
                m_metrics.collisions++;
            }
            i++;
        }

        if (index_disponivel != -1) {
            if (m_table[index_disponivel].status == Status::DELETED) {
                m_deleted_elements--;
            }
            m_table[index_disponivel].key = k;
            m_table[index_disponivel].value = v;
            m_table[index_disponivel].status = Status::ACTIVE;
            m_number_of_elements++;
            return true;
        }

        return false;
    }


    /**
     * @brief Atualiza o valor de uma chave existente.
     * 
     * @param k := chave
     * @param v := novo valor 
     */
    void update(const Key& k, const Value& v) override {
        long long j = aux_hash_search(k);
        if (j != -1) {
            m_table[j].value = v;
        } else {
            throw std::out_of_range("invalid key");
        }
    }


    /**
     * @brief Recebe como entrada uma chave k e retorna true 
     * se e somente se a chave k estiver presente na tabela hash.
     * 
     * @param k := chave a ser pesquisada
     */
    bool contains(const Key& k) const override {
        return aux_hash_search(k) != -1;
    }


    /**
     * @brief Retorna uma referencia para o valor associado a chave k.
     * Se k nao estiver na tabela, a funcao lanca uma out_of_range exception.
     * 
     * @param k := chave
     * @return Value& := valor associado a chave
     */
    Value& at(const Key& k) override {
        long long j = aux_hash_search(k);
        if (j != -1) return m_table[j].value;
        throw std::out_of_range("invalid key");
    }


    /**
     * @brief Versao const da funcao at()
     * 
     * @param k := chave
     * @return const Value& := valor associado a chave
     */
    const Value& at(const Key& k) const override {
        long long j = aux_hash_search(k);
        if (j != -1) return m_table[j].value;
        throw std::out_of_range("invalid key");
    }


    /**
     * @brief Remove da tabela hash o elemento com chave k se ele existir.
     * Ao remover o elemento, o numero de elementos eh decrementado em 1 unidade.
     * Retorna um booleano indicando se a remocao foi realizada.
     * 
     * @param k := chave a ser removida
     */
    bool remove(const Key& k) override {
        long long j = aux_hash_search(k);
        if (j != -1) {
            m_table[j].status = Status::DELETED;
            m_number_of_elements--;
            m_deleted_elements++;
            return true;
        }
        return false;
    }


    std::vector<std::pair<Key,Value>> to_sorted_vector() const override {
        std::vector<std::pair<Key,Value>> vec;
        for (size_t i = 0; i < m_table_size; ++i) {
            if (m_table[i].status == Status::ACTIVE) {
                vec.push_back({m_table[i].key, m_table[i].value});
            }
        }
        std::sort(vec.begin(), vec.end(), [](const std::pair<Key,Value>& a, const std::pair<Key,Value>& b) {
            return a.first < b.first;
        });
        return vec;
    }


    const Metrics& metrics() const override { 
        return m_metrics; 
    }


    void reset_metrics() override { 
        m_metrics.reset(); 
    }


    void print_csv(std::ostream& os) const override {
        auto sorted = to_sorted_vector();
        for(const auto& par : sorted) {
            os << par.first << "," << par.second << "\n";
        }
    }

    
    /**
     * @brief Classe Iterator para a OpenAddressingHashTable
     * Permite navegar sequencialmente pelos elementos ativos da tabela.
     */
    class Iterator {
    private:
        const OpenAddressingHashTable* m_hash_table;
        size_t m_index;

        void advance_to_valid_element() {
            while (m_index < m_hash_table->m_table_size && 
                   m_hash_table->m_table[m_index].status != Status::ACTIVE) {
                ++m_index;
            }
        }

    public:
        Iterator(const OpenAddressingHashTable* table, size_t start_index) : m_hash_table(table), m_index(start_index) {
            if (m_index < m_hash_table->m_table_size) {
                advance_to_valid_element();
            }
        }

        std::pair<Key, Value> operator*() {
            return {m_hash_table->m_table[m_index].key, m_hash_table->m_table[m_index].value};
        }

        Iterator& operator++() {
            ++m_index;
            advance_to_valid_element();
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return m_index != other.m_index;
        }
    };

    Iterator begin() const {
        return Iterator(this, 0);
    }

    Iterator end() const {
        return Iterator(this, m_table_size);
    }
    // END of Iterator
};

#endif // OPEN_ADDRESSING_HASHTABLE_H