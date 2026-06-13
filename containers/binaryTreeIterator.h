#ifndef __BINARY_TREE_ITERATOR_H__
#define __BINARY_TREE_ITERATOR_H__
#include <stack>

enum class TraversalOrder     { Inorder, Preorder, Postorder };
enum class TraversalDirection { Forward, Backward };

template <typename Container,
          TraversalOrder    Order,
          TraversalDirection Dir>
class BinaryTreeIterator {
public:
    using NodePtr     = typename Container::NodePtr;
    using value_type  = typename Container::value_type;
    using MySelf      = BinaryTreeIterator<Container, Order, Dir>;
 
private:
    // FIRST  = hijo "primero" (izquierda en Forward, derecha en Backward)
    // SECOND = hijo "segundo" (derecha en Forward, izquierda en Backward)
    static constexpr size_t FIRST  = (Dir == TraversalDirection::Forward) ? 0 : 1;
    static constexpr size_t SECOND = (Dir == TraversalDirection::Forward) ? 1 : 0;
 
    std::stack<NodePtr> m_stack;
    NodePtr             m_pCurrent = nullptr;
 
    // Inorder
    void inorder_push_first(NodePtr p) {
        while (p) { m_stack.push(p); p = p->getChild(FIRST); }
    }

    void inorder_advance() {
        if (m_stack.empty()) { m_pCurrent = nullptr; return; }
        m_pCurrent = m_stack.top(); m_stack.pop();
        inorder_push_first(m_pCurrent->getChild(SECOND));
    }
 
    // Preorder
    void preorder_advance() {
        if (m_stack.empty()) { m_pCurrent = nullptr; return; }
        m_pCurrent = m_stack.top(); m_stack.pop();
        // Se apila SECOND primero para que FIRST sea el próximo en salir
        if (m_pCurrent->getChild(SECOND)) m_stack.push(m_pCurrent->getChild(SECOND));
        if (m_pCurrent->getChild(FIRST))  m_stack.push(m_pCurrent->getChild(FIRST));
    }
 
    // Postorder
    std::stack<NodePtr> m_postorder_seq;
 
    void postorder_build(NodePtr root) {
        if (!root) return;
        std::stack<NodePtr> tmp;
        tmp.push(root);
        while (!tmp.empty()) {
            NodePtr p = tmp.top(); tmp.pop();
            m_postorder_seq.push(p);
            if (p->getChild(FIRST))  tmp.push(p->getChild(FIRST));
            if (p->getChild(SECOND)) tmp.push(p->getChild(SECOND));
        }
    }

    void postorder_advance() {
        if (m_postorder_seq.empty()) { m_pCurrent = nullptr; return; }
        m_pCurrent = m_postorder_seq.top();
        m_postorder_seq.pop();
    }
 
    void advance() {
        if      constexpr (Order == TraversalOrder::Inorder)   inorder_advance();
        else if constexpr (Order == TraversalOrder::Preorder)  preorder_advance();
        else                                                   postorder_advance();
    }
 
public:
    explicit BinaryTreeIterator(std::nullptr_t) : m_pCurrent(nullptr) {}
 
    explicit BinaryTreeIterator(NodePtr root) {
        if (!root) return;
        if constexpr (Order == TraversalOrder::Inorder) {
            inorder_push_first(root);
            inorder_advance();
        } else if constexpr (Order == TraversalOrder::Preorder) {
            m_stack.push(root);
            preorder_advance();
        } else {
            postorder_build(root);
            postorder_advance();
        }
    }
 
    value_type& operator*()  const { return m_pCurrent->getDataRef(); }
    value_type* operator->() const { return &m_pCurrent->getDataRef(); }
    NodePtr     getNode()    const { return m_pCurrent; }
 
    MySelf& operator++()    { advance(); return *this; }
    
    bool operator==(const MySelf& o) const { return m_pCurrent == o.m_pCurrent; }
    bool operator!=(const MySelf& o) const { return !(*this == o); }
};


#endif // __BINARY_TREE_ITERATOR_H__