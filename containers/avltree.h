#ifndef __AVL_TREE_H__
#define __AVL_TREE_H__

#include "binarytree.h"

// AVLNode — extiende BinaryTreeNode agregando altura
template <typename T>
class AVLNode : public BinaryTreeNode<T> {
public:
    using Base    = BinaryTreeNode<T>;
    using Node    = AVLNode<T>;
    using NodePtr = Node*;
    Ref m_height = 1;

    AVLNode(const T& data, const Ref& ref,
            NodePtr left   = nullptr,
            NodePtr right  = nullptr,
            NodePtr parent = nullptr)
        : Base(data, ref,
               static_cast<typename Base::NodePtr>(left),
               static_cast<typename Base::NodePtr>(right),
               static_cast<typename Base::NodePtr>(parent))
        , m_height(1)
    {}

    AVLNode(const AVLNode& other)
        : Base(other), m_height(other.m_height)
    {}

    AVLNode(AVLNode&& other) noexcept
        : Base(std::move(other)), m_height(std::exchange(other.m_height, 0))
    {}

    static Ref height(typename Base::NodePtr p) {
        if (!p) return 0;
        return static_cast<AVLNode*>(p)->m_height;
    }

    void update_height() {
        m_height = 1 + max(height(this->m_pChild[0]),
                           height(this->m_pChild[1]));
    }

    Ref balance_factor() const {
        return height(this->m_pChild[0]) - height(this->m_pChild[1]);
    }

    string to_string() const {
        stringstream ss;
        ss << "AVLNode(data: " << this->m_data
           << ", ref: "        << this->m_ref
           << ", h: "          << m_height
           << ", bf: "         << balance_factor() << ")";
        return ss.str();
    }
};


// Traits para AVL
template <typename T>
using AscendingAVLTrait  = AscendingContainerTrait <T, AVLNode>;

template <typename T>
using DescendingAVLTrait = DescendingContainerTrait<T, AVLNode>;


// AVLTree — hereda de BinaryTree y sobreescribe insert con balanceo
template <typename Traits>
class AVLTree : public BinaryTree<Traits> {
public:
    using Base       = BinaryTree<Traits>;
    using value_type = typename Traits::value_type;
    using Node       = typename Traits::Node;       // es AVLNode<T>
    using NodePtr    = Node*;
    using BasePtr    = typename Base::NodePtr;      // BinaryTreeNode<T>*
    using Comp       = typename Traits::Comp;
    using MySelf     = AVLTree<Traits>;

private:
    static Ref height(BasePtr p) {
        return Node::height(p);
    }

    static Ref balance_factor(BasePtr p) {
        if (!p) return 0;
        return static_cast<NodePtr>(p)->balance_factor();
    }

    static void update_height(BasePtr p) {
        if (p) static_cast<NodePtr>(p)->update_height();
    }

    /*  Rotación simple derecha (caso LL):    
          y                x
         / \              / \
        x   T3    ->    T1   y
       / \                  / \
      T1  T2              T2  T3
    */
    static BasePtr rotate_right(BasePtr y) {
        BasePtr x  = static_cast<NodePtr>(y->getChild(0));
        BasePtr T2 = static_cast<NodePtr>(x->getChild(1));

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
    static BasePtr rotate_left(BasePtr x) {
        BasePtr y  = static_cast<NodePtr>(x->getChild(1));
        BasePtr T2 = static_cast<NodePtr>(y->getChild(0));

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
    static BasePtr rebalance(BasePtr pNode) {
        update_height(pNode);

        Ref bf = balance_factor(pNode);

        // Caso LL: subárbol izquierdo pesado, hijo izq también izq-pesado
        if (bf > 1 && balance_factor(static_cast<NodePtr>(pNode->getChild(0))) >= 0)
            return rotate_right(pNode);

        // Caso LR: subárbol izquierdo pesado, hijo izq der-pesado
        if (bf > 1 && balance_factor(static_cast<NodePtr>(pNode->getChild(0))) < 0) {
            pNode->setChild(0, rotate_left(static_cast<NodePtr>(pNode->getChild(0))));
            pNode->getChild(0)->setParent(pNode);
            return rotate_right(pNode);
        }

        // Caso RR: subárbol derecho pesado, hijo der también der-pesado
        if (bf < -1 && balance_factor(static_cast<NodePtr>(pNode->getChild(1))) <= 0)
            return rotate_left(pNode);

        // Caso RL: subárbol derecho pesado, hijo der izq-pesado
        if (bf < -1 && balance_factor(static_cast<NodePtr>(pNode->getChild(1))) > 0) {
            pNode->setChild(1, rotate_right(static_cast<NodePtr>(pNode->getChild(1))));
            pNode->getChild(1)->setParent(pNode);
            return rotate_left(pNode);
        }

        return pNode;   // ya balanceado
    }

    // Insert interno
    // Devuelve la nueva raíz del subárbol (puede cambiar tras una rotación)
    // y llama a rebalance al retornar de la recursión.
    BasePtr avl_insert(BasePtr pNode, const value_type& value, Ref ref,
                       BasePtr parent = nullptr)
    {
        // posición vacía
        if (!pNode) {
            NodePtr n = new Node(value, ref, nullptr, nullptr,
                                 static_cast<NodePtr>(parent));
            return n;
        }

        size_t pos = !this->m_comp(value, pNode->getDataRef());
        BasePtr child = avl_insert(static_cast<NodePtr>(pNode->getChild(pos)), value, ref, pNode);
        pNode->setChild(pos, child);
        child->setParent(pNode);

        // al retornar de la recursión, rebalancear este nodo
        return rebalance(pNode);
    }

    void write_node(ostream& os, BasePtr p) const {
        if (!p) { os << "NULL\n"; return; }
        os << *static_cast<NodePtr>(p) << "\n";   // usa operator<< de BinaryTreeNode
        write_node(os, static_cast<NodePtr>(p->getChild(0)));
        write_node(os, static_cast<NodePtr>(p->getChild(1)));
    }

public:
    AVLTree()  = default;

    // Insert (sobreescribe BinaryTree::insert)
    void insert(const value_type& value, Ref ref) {
        std::unique_lock lock(this->m_mutex);
        this->m_pRoot = avl_insert(this->m_pRoot, value, ref);
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
        tree.write_node(os, tree.m_pRoot);
        return os;
    }
};


#endif // __AVL_TREE_H__
