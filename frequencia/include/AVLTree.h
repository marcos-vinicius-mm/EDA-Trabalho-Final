/**
 * @file AVLTree.h
 * @author João Pedro Teófilo (joaopedroteofilo@alu.ufc.br)
 * @brief Uma arvore AVL com balanceamento completamente iterativo - Estrutura de dados avancada - 2026.1
 * @version 0.1
 * @date 2026-06-04
 * * @copyright Copyright (c) 2026
 */

#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <utility>

#include "Dictionary.h"

/**
 * @brief Classe que implementa um dicionario usando uma Arvore AVL Iterativa.
 * * @tparam Key tipo da chave
 * @tparam Value tipo do valor
 */
template <typename Key, typename Value>
class AVLTree : public Dictionary<Key, Value> {
private:
    /**
     * @brief Estrutura interna do No da arvore AVL.
     * O ponteiro 'parent' eh essencial para permitir a subida na arvore
     * sem o uso de recursao, recalculando alturas e balanceamento.
     */
    struct Node {
        Key key;
        Value value;
        int height;
        Node* left;
        Node* right;
        Node* parent; 

        Node(Key k, Value v, Node* p = nullptr) 
            : key(k), value(v), height(1), left(nullptr), right(nullptr), parent(p) {}
    };

    Node* m_root;
    size_t m_number_of_elements;
    mutable Metrics m_metrics;

    // Metodos auxiliares iterativos de altura e balanceamento

    int get_height(Node* node) const {
        return node ? node->height : 0;
    }

    int get_balance(Node* node) const {
        return node ? get_height(node->left) - get_height(node->right) : 0;
    }

    void update_height(Node* node) {
        if (node) {
            node->height = 1 + std::max(get_height(node->left), get_height(node->right));
        }
    }

    // Metodos iterativos de rotacao

    /**
     * @brief Rotacao simples a direita (Caso LL)
     */
    void right_rotate(Node* y) {
        Node* x = y->left;
        Node* T2 = x->right;

        x->right = y;
        y->left = T2;

        x->parent = y->parent;
        if (y->parent == nullptr) {
            m_root = x;
        } else if (y == y->parent->left) {
            y->parent->left = x;
        } else {
            y->parent->right = x;
        }
        y->parent = x;
        if (T2 != nullptr) {
            T2->parent = y;
        }

        update_height(y);
        update_height(x);
        m_metrics.rotations++;
    }

    /**
     * @brief Rotacao simples a esquerda (Caso RR)
     */
    void left_rotate(Node* x) {
        Node* y = x->right;
        Node* T2 = y->left;

        y->left = x;
        x->right = T2;

        y->parent = x->parent;
        if (x->parent == nullptr) {
            m_root = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }
        x->parent = y;
        if (T2 != nullptr) {
            T2->parent = x;
        }

        update_height(x);
        update_height(y);
        m_metrics.rotations++;
    }

    /**
     * @brief Metodo para balancear a arvore subindo ate a raiz de forma iterativa.
     * Deve ser chamado apos insercoes e remocoes passando o no que foi modificado/inserido.
     */
    void balance_upwards(Node* node) {
        while (node != nullptr) {
            update_height(node);
            int bf = get_balance(node);
            Node* next = node->parent; // Pre-salva o proximo no a visitar na subida

            if (bf > 1) { // Desbalanceado para a Esquerda
                if (get_balance(node->left) < 0) {
                    left_rotate(node->left); // Caso LR
                }
                right_rotate(node); // Caso LL
                // Como o no 'node' desceu na rotacao, seu novo pai eh a nova raiz desta subarvore.
                // O proximo no a ser checado deve ser o pai desta nova raiz.
                next = node->parent->parent; 
            } 
            else if (bf < -1) { // Desbalanceado para a Direita
                if (get_balance(node->right) > 0) {
                    right_rotate(node->right); // Caso RL
                }
                left_rotate(node); // Caso RR
                next = node->parent->parent;
            }

            node = next; // Sobe para o proximo ancestral
        }
    }


public:
    /**
     * @brief Construtor da Arvore AVL
     */
    AVLTree() : m_root(nullptr), m_number_of_elements(0) {}


    /**
     * @brief Destrutor
     */
    ~AVLTree() override {
        clear();
    }


    /**
     * @brief Retorna o numero de elementos no dicionario
     */
    size_t size() const override {
        return m_number_of_elements;
    }


    /**
     * @brief Retorna um booleano indicando se o dicionario esta vazio
     */
    bool empty() const override {
        return m_number_of_elements == 0;
    }


    /**
     * @brief Esvazia a arvore de forma completamente iterativa.
     * Utiliza uma simulacao de pilha (vector) para deletar os nos sem recursao (Pre-Order traversal).
     */
    void clear() override {
        if (!m_root) return;
        std::vector<Node*> stack;
        stack.push_back(m_root);

        while (!stack.empty()) {
            Node* curr = stack.back();
            stack.pop_back();

            if (curr->left) stack.push_back(curr->left);
            if (curr->right) stack.push_back(curr->right);

            delete curr;
        }

        m_root = nullptr;
        m_number_of_elements = 0;
    }


    /**
     * @brief Insere um novo elemento na arvore iterativamente e efetua o rebalanceamento.
     * * @param k := chave
     * @param v := valor
     */
    bool insert(const Key& k, const Value& v) override {
        if (m_root == nullptr) {
            m_root = new Node(k, v);
            m_number_of_elements++;
            return true;
        }

        Node* current = m_root;
        Node* parent = nullptr;

        // 1. Busca Iterativa pelo local de insercao
        while (current != nullptr) {
            m_metrics.comparisons++;
            parent = current;
            if (k == current->key) {
                return false; // Chave ja existe
            } else if (k < current->key) {
                current = current->left;
            } else {
                current = current->right;
            }
        }

        // 2. Insercao do novo No
        Node* new_node = new Node(k, v, parent);
        if (k < parent->key) {
            parent->left = new_node;
        } else {
            parent->right = new_node;
        }
        m_number_of_elements++;

        // 3. Balanceamento bottom-up (Iterativo)
        balance_upwards(parent);

        return true;
    }


    /**
     * @brief Atualiza o valor de uma chave existente iterativamente.
     * * @param k := chave
     * @param v := novo valor 
     */
    void update(const Key& k, const Value& v) override {
        Node* current = m_root;
        while (current != nullptr) {
            m_metrics.comparisons++;
            if (k == current->key) {
                current->value = v;
                return;
            } else if (k < current->key) {
                current = current->left;
            } else {
                current = current->right;
            }
        }
        throw std::out_of_range("invalid key");
    }


    /**
     * @brief Retorna true se a chave estiver presente, usando busca iterativa.
     * * @param k := chave a ser pesquisada
     */
    bool contains(const Key& k) const override {
        Node* current = m_root;
        while (current != nullptr) {
            m_metrics.comparisons++;
            if (k == current->key) {
                return true;
            } else if (k < current->key) {
                current = current->left;
            } else {
                current = current->right;
            }
        }
        return false;
    }


    /**
     * @brief Retorna uma referencia para o valor associado a chave k.
     */
    Value& at(const Key& k) override {
        Node* current = m_root;
        while (current != nullptr) {
            m_metrics.comparisons++;
            if (k == current->key) {
                return current->value;
            } else if (k < current->key) {
                current = current->left;
            } else {
                current = current->right;
            }
        }
        throw std::out_of_range("invalid key");
    }


    /**
     * @brief Versao const da funcao at()
     */
    const Value& at(const Key& k) const override {
        Node* current = m_root;
        while (current != nullptr) {
            m_metrics.comparisons++;
            if (k == current->key) {
                return current->value;
            } else if (k < current->key) {
                current = current->left;
            } else {
                current = current->right;
            }
        }
        throw std::out_of_range("invalid key");
    }


    /**
     * @brief Remove iterativamente o elemento com chave k se ele existir.
     * Lida com os 3 casos de delecao em BST e ajusta as alturas subindo ate a raiz.
     * * @param k := chave a ser removida
     */
    bool remove(const Key& k) override {
        Node* current = m_root;
        
        // 1. Encontra iterativamente o No a ser removido
        while (current != nullptr) {
            m_metrics.comparisons++;
            if (k == current->key) break;
            if (k < current->key) current = current->left;
            else current = current->right;
        }

        if (current == nullptr) return false; // Nao encontrado

        Node* node_to_balance_from = nullptr;

        // 2. Remocao do No
        if (current->left == nullptr || current->right == nullptr) {
            // Caso A: 0 ou 1 filho
            Node* child = (current->left != nullptr) ? current->left : current->right;
            
            if (current->parent == nullptr) {
                m_root = child;
                if (child) child->parent = nullptr;
            } else {
                if (current == current->parent->left) {
                    current->parent->left = child;
                } else {
                    current->parent->right = child;
                }
                if (child) child->parent = current->parent;
                node_to_balance_from = current->parent; // Pai do no removido eh onde iniciamos o balanceamento
            }
            delete current;
        } else {
            // Caso B: 2 filhos. Encontrar o sucessor (menor no da subarvore direita)
            Node* succ = current->right;
            while (succ->left != nullptr) {
                succ = succ->left;
            }
            
            // Troca a chave/valor (iterativamente muito mais simples que refazer ponteiros)
            current->key = succ->key;
            current->value = succ->value;
            
            // O sucessor tem no maximo um filho a direita
            Node* child = succ->right;
            node_to_balance_from = succ->parent;
            
            if (succ->parent->left == succ) {
                succ->parent->left = child;
            } else {
                succ->parent->right = child;
            }
            if (child) child->parent = succ->parent;
            
            delete succ;
        }
        
        m_number_of_elements--;

        // 3. Balanceamento bottom-up (Iterativo)
        balance_upwards(node_to_balance_from);
        
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

        while (curr != nullptr || !stack.empty()) {
            while (curr != nullptr) {
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
};

#endif // AVL_TREE_H