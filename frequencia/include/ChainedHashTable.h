/**
 * @file ChainedHashTable.h
 * @author Atilio G. Luiz (gomes.atilio@ufc.br)
 * @author Marcos V. de Morais Manicoba (marcosufcvinicius@alu.ufc.br)
 * @brief Uma tabela hash com tratamento de colisao por encadeamento exterior - Estrutura de dados avancada - 2026.1
 * @version 0.2
 * @date 2026-05-28
 * @copyright Copyright (c) 2026
 * */
#ifndef CHAINED_HASHTABLE_H
#define CHAINED_HASHTABLE_H

#include <iostream>
#include <cmath>
#include <string>
#include <list>
#include <vector>
#include <utility>
#include <stdexcept>
#include <functional>
#include <algorithm>

#include "Dictionary.h"

/**
 * @brief Classe que implementa uma tabela hash com tratamento de
 * colisao por encadeamento exterior (chained hash table).
 * 
 * @tparam Key key type
 * @tparam Value value type
 * @tparam Hash hash function type
 */
template <typename Key, typename Value, typename Hash = std::hash<Key>>
class ChainedHashTable : public Dictionary<Key, Value> {
private:
    // quantidade de pares (chave,valor)
    size_t m_number_of_elements;

    // tamanho atual da tabela
    size_t m_table_size;

    // O maior valor que o fator de carga pode ter. 
    // Seja load_factor = m_number_of_elements/m_table_size.
    // Temos que load_factor <= m_max_load_factor.
    // Quando load_factor ultrapassa o valor de m_max_load_factor, 
    // eh preciso executar a operacao de rehashing.                   
    float m_max_load_factor;

    // tabela                               
    std::vector<std::list<std::pair<Key,Value>>> m_table;

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
        if(x <= 2) {
            return 3;
        }
        x = (x % 2 == 0) ? x + 1 : x;
        
        while(true) {
            bool prime = true;
            for(size_t i = 2; i <= x/i; ++i) {
                if(x % i == 0) {
                    prime = false;
                    break;
                }
            }
            if(prime) {
                return x;
            }
            x += 2;
        }
    }

    /**
     * @brief Retorna um inteiro no intervalo [0 ... m_table_size-1].
     * Esta funcao recebe uma chave k e faz o seguinte:
     * (1) computa o codigo hash h(k) usando a 
     * funcao no atributo privado m_hashing
     * (2) computa um indice no intervalo [0 ... m_table_size-1] 
     * aplicando o metodo da divisao: h(k) % m_table_size
     * 
     * @param k := um valor de chave do tipo Key
     * @return size_t := um inteiro no intervalo [0 ... m_table_size-1]
     */
    size_t hash_code(const Key& k) const {
        return m_hashing(k) % m_table_size;
    }


    /**
     * @brief Retorna um booleano indicando se a tabela hash precisa ser redimensionada.
      * A tabela hash precisa ser redimensionada se o valor do fator de carga atual 
      * for maior que o valor do fator de carga maximo permitido (m_max_load_factor).
     */
    bool needs_rehash() {
        if(static_cast<float>(m_number_of_elements + 1) / m_table_size > m_max_load_factor) {
            return true;
        }
        return false;
    }


public:
    /**
     * @brief Construtor: cria uma tabela hash com um numero primo de slots.
     * 
     * @param tableSize := o numero de slots da tabela. 
     * @param maxLoadFactor := fator de carga maximo
     */
    ChainedHashTable(size_t tableSize = 19, float maxLoadFactor = 0.8f) {
        if(maxLoadFactor <= 0) {
            throw std::invalid_argument("invalid load factor");
        }
        m_max_load_factor = maxLoadFactor;
        m_table_size = get_next_prime(tableSize);
        m_table.resize(m_table_size);
        m_number_of_elements = 0;
    }


    /**
     * @brief Retorna o numero de elementos na tabela hash
     */
    size_t size() const {
        return m_number_of_elements;
    }


    /**
     * @brief Retorna um booleano indicando se a tabela esta vazia
     */
    bool empty() const {
        return m_number_of_elements == 0;
    }


    /**
     * @brief Retorna o numero de slots na HashTable (o tamanho da tabela).
     * Um slot eh um campo na tabela hash para o qual 
     * os elementos sao atribuidos com base no valor de hashing de sua chave.
     * O numero de slots influencia diretamente no fator de carga da 
     * tabela hash (e, portanto, a probabilidade de colisao).
     * 
     * @return size_t := o numero de slots
     */
    size_t bucket_count() const {
        return m_table_size;
    }


    /**
     * @brief Retorna o numero de elementos armazenados no slot n da tabela.
     * O valor de n deve ser tal que 0 <= n <= m_table_size - 1; 
     * caso contrario lanca uma std::out_of_range exception.
     * 
     * @param n := numero do slot
     * @return size_t := numero de elementos no slot n
     */
    size_t bucket_size(size_t n) const {
        if(n >= m_table_size) {
            throw std::out_of_range("invalid index");
        }
        return m_table[n].size();
    }


    /**
     * @brief Retorna o numero do slot onde a chave k estaria localizada.
     * 
     * @param k := chave  
     * @return size_t := numero do slot
     */
    size_t bucket(const Key& k) const {
        return hash_code(k);
    }


    /**
     * @brief retorna o valor do fator de carga atual 
     * Atencao: cuidado com divisao inteira: voce deve retornar um float.
     */
    float load_factor() const {
        return static_cast<float>(m_number_of_elements) / m_table_size;
    }


    /**
     * @brief retorna o maior valor que o fator de carga pode ter
     */
    float max_load_factor() const {
        return m_max_load_factor;
    }


    /**
     * @brief Todos os pares de (chave,valor) da tabela hash sao deletados: 
     * A funcao clear() de cada lista encadeada eh chamada e as listas sao esvaziadas, 
     * deixando a tabela hash com zero pares (m_number_of_elements == 0).
     */
    void clear() override {
        for(size_t i = 0; i < m_table_size; ++i) {
            m_table[i].clear();
        }
        m_number_of_elements = 0;
    }


    /**
     * @brief Destroy the Hash Table object
     */
    ~ChainedHashTable() = default;


    /**
     * @brief Insere um novo elemento na tabela hash.
     * Se ((m_number_of_elements + 1) / m_table_size) > m_max_load_factor entao a funcao
     * invoca a funcao rehash() passando o dobro do tamanho atual da tabela.
     * O elemento eh inserido somente se a chave dele ja nao estiver presente
     * na tabela (numa tabela hash, as chaves sao unicas). 
     * Caso a insercao seja feita, isso incrementa o 
     * numero de elementos da tabela em 1 unidade.
     * Retorna true se e somente se a insercao for feita. Retorna false se a chave ja existir.
     * 
     * @param k := chave
     * @param v := valor 
     */
    bool insert(const Key& k, const Value& v) override {
        size_t slot = hash_code(k);
        for (auto& par : m_table[slot]) {
            m_metrics.comparisons++;
            if (par.first == k) return false;   // ja existe
        }
        if (needs_rehash()) {
            rehash(2 * m_table_size);
            slot = hash_code(k);
        }
        if (!m_table[slot].empty()) m_metrics.collisions++;
        m_table[slot].push_back({k, v});
        m_number_of_elements++;
        return true;
    }


    /**
     * @brief Recebe como entrada uma chave k e retorna true 
     * se e somente se a chave k estiver presente na tabela hash.
     * 
     * @param k := chave a ser pesquisada
     */
    bool contains(const Key& k) const override {
        size_t slot = hash_code(k);
        for(auto& par : m_table[slot]) {
            m_metrics.comparisons++;
            if(par.first == k) {
                return true;
            }
        }
        return false;
    }


    /**
     * @brief Retorna uma referencia para o valor associado a chave k.
     * Se k nao estiver na tabela, a funcao lanca uma out_of_range exception.
     * 
     * @param k := chave
     * @return Value& := valor associado a chave
     */
    Value& at(const Key& k) override {
        size_t slot = hash_code(k);
        for(auto& par : m_table[slot]) {
            m_metrics.comparisons++;
            if(par.first == k) {
                return par.second;
            }
        }
        throw std::out_of_range("invalid key");
    }


    /**
     * @brief Versao const da funcao at()
     * 
     * @param k 
     * @return const Value& 
     */
    const Value& at(const Key& k) const override { 
        size_t slot = hash_code(k);
        for(const auto& par : m_table[slot]) {
            m_metrics.comparisons++;
            if(par.first == k) {
                return par.second;
            }
        }
        throw std::out_of_range("invalid key");
    }


    void update(const Key& k, const Value& v) override {
        size_t slot = hash_code(k);
        for(auto& par : m_table[slot]) {
            m_metrics.comparisons++;
            if(par.first == k) {
                par.second = v;
                return;
            }
        }
        throw std::out_of_range("invalid key");
    }


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
    void rehash(size_t m) {
        if(m > m_table_size) {
            
            size_t oldSize = m_table_size;

            size_t newTableSize = get_next_prime(m);
            m_table_size = newTableSize;

            // cria a nova tabela como tamanho maior
            std::vector<std::list<std::pair<Key,Value>>> temp_table(newTableSize);
            
            // transfere pares da tabela antiga para a nova
            for(size_t i = 0; i < oldSize; ++i) {
                for(auto& par : m_table[i]) {
                    size_t slot = hash_code(par.first);
                    temp_table[slot].push_back({par.first, par.second});
                }
            }
            // faz o swap das tabelas
            m_table.swap(temp_table);
        }
    }


    /**
     * @brief Remove da tabela hash o elemento com chave k se ele existir.
     * Ao remover o elemento, o numero de elementos eh decrementado em 1 unidade.
     * Retorna um booleano indicando se a remocao foi realizada.
     * 
     * @param k := chave a ser removida
     */
    bool remove(const Key& k) override {
        size_t slot = hash_code(k);
        for(auto i = m_table[slot].begin(); i != m_table[slot].end(); ++i) {
            m_metrics.comparisons++;
            if(i->first == k) {
                m_table[slot].erase(i);
                m_number_of_elements--;
                return true;
            }
        }
        return false;
    }


    /**
     * @brief Redimensiona a tabela hash de modo que ela tenha 
     * o tamanho mais apropriado a fim de conter pelo menos n elementos.
     * Se n > m_table_size * m_max_load_factor, entao a operacao 
     * de rehash() eh executada sendo passado para ela o tamanho apropriado da nova tabela, 
     * que eh std::ceil(n/m_max_load_factor).
     * Se n <= m_table_size * m_max_load_factor, entao a funcao nao tem efeito, nao faz nada.   
     * 
     * @param n := numero de elementos 
     */
    void reserve(size_t n){
        if(n > (m_table_size * m_max_load_factor)){
            size_t new_load = std::ceil(n/m_max_load_factor);
            rehash(new_load);
        }
    }


    /**
     * @brief Recebe um float lf e muda o valor 
     * do atributo m_max_load_factor para esse valor.
     * Uma restricao eh que: 0 < m_max_load_factor.
     * Se lf <= 0 entao uma 
     * out_of_range exception eh lancada
     * Ao mudar o fator de carga, eh possivel que a tabela hash tenha 
     * que ser redimensionada. Para isso, invocamos 
     * a funcao reserve(m_number_of_elements).
     * 
     * @param lf := novo fator de carga
     */
    void set_max_load_factor(float lf){
        if(lf <= 0) throw std::out_of_range("invalid load factor");
        
        m_max_load_factor = lf;
        reserve(m_number_of_elements);
    }

    std::vector<std::pair<Key,Value>> to_sorted_vector() const override {
        std::vector<std::pair<Key,Value>> vec;
        for(size_t i = 0; i < m_table_size; ++i) {
            for(const auto& par : m_table[i]) {
                vec.push_back(par);
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
            os << par.first << " , " << par.second << "\n";
        }
    }

    
    //=============================================================================
    /**
     * @brief Classe Iterator para a ChainedHashTable
     * Um iterador eh um objeto que nos permite navegar pelos elementos da estrutura de dados.
     * Todo iterador eh invalidado por operacoes de rehash().
     */
    class Iterator {
    private: 
        ChainedHashTable* m_hash_table;
        size_t m_bucket;
        typename std::list<std::pair<Key,Value>>::iterator m_list_it;

        /**
         * @brief Avanca ate o proximo bucket nao vazio
         */
        void advance_to_valid_bucket() {
            while(m_bucket < m_hash_table->m_table_size && m_list_it == m_hash_table->m_table[m_bucket].end()) {
                ++m_bucket;
                if(m_bucket < m_hash_table->m_table_size) {
                    m_list_it = m_hash_table->m_table[m_bucket].begin();
                }
            }
        }
    public:
        /**
         * @brief Construtor do iterador
         */
        Iterator(ChainedHashTable* table)  {
            m_hash_table = table;
            m_bucket = 0;
            if(m_hash_table->m_table_size > 0) {
                m_list_it = m_hash_table->m_table[0].begin();
            }
            advance_to_valid_bucket();
        }

        /**
         * @brief Retorna true se ainda existe proximo elemento
         */
        bool hasNext() const {
            return m_bucket < m_hash_table->m_table_size;
        }

        const std::pair<Key,Value>& next() {
            if(!hasNext()) {
                throw std::out_of_range("no more elements");
            }
            auto& current = *m_list_it;
            ++m_list_it;
            advance_to_valid_bucket();
            return current;
        }

    }; 
    // END of Iterator
    //=============================================================================


    /**
     * @brief Metodo da classe ChainedHashTable que retorna um iterador para o inicio da tabela
     * 
     * @return Iterator 
     */
    Iterator iterator() {
        return Iterator(this);
    }

};

#endif // END of CHAINED_HASHTABLE_H