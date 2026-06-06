/**
 * @file RBTree.h
 * @author Marcos V. de Morais Manicoba (marcosufcvinicius@alu.ufc.br)
 * @brief Uma arvore Rubro-Negra com balanceamento completamente iterativo - Estrutura de dados avancada - 2026.1
 * @version 0.1
 * @date 2026-06-05
 * @copyright Copyright (c) 2026
 */

#ifndef RB_TREE_H
#define RB_TREE_H

#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <utility>

#include "Dictionary.h"

/**
 * @brief Classe que implementa um dicionario usando uma Arvore Rubro-Negra.
 *
 * Propriedades da Arvore Rubro-Negra:
 *  1. Todo no e vermelho ou preto.
 *  2. A raiz e preta.
 *  3. Todo no folha (NIL sentinela) e preto.
 *  4. Se um no e vermelho, ambos os seus filhos sao pretos.
 *  5. Para cada no, todos os caminhos simples ate as folhas
 *     descendentes contem o mesmo numero de nos pretos (black-height).
 *
 * @tparam Key   tipo da chave
 * @tparam Value tipo do valor
 */
template <typename Key, typename Value>
class RBTree : public Dictionary<Key, Value> {
private:

    enum class Color { RED, BLACK };

    /**
     * @brief Estrutura interna do No da arvore Rubro-Negra.
     * O no sentinela NIL compartilhado evita verificacoes de nullptr.
     */
    struct Node {
        Key    key;
        Value  value;
        Color  color;
        Node*  left;
        Node*  right;
        Node*  parent;

        Node(const Key& k, const Value& v, Color c, Node* nil)
            : key(k), value(v), color(c), left(nil), right(nil), parent(nil) {}
    };

    Node*   m_root;
    Node*   m_nil;                
    size_t  m_number_of_elements;
    mutable Metrics m_metrics;

    // Rotacoes (O(1))

    /** 
     * @brief Rotacao simples a esquerda em torno de x (Caso RR / parte do RL).
     */
    void left_rotate(Node* x) {
        Node* y  = x->right;
        x->right = y->left;

        if (y->left != m_nil)
            y->left->parent = x;

        y->parent = x->parent;

        if (x->parent == m_nil)
            m_root = y;
        else if (x == x->parent->left)
            x->parent->left  = y;
        else
            x->parent->right = y;

        y->left   = x;
        x->parent = y;
        m_metrics.rotations++;
    }

    /** 
     * @brief simples a direita em torno de y (Caso LL / parte do LR).
     */
    void right_rotate(Node* y) {
        Node* x  = y->left;
        y->left  = x->right;

        if (x->right != m_nil)
            x->right->parent = y;

        x->parent = y->parent;

        if (y->parent == m_nil)
            m_root = x;
        else if (y == y->parent->right)
            y->parent->right = x;
        else
            y->parent->left  = x;

        x->right  = y;
        y->parent = x;
        m_metrics.rotations++;
    }

    
    /**
     * @brief Corrige as violacoes das propriedades RN apos uma insercao.
     * Sobe pela arvore iterativamente verificando violacoes.
     *
     * Tres casos simetricos (esquerda / direita):
     *  Caso 1: tio vermelho         => recolorir tio, pai e avo; subir para o avo.
     *  Caso 2: tio preto, joelho    => rotacionar o pai; cair no Caso 3.
     *  Caso 3: tio preto, linha     => recolorir pai e avo; rotacionar o avo.
     *
     * @param z no recem-inserido (vermelho)
     */
    void insert_fixup(Node* z) {
        while (z->parent->color == Color::RED) {

            // pai eh filho esquerdo
            if (z->parent == z->parent->parent->left) {
                Node* y = z->parent->parent->right;    

                // Caso 1
                if (y->color == Color::RED) {
                    z->parent->color         = Color::BLACK;
                    y->color                 = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    z = z->parent->parent;
                } else {

                    // Caso 2 => vira Caso 3
                    if (z == z->parent->right) {        
                        z = z->parent;
                        left_rotate(z);
                    }

                    // Caso 3
                    z->parent->color         = Color::BLACK;   
                    z->parent->parent->color = Color::RED;
                    right_rotate(z->parent->parent);
                }
            } 
            
            // pai eh filho direito
            else {                              
                Node* y = z->parent->parent->left;

                // Caso 1
                if (y->color == Color::RED) {           
                    z->parent->color         = Color::BLACK;
                    y->color                 = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    z = z->parent->parent;
                } else {

                    // Caso 2 => vira Caso 3
                    if (z == z->parent->left) {         
                        z = z->parent;
                        right_rotate(z);
                    }

                    // Caso 3
                    z->parent->color         = Color::BLACK;   
                    z->parent->parent->color = Color::RED;
                    left_rotate(z->parent->parent);
                }
            }
        }
        m_root->color = Color::BLACK;
    }

    /**
     * @brief a subarvore enraizada em u pela subarvore enraizada em v.
     * Auxiliar da remocao.
     */
    void transplant(Node* u, Node* v) {
        if (u->parent == m_nil)
            m_root           = v;
        else if (u == u->parent->left)
            u->parent->left  = v;
        else
            u->parent->right = v;
        v->parent = u->parent;
    }

    /**
     * @brief Corrige as violacoes das propriedades RB apos uma remocao.
     * Sobe pela arvore iterativamente ate verificando violacoes.
     *
     * Quatro casos simetricos:
     *  Caso 1: irmao vermelho => recolorir e rotacionar; recair em casos 2/3/4.
     *  Caso 2: irmao preto, filhos pretos => recolorir irmao; subir x para o pai.
     *  Caso 3: irmao preto, filho prox vermelho => recolorir e rotacionar; transformar em Caso 4.
     *  Caso 4: irmao preto, filho distante vermelho => recolorir e rotacionar.
     *
     * @param x no que pode estar "duplo preto"
     */
    void delete_fixup(Node* x) {
        while (x != m_root && x->color == Color::BLACK) {
            if (x == x->parent->left) {
                Node* w = x->parent->right;

                // Caso 1
                if (w->color == Color::RED) { 
                    w->color         = Color::BLACK;
                    x->parent->color = Color::RED;
                    left_rotate(x->parent);
                    w = x->parent->right;
                }

                // Caso 2
                if (w->left->color  == Color::BLACK &&
                    w->right->color == Color::BLACK) {    
                    w->color = Color::RED;
                    x = x->parent;
                } else {

                    // Caso 3
                    if (w->right->color == Color::BLACK) { 
                        w->left->color = Color::BLACK;
                        w->color       = Color::RED;
                        right_rotate(w);
                        w = x->parent->right;
                    }

                    // Caso 4
                    w->color         = x->parent->color;  
                    x->parent->color = Color::BLACK;
                    w->right->color  = Color::BLACK;
                    left_rotate(x->parent);
                    x = m_root;
                }
            } else {                                      
                Node* w = x->parent->left;

                // Caso 1
                if (w->color == Color::RED) {             
                    w->color         = Color::BLACK;
                    x->parent->color = Color::RED;
                    right_rotate(x->parent);
                    w = x->parent->left;
                }

                // Caso 2
                if (w->right->color == Color::BLACK &&
                    w->left->color  == Color::BLACK) {    
                    w->color = Color::RED;
                    x = x->parent;
                } else {

                    // Caso 3
                    if (w->left->color == Color::BLACK) { 
                        w->right->color = Color::BLACK;
                        w->color        = Color::RED;
                        left_rotate(w);
                        w = x->parent->left;
                    }

                    // Caso 4
                    w->color         = x->parent->color;  
                    x->parent->color = Color::BLACK;
                    w->left->color   = Color::BLACK;
                    right_rotate(x->parent);
                    x = m_root;
                }
            }
        }
        x->color = Color::BLACK;
    }

    // Busca iterativa — auxiliar interno

    /**
     * @brief Busca iterativa pela chave k.
     * @return ponteiro para o no, ou m_nil se nao encontrado.
     */
    Node* search(const Key& k) const {
        Node* current = m_root;
        while (current != m_nil) {
            m_metrics.comparisons++;
            if (k == current->key)
                return current;
            else if (k < current->key)
                current = current->left;
            else
                current = current->right;
        }
        return m_nil;
    }

    /**
     * @brief Retorna o no com menor chave na subarvore enraizada em node.
     */
    Node* minimum(Node* node) const {
        while (node->left != m_nil)
            node = node->left;
        return node;
    }


public:

    /**
     * @brief Construtor da Arvore Rubro-Negra.
     * Inicializa o sentinela NIL e a raiz.
     */
    RBTree() : m_number_of_elements(0) {
        m_nil         = new Node(Key{}, Value{}, Color::BLACK, nullptr);
        m_nil->left   = m_nil;
        m_nil->right  = m_nil;
        m_nil->parent = m_nil;
        m_root        = m_nil;
    }


    /**
     * @brief Destrutor: limpa todos os nos e deleta o sentinela NIL.
     */
    ~RBTree() override {
        clear();
        delete m_nil;
    }


    /**
     * @brief Retorna o numero de elementos no dicionario.
     */
    size_t size() const override {
        return m_number_of_elements;
    }


    /**
     * @brief Retorna um booleano indicando se o dicionario esta vazio.
     */
    bool empty() const override {
        return m_number_of_elements == 0;
    }


    /**
     * @brief Esvazia a arvore de forma completamente iterativa.
     * Utiliza uma simulacao de pilha (vector) para deletar os nos sem recursao (Pre-Order traversal).
     */
    void clear() override {
        if (m_root == m_nil) return;

        std::vector<Node*> stack;
        stack.push_back(m_root);

        while (!stack.empty()) {
            Node* curr = stack.back();
            stack.pop_back();

            if (curr->left  != m_nil) stack.push_back(curr->left);
            if (curr->right != m_nil) stack.push_back(curr->right);

            delete curr;
        }

        m_root = m_nil;
        m_number_of_elements = 0;
    }


    /**
     * @brief Insere um novo elemento na arvore iterativamente e efetua o rebalanceamento.
     * Retorna true se a insercao foi feita, false se a chave ja existia.
     *
     * @param k chave
     * @param v valor
     */
    bool insert(const Key& k, const Value& v) override {

        // 1. Busca iterativa pelo local de insercao
        Node* y = m_nil;
        Node* current = m_root;
        bool  go_left = false;

        while (current != m_nil) {
            m_metrics.comparisons++;
            y = current;
            if (k == current->key) {
                // Chave ja existe
                return false; 
            } else if (k < current->key) {
                current = current->left;
                go_left = true;
            } else {
                current = current->right;
                go_left = false;
            }
        }

        // 2. Criacao do novo no (vermelho)
        Node* z = new Node(k, v, Color::RED, m_nil);
        z->parent = y;
        m_number_of_elements++;

        if (y == m_nil) {
            // Arvore vazia
            m_root = z;                    
        } else if (go_left) {
            y->left  = z;
        } else {
            y->right = z;
        }

        // 3. Correcao das propriedades RN (iterativo)
        insert_fixup(z);
        return true;
    }


    /**
     * @brief Atualiza o valor de uma chave existente iterativamente.
     * Lanca std::out_of_range se a chave nao existir.
     * 
     * @param k chave
     * @param v novo valor
     */
    void update(const Key& k, const Value& v) override {
        Node* node = search(k);
        if (node == m_nil)
            throw std::out_of_range("invalid key");
        node->value = v;
    }


    /**
     * Retorna true se a chave estiver presente.
     * @param k chave a ser pesquisada
     */
    bool contains(const Key& k) const override {
        return search(k) != m_nil;
    }


    /**
     * @brief Retorna uma referencia para o valor associado a chave k.
     * Lanca std::out_of_range se k nao existir.
     *
     * @param k chave
     * @return Value&
     */
    Value& at(const Key& k) override {
        Node* node = search(k);
        if (node == m_nil)
            throw std::out_of_range("invalid key");
        return node->value;
    }


    /**
     * @brief Versao const da funcao at().
     *
     * @param k chave
     * @return const Value&
     */
    const Value& at(const Key& k) const override {
        Node* node = search(k);
        if (node == m_nil)
            throw std::out_of_range("invalid key");
        return node->value;
    }


    /**
     * @brief Remove iterativamente o elemento com chave k se ele existir.
     * Implementa o algoritmo RB-DELETE de forma completamente iterativa,
     * usando transplant() e delete_fixup().
     *
     * @param k chave a ser removida
     * @return true se removido, false se nao encontrado
     */
    bool remove(const Key& k) override {
        // 1. Localiza o no a remover
        Node* z = search(k);
        if (z == m_nil) return false;

        Node* y = z;
        Node* x = m_nil;
        Color y_orig = y->color;

        if (z->left == m_nil) {
            // Caso A: sem filho esquerdo
            x = z->right;
            transplant(z, z->right);

        } else if (z->right == m_nil) {
            // Caso B: sem filho direito
            x = z->left;
            transplant(z, z->left);

        } else {
            // Caso C: dois filhos — substitui pelo sucessor in-order
            y = minimum(z->right);
            y_orig = y->color;
            x = y->right;

            if (y->parent == z) {
                // x pode ser m_nil; ajusta o pai mesmo assim
                x->parent = y;             
            } else {
                transplant(y, y->right);
                y->right         = z->right;
                y->right->parent = y;
            }
            transplant(z, y);
            y->left         = z->left;
            y->left->parent = y;
            y->color        = z->color;
        }

        delete z;
        m_number_of_elements--;

        // 2. Correcao das propriedades RB (so necessaria se o no removido era preto)
        if (y_orig == Color::BLACK)
            delete_fixup(x);

        return true;
    }


    /**
     * @brief Retorna um vetor ordenado usando travessia In-Order iterativa.
     * Utiliza uma simulacao de pilha para evitar recursao.
     */
    std::vector<std::pair<Key,Value>> to_sorted_vector() const override {
        std::vector<std::pair<Key,Value>> vec;
        std::vector<Node*> stack;
        Node* curr = m_root;

        while (curr != m_nil || !stack.empty()) {
            while (curr != m_nil) {
                stack.push_back(curr);
                curr = curr->left;
            }
            curr = stack.back();
            stack.pop_back();

            vec.push_back({curr->key, curr->value});
            curr = curr->right;
        }
        return vec;
    }


    // Metodos de bucket nao se aplicam a arvores — retornam valores neutros
    size_t bucket_count() const override { return 0; }
    size_t bucket_size(size_t) const override { throw std::out_of_range("RBTree: sem buckets"); }
    size_t bucket(const Key&) const override { return 0; }
    float  load_factor() const override { return 0.0f; }
    float  max_load_factor() const override { return 0.0f; }


    const Metrics& metrics() const override {
        return m_metrics;
    }


    void reset_metrics() override {
        m_metrics.reset();
    }


    void print_csv(std::ostream& os) const override {
        auto sorted = to_sorted_vector();
        for (const auto& par : sorted) {
            os << par.first << "," << par.second << "\n";
        }
    }


    //=============================================================================
    /**
     * @brief Classe Iterator para a RBTree.
     * Permite navegar In-Order pelos elementos sem recursao.
     * Todo iterador e invalidado por operacoes de insert() ou remove().
     */
    class Iterator {
    private:
        RBTree* m_tree;
        std::vector<Node*>  m_stack;

        /**
         * @brief Desce pelo caminho mais a esquerda empilhando os nos.
         */
        void push_left(Node* node) {
            while (node != m_tree->m_nil) {
                m_stack.push_back(node);
                node = node->left;
            }
        }

    public:
        /**
         * @brief Construtor do iterador: posiciona no menor elemento.
         */
        Iterator(RBTree* tree) : m_tree(tree) {
            push_left(m_tree->m_root);
        }

        /**
         * @brief Retorna true se ainda existe proximo elemento.
         */
        bool hasNext() const {
            return !m_stack.empty();
        }

        /**
         * @brief Retorna o par (chave, valor) atual e avanca para o proximo In-Order.
         */
        std::pair<Key, Value> next() {
            if (!hasNext())
                throw std::out_of_range("no more elements");

            Node* current = m_stack.back();
            m_stack.pop_back();

            std::pair<Key, Value> result = {current->key, current->value};

            push_left(current->right);
            return result;
        }

    }; // END of Iterator
    //=============================================================================


    /**
     * @brief Retorna um iterador para o inicio da arvore (menor elemento).
     *
     * @return Iterator
     */
    Iterator iterator() {
        return Iterator(this);
    }
};

#endif // RB_TREE_H