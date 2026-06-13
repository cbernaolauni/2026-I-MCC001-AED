#ifndef __BINARY_TREE_H__
#define __BINARY_TREE_H__
#include <iostream>
#include <cstddef>   // size_t
#include <string>
#include <sstream>
#include <shared_mutex>
#include <mutex>
#include <utility>
#include <type_traits>
#include <functional>
#include "general_iterator.h"
#include "binaryTreeIterator.h"
#include "../types.h"
#include "../foreach.h"

template <typename T>
struct AscendingBinaryTreeTrait {
    using value_type = T;
    using Comp       = less<T>;
};

template <typename T>
struct DescendingBinaryTreeTrait {
    using value_type = T;
    using Comp       = greater<T>;
};

template <typename Traits>
class BinaryTree{
public:
    using value_type = typename Traits::value_type;
    using Comp       = typename Traits::Comp;
    using MySelf     = BinaryTree<Traits>;

    class Node {
    public:
        using NodePtr = Node*;

        value_type m_data;
        Ref        m_ref;
        NodePtr    m_pChild[2] = {nullptr, nullptr};
        NodePtr    m_pParent   = nullptr;

        Node(const value_type& data, const Ref& ref,
             NodePtr left = nullptr, NodePtr right = nullptr,
             NodePtr parent = nullptr)
            : m_data(data), m_ref(ref), m_pParent(parent)
        {
            m_pChild[0] = left;
            m_pChild[1] = right;
        }

        Node(const Node& other)
            : m_data(other.m_data), m_ref(other.m_ref), m_pParent(nullptr)
        {
            m_pChild[0] = other.m_pChild[0] ? new Node(*other.m_pChild[0]) : nullptr;
            m_pChild[1] = other.m_pChild[1] ? new Node(*other.m_pChild[1]) : nullptr;
            if (m_pChild[0]) m_pChild[0]->m_pParent = this;
            if (m_pChild[1]) m_pChild[1]->m_pParent = this;
        }

        Node(Node&& other) noexcept
            : m_data   (std::move(other.m_data))
            , m_ref    (std::move(other.m_ref))
            , m_pParent(std::exchange(other.m_pParent, nullptr))
        {
            m_pChild[0] = std::exchange(other.m_pChild[0], nullptr);
            m_pChild[1] = std::exchange(other.m_pChild[1], nullptr);
            if (m_pChild[0]) m_pChild[0]->m_pParent = this;
            if (m_pChild[1]) m_pChild[1]->m_pParent = this;
        }

        virtual ~Node() {
            delete m_pChild[0];
            delete m_pChild[1];
        }

        value_type  getData()    const         { return m_data; }
        value_type& getDataRef()               { return m_data; }
        void        setData(value_type data)   { m_data = data; }

        Ref   getRef()    const { return m_ref; }
        Ref&  getRefRef()       { return m_ref; }
        void  setRef(Ref ref)   { m_ref = ref;  }

        virtual NodePtr getChild   (size_t pos) const { return m_pChild[pos]; }
        NodePtr&        getChildRef(size_t pos)       { return m_pChild[pos]; }
        void            setChild(size_t pos, NodePtr p) { m_pChild[pos] = p; }

        NodePtr getParent()          const { return m_pParent; }
        void    setParent(NodePtr p)       { m_pParent = p;    }

        virtual string to_string() const {
            stringstream ss;
            ss << "Node(data: " << m_data << ", ref: " << m_ref << ")";
            return ss.str();
        }

        friend ostream& operator<<(ostream& os, const Node& node) {
            os << node.m_data << " " << node.m_ref;
            return os;
        }

        friend istream& operator>>(istream& is, Node& node) {
            is >> node.m_data >> node.m_ref;
            return is;
        }
    };

    using NodePtr    = Node*;

    using forward_inorder_iterator    = BinaryTreeIterator<MySelf, TraversalOrder::Inorder,   TraversalDirection::Forward>;
    using backward_inorder_iterator   = BinaryTreeIterator<MySelf, TraversalOrder::Inorder,   TraversalDirection::Backward>;
    using forward_preorder_iterator   = BinaryTreeIterator<MySelf, TraversalOrder::Preorder,  TraversalDirection::Forward>;
    using backward_preorder_iterator  = BinaryTreeIterator<MySelf, TraversalOrder::Preorder,  TraversalDirection::Backward>;
    using forward_postorder_iterator  = BinaryTreeIterator<MySelf, TraversalOrder::Postorder, TraversalDirection::Forward>;
    using backward_postorder_iterator = BinaryTreeIterator<MySelf, TraversalOrder::Postorder, TraversalDirection::Backward>;

protected:
    NodePtr m_pRoot = nullptr;
    Comp    m_comp;
    mutable shared_mutex m_mutex;
public:
    BinaryTree() {}
    BinaryTree(const BinaryTree &other){ // Copy constructor
        std::shared_lock lock(other.m_mutex);
        m_comp = other.m_comp;
        m_pRoot = clone_subtree(other.m_pRoot);
    };
    BinaryTree(BinaryTree &&other){ // Move constructor
        std::unique_lock lock(other.m_mutex);
        m_comp = other.m_comp;
        m_pRoot = exchange(other.m_pRoot, nullptr);
    };

    void insert(const value_type &value, Ref ref){
        std::unique_lock lock(m_mutex);
        internal_insert(m_pRoot, value, ref);
    }

    ~BinaryTree() {
        delete m_pRoot;
    }

    forward_inorder_iterator    inorder_begin()     { return forward_inorder_iterator(m_pRoot);    }
    forward_inorder_iterator    inorder_end()       { return forward_inorder_iterator(nullptr);    }
    backward_inorder_iterator   rinorder_begin()    { return backward_inorder_iterator(m_pRoot);   }
    backward_inorder_iterator   rinorder_end()      { return backward_inorder_iterator(nullptr);   }

    forward_preorder_iterator   preorder_begin()    { return forward_preorder_iterator(m_pRoot);   }
    forward_preorder_iterator   preorder_end()      { return forward_preorder_iterator(nullptr);   }
    backward_preorder_iterator  rpreorder_begin()   { return backward_preorder_iterator(m_pRoot);  }
    backward_preorder_iterator  rpreorder_end()     { return backward_preorder_iterator(nullptr);  }

    forward_postorder_iterator  postorder_begin()   { return forward_postorder_iterator(m_pRoot);  }
    forward_postorder_iterator  postorder_end()     { return forward_postorder_iterator(nullptr);  }
    backward_postorder_iterator rpostorder_begin()  { return backward_postorder_iterator(m_pRoot); }
    backward_postorder_iterator rpostorder_end()    { return backward_postorder_iterator(nullptr); }

    // Range Proxies - Inorder
    auto ForEach() {
        return SafeIteratorRange(inorder_begin(), inorder_end(), m_mutex);
    }

    auto ReverseForEach() {
        return SafeIteratorRange(rinorder_begin(), rinorder_end(), m_mutex);
    }

    // Range Proxies - Preorder
    auto PreorderForEach() {
        return SafeIteratorRange(preorder_begin(), preorder_end(), m_mutex);
    }

    auto ReversePreorderForEach() {
        return SafeIteratorRange(rpreorder_begin(), rpreorder_end(), m_mutex);
    }

    // Range Proxies - Postorder
    auto PostorderForEach() {
        return SafeIteratorRange(postorder_begin(), postorder_end(), m_mutex);
    }

    auto ReversePostorderForEach() {
        return SafeIteratorRange(rpostorder_begin(), rpostorder_end(), m_mutex);
    }

    friend ostream& operator<<(ostream& os, const MySelf& tree) {
        std::shared_lock lock(tree.m_mutex);
        tree.write_node(os, tree.m_pRoot);
        return os;
    }

    friend istream& operator>>(istream& is, MySelf& tree) {
        std::unique_lock lock(tree.m_mutex);
        delete tree.m_pRoot;
        tree.m_pRoot = nullptr;
        tree.read_node(is, tree.m_pRoot);
        return is;
    }

    string ToString() const {
        std::shared_lock lock(m_mutex);
        stringstream ss;
        auto* self = const_cast<MySelf*>(this);
        for (auto it = self->inorder_begin(); it != self->inorder_end(); ++it)
            ss << it.getNode()-> to_string() << "\n";
        return ss.str();
    }

private:
    void internal_insert(NodePtr &pNode, const value_type &value, Ref ref, NodePtr parent = nullptr){
        if( !pNode ){
            pNode = new Node(value, ref, nullptr, nullptr, parent);
            return;
        }
        size_t pos = !m_comp(value, pNode->getDataRef());
        internal_insert(pNode->getChildRef(pos), value, ref, pNode);
    }

    static NodePtr clone_subtree(const NodePtr src, NodePtr parent = nullptr){
        if( !src ) return nullptr;
        NodePtr copy = new Node(src->getData(), src->getRef());
        copy->setParent(parent);
        copy->setChild(0, clone_subtree(src->getChild(0), copy));
        copy->setChild(1, clone_subtree(src->getChild(1), copy));
        return copy;
    }

    void write_node(ostream& os, const NodePtr p) const {
        if (!p) { os << "NULL\n"; return; }
        os << *p << "\n";
        write_node(os, p->getChild(0));
        write_node(os, p->getChild(1));
    }

    void read_node(istream& is, NodePtr& pNode, NodePtr parent = nullptr) {
        string line;
        if (!getline(is, line)) return;
        if (line == "NULL") { pNode = nullptr; return; }

        NodePtr node = new Node(value_type{}, Ref{});
        node->setParent(parent);
        { stringstream ss(line); ss >> *node; }
        pNode = node;
        read_node(is, node->getChildRef(0), node);
        read_node(is, node->getChildRef(1), node);
    }

};

#endif // __BINARY_TREE_H__