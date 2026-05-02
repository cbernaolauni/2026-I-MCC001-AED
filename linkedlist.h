#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <iostream>
#include <cstddef>   // size_t
#include <string>
#include <sstream>
#include <mutex>     // mutex
#include "general_iterator.h"
#include "util.h"
#include "types.h"
#include "foreach.h"
#include "basetrait.h"

using namespace std;

// Forward iterator
template <typename Container>
class LinkedListForwardIterator : public general_iterator<Container, 
                                            LinkedListForwardIterator<Container>>{
    using MySelf = LinkedListForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
public:
    // TODO: Completar el operator++
    MySelf& operator++(){
        this->m_pNode = this->m_pNode->getNext();
        return *this;
    }
};

// Linked List Node
template <typename T>
class LLNode{
    using value_type = T;
    using Node       = LLNode<T>;
private:
    value_type m_data;
    Ref        m_ref;
    Node      *m_pNext;
public:
    LLNode(){}
    LLNode(value_type data, Ref ref, Node *pNext = nullptr) 
           : m_data(data), m_ref(ref), m_pNext(pNext) {}
    virtual ~LLNode() {}

    value_type      getData() const { return m_data; }
    value_type&     getDataRef()    { return m_data; }
    void            setData(value_type data) { m_data = data; }
    Ref             getRef() const  { return m_ref; }
    Ref&            getRefRef()     { return m_ref; }
    void            setRef(Ref ref) { m_ref = ref; }
    Node*           getNext() const { return m_pNext; }
    Node*&          getNextRef()    { return m_pNext; }
    void            setNext(Node *pNext) { m_pNext = pNext; }
    void operator++() { ++m_data; }
    void operator+=(const value_type& value) { m_data += value; }
    void operator*=(const value_type& value) { m_data *= value; }
};

template <typename T>
ostream &operator<<(ostream &os, const LLNode<T> &node){
    return os << "(" <<node.getData() << ", " << node.getRef() << ")";
}

template <typename T>
struct BaseLinkedListTrait : public BaseContainerTrait<T, LLNode<T>>{

};

template <typename T>
struct AscendingLinkedListTrait : public BaseLinkedListTrait<T>{
    using Comp = less<T>;
};

template <typename T>
struct DescendingLinkedListTrait : public BaseLinkedListTrait<T>{
    using Comp = greater<T>;
};

template <typename Traits>
class LinkedList{
public:
    using value_type = typename Traits::value_type;
    using Node       = typename Traits::Node;
    using Comp       = typename Traits::Comp;
    using MySelf     = LinkedList<Traits>;

    using forward_iterator = LinkedListForwardIterator<MySelf>;
    // friend forward_iterator;

private:
    Node *m_pRoot = nullptr;
    Node *m_pTail = nullptr;
    size_t m_size = 0;
    Comp   m_comp;
    mutex m_mtx;
public:
    LinkedList() {}
    LinkedList(const LinkedList &other){ // Copy constructor
        Node *pNode = other.m_pRoot;
        while (pNode) {
            push_back(pNode->getData(), pNode->getRef());
            pNode = pNode->getNext();
        }
    }
    LinkedList(LinkedList &&other){ // Move constructor
        m_pRoot = other.m_pRoot;
        m_pTail = other.m_pTail;
        m_size  = other.m_size;

        other.m_pRoot = nullptr;
        other.m_pTail = nullptr;
        other.m_size  = 0;
    }
    LinkedList& operator=(const LinkedList &other){ // Copy assignment operator
    }
    LinkedList& operator=(LinkedList &&other){ // Move assignment operator
    }
    
    virtual        ~LinkedList() {
        Node *pNode = m_pRoot;
        while (pNode) {
            Node* pNext = pNode->getNext();
            cout << "Eliminando node: " << *pNode << endl;
            delete pNode;
            pNode = pNext;
        }
        m_pRoot = nullptr;
        m_pTail = nullptr;
        m_size = 0;
    }
    virtual void    push_front(value_type value, Ref ref){
        m_pRoot = new Node(value, ref, m_pRoot);
        if (m_size == 0)
            m_pTail = m_pRoot;
        m_size++;
    }
    virtual auto    pop_front() -> std::pair<value_type, Ref>{ 
        if( m_pRoot ){
            Node* pTemp = m_pRoot;
            m_pRoot = m_pRoot->getNext();
            if (!m_pRoot)
                m_pTail = nullptr;
            m_size--;
            auto result = std::make_pair(pTemp->getData(), pTemp->getRef());
            delete pTemp;
            return result;
        }else
            throw std::out_of_range("pop_front(): empty list");
    }
    virtual void    push_back(value_type value, Ref ref){
        Node* pNew = new Node(value, ref);
        if (m_pTail)
            m_pTail->setNext(pNew);
        m_pTail = pNew;
        if (m_size == 0)
            m_pRoot = m_pTail;
        m_size++;
    }
    virtual auto    pop_back() -> std::pair<value_type, Ref>{
        if(!m_pRoot)
            throw std::out_of_range("pop_back(): empty list");
        
        if (m_pRoot == m_pTail) {
            return pop_front();
        }

        Node* pPrev = m_pRoot;
        while (pPrev->getNext() != m_pTail) {
            pPrev = pPrev->getNext();
        }

        auto result = std::make_pair(m_pTail->getData(), m_pTail->getRef());
        delete m_pTail;

        m_pTail = pPrev;
        m_pTail->setNext(nullptr);
        m_size--;
        return result;
    }

    virtual Node& operator[](size_t index){
        if (index >= m_size)
            throw std::out_of_range("Indice fuera de rango");
        
        Node* pNode = m_pRoot;
        for (size_t i = 0; i < index; i++) {
            pNode = pNode->getNext();
        }
        return *pNode;
    }
private:
            void    internal_insert(Node* &pParent, const value_type &value, Ref ref);
public:
    virtual void    insert(const value_type &value, Ref ref);
    
    // virtual Node& operator[](size_t index);
    virtual size_t  size() const { return m_size; }
    virtual string  toString();

    forward_iterator begin() { return forward_iterator(this, m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr); }

    // Agregar Foreach
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...  args){
        unique_lock<mutex> lock(m_mtx);
        ::ForEach(begin(), end(), func, std::forward<Args>(args)... );
    }

    template <typename Func, typename... Args>
    forward_iterator FirstThat(Func func, Args &&... args){
        return ::FirstThat(begin(), end(), func, forward<Args>(args)...);
    }
};

template <typename Traits>
void LinkedList<Traits>::internal_insert(Node* &pPrev, const value_type &value, Ref ref){
    if(!pPrev || m_comp(value, pPrev->getDataRef())){
        pPrev = new Node(value, ref, pPrev);
        m_size++;
        if(pPrev == m_pRoot)
            m_pTail = pPrev;
        return;
    }
    internal_insert(pPrev->getNextRef(), value, ref);
}

template <typename Traits>
void LinkedList<Traits>::insert(const value_type &value, Ref ref){
    internal_insert(m_pRoot, value, ref);
}

template <typename Traits>
string  LinkedList<Traits>::toString() {
    scoped_lock lock(m_mtx);
    stringstream ss;
    Node *pNode = m_pRoot;
    ss << "[";
    if( m_size > 0 ){
        for( size_t i = 0 ; i < size()-1 ; ++i ){
            ss << *pNode << ",";
            pNode = pNode->getNext();
        }
        ss << *pNode;
    }
    ss << "]";
    return ss.str();
}

template <typename Traits>
ostream& operator<<(ostream& os, LinkedList<Traits>& list){
    return os << list.toString();
}

template <typename Traits>
istream& operator>>(istream& is, LinkedList<Traits>& list){
    typename Traits::value_type value;
    Ref ref;
    is >> value >> ref;
    list.insert(value, ref);
    return is;
}

#endif // __LINKEDLIST_H__