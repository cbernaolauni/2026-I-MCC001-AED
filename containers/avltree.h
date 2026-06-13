#ifndef __AVL_TREE_H__
#define __AVL_TREE_H__

#include "binarytree.h"

template <typename T>
struct AscendingAVLTrait {
    using value_type = T;
    using Comp       = less<T>;
};

template <typename T>
struct DescendingAVLTrait {
    using value_type = T;
    using Comp       = greater<T>;
};

// AVLTree — hereda de BinaryTree y sobreescribe insert con balanceo
template <typename Traits>
class AVLTree : public BinaryTree<Traits> {
public:
    using Base       = BinaryTree<Traits>;
    using value_type = typename Traits::value_type;
    using Comp       = typename Traits::Comp;
    using BaseNode   = typename Base::Node;
    using BaseNodePtr= typename Base::NodePtr;
    using MySelf     = AVLTree<Traits>;

    class AVLNode : public BaseNode {
    public:
        using NodePtr = AVLNode*;

        size_t m_height = 1;

        AVLNode(const value_type& data, const Ref& ref,
                AVLNode* left = nullptr, AVLNode* right = nullptr,
                AVLNode* parent = nullptr)
            : BaseNode(data, ref,
                       static_cast<BaseNodePtr>(left),
                       static_cast<BaseNodePtr>(right),
                       static_cast<BaseNodePtr>(parent))
            , m_height(1)
        {}

        AVLNode(const AVLNode& other)
            : BaseNode(other), m_height(other.m_height)
        {}

        AVLNode(AVLNode&& other) noexcept
            : BaseNode(std::move(other))
            , m_height(std::exchange(other.m_height, 0))
        {}

        // getChild() override — devuelve AVLNode* directamente
        AVLNode* getChild(size_t pos) const override {
            return static_cast<AVLNode*>(this->m_pChild[pos]);
        }

        static size_t height(BaseNodePtr p) {
            if (!p) return 0;
            return static_cast<AVLNode*>(p)->m_height;
        }

        void update_height() {
            m_height = 1 + max(height(this->m_pChild[0]),
                               height(this->m_pChild[1]));
        }

        Ref balance_factor() const {
            return static_cast<Ref>(height(this->m_pChild[0]))
                 - static_cast<Ref>(height(this->m_pChild[1]));
        }

        string to_string() const override {
            stringstream ss;
            ss << "AVLNode(data: " << this->m_data
               << ", ref: "        << this->m_ref
               << ", h: "          << m_height
               << ", bf: "         << balance_factor() << ")";
            return ss.str();
        }
    };

    using NodePtr    = AVLNode*;

private:
    AVLNode* avlRoot() const {
        return static_cast<AVLNode*>(this->m_pRoot);
    }

    static Ref height(NodePtr p) {
        return AVLNode::height(p);
    }

    static Ref balance_factor(NodePtr p) {
        if (!p) return 0;
        return p->balance_factor();
    }

    static void update_height(NodePtr p) {
        if (p) p->update_height();
    }

    /*  Rotación simple derecha (caso LL):    
          y                x
         / \              / \
        x   T3    ->    T1   y
       / \                  / \
      T1  T2              T2  T3
    */
    static NodePtr rotate_right(NodePtr y) {
        NodePtr x  = y->getChild(0);
        NodePtr T2 = x->getChild(1);

        // rotación
        x->setChild(1, y);
        y->setChild(0, T2);

        // actualizar padres
        if (T2) T2->setParent(y);
        x->setParent(y->getParent());
        y->setParent(x);

        // actualizar alturas — primero y (ahora hijo), luego x (nueva raíz)
        update_height(y);
        update_height(x);

        return x; // nueva raíz del subárbol
    }

    /*  Rotación simple izquierda (caso RR):
        x                  y
       / \                / \
      T1   y     ->      x   T3
          / \           / \
         T2  T3        T1  T2
    */
    static NodePtr rotate_left(NodePtr x) {
        NodePtr y  = x->getChild(1);
        NodePtr T2 = y->getChild(0);

        // rotación
        y->setChild(0, x);
        x->setChild(1, T2);

        // actualizar padres
        if (T2) T2->setParent(x);
        y->setParent(x->getParent());
        x->setParent(y);

        // actualizar alturas
        update_height(x);
        update_height(y);

        return y; // nueva raíz del subárbol
    }

    // Balance
    // Recibe la raíz del subárbol recién modificado y devuelve la nueva raíz
    // después de aplicar la rotación necesaria (si la hay).
    static NodePtr rebalance(NodePtr pNode) {
        update_height(pNode);

        Ref bf = balance_factor(pNode);

        // Caso LL: subárbol izquierdo pesado, hijo izq también izq-pesado
        if (bf > 1 && balance_factor(pNode->getChild(0)) >= 0)
            return rotate_right(pNode);

        // Caso LR: subárbol izquierdo pesado, hijo izq der-pesado
        if (bf > 1 && balance_factor(pNode->getChild(0)) < 0) {
            pNode->setChild(0, rotate_left(pNode->getChild(0)));
            pNode->getChild(0)->setParent(pNode);
            return rotate_right(pNode);
        }

        // Caso RR: subárbol derecho pesado, hijo der también der-pesado
        if (bf < -1 && balance_factor(pNode->getChild(1)) <= 0)
            return rotate_left(pNode);

        // Caso RL: subárbol derecho pesado, hijo der izq-pesado
        if (bf < -1 && balance_factor(pNode->getChild(1)) > 0) {
            pNode->setChild(1, rotate_right(pNode->getChild(1)));
            pNode->getChild(1)->setParent(pNode);
            return rotate_left(pNode);
        }

        return pNode;   // ya balanceado
    }

    // Insert interno
    // Devuelve la nueva raíz del subárbol (puede cambiar tras una rotación)
    // y llama a rebalance al retornar de la recursión.
    NodePtr avl_insert(NodePtr pNode, const value_type& value, Ref ref,
                       NodePtr parent = nullptr)
    {
        // posición vacía
        if (!pNode) {
            return new AVLNode(value, ref, nullptr, nullptr, parent);
        }

        size_t pos = !this->m_comp(value, pNode->getDataRef());
        NodePtr child = avl_insert(pNode->getChild(pos), value, ref, pNode);
        pNode->setChild(pos, child);
        child->setParent(pNode);

        // al retornar de la recursión, rebalancear este nodo
        return rebalance(pNode);
    }

    void write_node(ostream& os, NodePtr p) const {
        if (!p) { os << "NULL\n"; return; }
        os << *p << "\n";
        write_node(os, p->getChild(0));
        write_node(os, p->getChild(1));
    }

public:
    AVLTree()  = default;

    void insert(const value_type& value, Ref ref) {
        std::unique_lock lock(this->m_mutex);
        this->m_pRoot = avl_insert(avlRoot(), value, ref);
        if (this->m_pRoot) this->m_pRoot->setParent(nullptr);
    }

    // ToString con altura y factor de balance
    string ToStringVerbose() const {
        std::shared_lock lock(this->m_mutex);
        stringstream ss;
        auto* self = const_cast<MySelf*>(this);
        for (auto it = self->inorder_begin(); it != self->inorder_end(); ++it)
            ss << static_cast<NodePtr>(it.getNode())->to_string() << "\n";
        return ss.str();
    }
    
    friend ostream& operator<<(ostream& os, const MySelf& tree) {
        std::shared_lock lock(tree.m_mutex);
        tree.write_node(os, tree.avlRoot());
        return os;
    }
};


#endif // __AVL_TREE_H__
