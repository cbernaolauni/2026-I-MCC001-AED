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

        Height m_height = 1;

        AVLNode(const value_type& data, const Height& height,
                AVLNode* left = nullptr, AVLNode* right = nullptr,
                AVLNode* parent = nullptr)
            : BaseNode(data, height, left, right, parent)
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
            return as_avl(this->m_pChild[pos]);
        }

        static Height height(BaseNodePtr p) {
            if (!p) return 0;
            return as_avl(p)->m_height;
        }

        void update_height() {
            m_height = 1 + max(height(this->m_pChild[0]),
                               height(this->m_pChild[1]));
        }

        Height balance_factor() const {
            return height(this->m_pChild[0]) - height(this->m_pChild[1]);
        }

        string to_string() const override {
            stringstream ss;
            ss << "AVLNode(data: " << this->m_data
               << ", height: "    << this->m_height
               << ", h: "          << m_height
               << ", bf: "         << balance_factor() << ")";
            return ss.str();
        }
    };

    using NodePtr    = AVLNode*;

protected:
    // Para castear BaseNodePtr a NodePtr
    static NodePtr as_avl(BaseNodePtr p) {
        return static_cast<NodePtr>(p);
    }

    NodePtr avlRoot() const {
        return as_avl(this->m_pRoot);
    }

    BaseNodePtr make_node(const value_type& value, Ref ref, BaseNodePtr parent) override {
        return new AVLNode(value, ref, nullptr, nullptr, as_avl(parent));
    }

    BaseNodePtr post_insert(BaseNodePtr pNode) override {
        return rebalance(as_avl(pNode));
    }

private:
    static Height height(NodePtr p) {
        return AVLNode::height(p);
    }

    static Height balance_factor(NodePtr p) {
        if (!p) return 0;
        return p->balance_factor();
    }

    static void update_height(NodePtr p) {
        if (p) p->update_height();
    }

    /*  Rotación simple derecha (caso LL):    
          parent           child
         / \              / \
      child   T3    ->  T1  parent 
       / \                  / \
      T1  T2              T2  T3
    */

    /*  Rotación simple izquierda (caso RR):
        parent            child
       / \                / \
      T1   child  -> parent   T3
          / \           / \
         T2  T3        T1  T2
    */

    static NodePtr rotate_node(NodePtr parent, bool rotation_left) {
        NodePtr child = parent->getChild(rotation_left ? 1 : 0);
        if (!child) return parent; // nothing to rotate
        NodePtr subtree = child->getChild(rotation_left ? 0 : 1);

        child->setChild(rotation_left ? 0 : 1, parent);
        parent->setChild(rotation_left ? 1 : 0, subtree);

        if (subtree) subtree->setParent(parent);
        child->setParent(parent->getParent());
        parent->setParent(child);

        update_height(parent);
        update_height(child);

        return child;
    }

    // Balance
    // Recibe la raíz del subárbol recién modificado y devuelve la nueva raíz
    // después de aplicar la rotación necesaria (si la hay).
    static NodePtr rebalance(NodePtr pNode) {
        update_height(pNode);

        Height bf = balance_factor(pNode);

        if (bf < -1 || bf > 1) {
            bool right_heavy = bf < -1;
            Side side = right_heavy ? 1 : 0;
            NodePtr child = pNode->getChild(side);
            if (!child) return pNode; // no child to rotate with

            bool double_rotation = right_heavy ? balance_factor(child) > 0
                                            : balance_factor(child) < 0;
            if (double_rotation) {
                NodePtr new_child = right_heavy ? rotate_node(child, true) : rotate_node(child, false);
                pNode->setChild(side, new_child);
                pNode->getChild(side)->setParent(pNode);
            }
            return rotate_node(pNode, right_heavy);
        }

        return pNode;   // ya balanceado
    }

    void write_node(ostream& os, NodePtr p) const {
        if (!p) { os << "NULL\n"; return; }
        os << *p << "\n";
        write_node(os, p->getChild(0));
        write_node(os, p->getChild(1));
    }

public:
    AVLTree()  = default;

    // ToString con altura y factor de balance
    string ToStringVerbose() const {
        std::shared_lock lock(this->m_mutex);
        stringstream ss;
        auto* self = const_cast<MySelf*>(this);
        for (auto it = self->inorder_begin(); it != self->inorder_end(); ++it)
            ss << as_avl(it.getNode())->to_string() << "\n";
        return ss.str();
    }
    
    friend ostream& operator<<(ostream& os, const MySelf& tree) {
        std::shared_lock lock(tree.m_mutex);
        tree.write_node(os, tree.avlRoot());
        return os;
    }
};


#endif // __AVL_TREE_H__
