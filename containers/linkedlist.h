#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <iostream>
#include <cstddef>   // size_t
#include <string>
#include <sstream>
#include <mutex>     // mutex
#include "general_iterator.h"
#include "../types.h"
#include "../foreach.h"
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
public:
    using value_type = T;
    using Node       = LLNode<T>;
protected:
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
};

template <typename T>
ostream &operator<<(ostream &os, const LLNode<T> &node){
    return os << "(" <<node.getData() << ", " << node.getRef() << ")";
}

template <typename T>
struct BaseLinkedListTrait : public BaseContainerTrait<T, LLNode<T>>{

};

template <typename T>
struct AscendingLinkedListTrait : public BaseLinkedListTrait<T>,
                                  public AscendingTrait<T>
{
};

template <typename T>
struct DescendingLinkedListTrait : public BaseLinkedListTrait<T>,
                                   public DescendingTrait<T>
{
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

        Node* pTemp = other.m_pRoot;

        while(pTemp != nullptr){
            push_back(pTemp -> getData(), pTemp -> getRef());
            pTemp = pTemp->getNext();
        }
            
    }
    LinkedList(LinkedList &&other){ // Move constructor
        
        scoped_lock<mutex> lock(m_mtx);
        m_pRoot = exchange(other.m_pRoot, nullptr);
        m_pTail = exchange(other.m_pTail, nullptr);
        m_size = exchange(other.m_size, 0);

    }
    LinkedList& operator=(const LinkedList &other){ // Copy assignment operator
    }
    LinkedList& operator=(LinkedList &&other){ // Move assignment operator
    }
    
    virtual        ~LinkedList() {
        
        scoped_lock<mutex> lock(m_mtx);
        Node* pTemp = m_pRoot;

        while (pTemp){
            Node* pNext = pTemp->getNext();
            delete pTemp;
            pTemp = pNext;
        }

        m_pRoot = nullptr;
        m_pTail = nullptr;
        m_size  = 0;
    }

    virtual void   push_front(value_type value, Ref ref) {
        Node* pTemp = new Node(value, ref, m_pRoot);  //Se crea el Nodo temporal con los datos ingresados que apunta a m_pRoot
        
        scoped_lock<mutex> lock(m_mtx);
        m_pRoot = pTemp;                              //Se actualiza el nodo raiz
        if (m_size == 0)
            m_pTail = pTemp;                          //Si la lista esta vacia la cola tambien se debe actualizar
        ++m_size;
    }
    virtual auto    pop_front() -> pair<value_type, Ref>{ 
        
        scoped_lock<mutex> lock(m_mtx);
        if( m_pRoot ){
            Node* pTemp = m_pRoot;
            m_pRoot = m_pRoot->getNext();
            --m_size;
            return make_pair(pTemp->getData(), pTemp->getRef());
        }else
            throw out_of_range("pop_front(): empty list");
    }
    virtual void    push_back(value_type value, Ref ref){
        Node* pTemp = new Node(value, ref, nullptr);  //Como es el ultimo nodo no apunta a nada
        
        scoped_lock<mutex> lock(m_mtx);
        if (m_size == 0){
            m_pRoot = pTemp;
            m_pTail = pTemp; 
        } else {
            m_pTail->setNext(pTemp);
            m_pTail = pTemp;
        }                  
        ++m_size;
    }
    virtual auto    pop_back() -> pair<value_type, Ref>{
        
        scoped_lock<mutex> lock(m_mtx);
        if( !m_pRoot )
            throw out_of_range("pop_back(): empty list");
        
        //Con un solo elemento
        if( m_pRoot == m_pTail){
            auto pDelete = make_pair(m_pTail->getData(), m_pTail->getRef());

            delete m_pTail;
            m_pRoot = nullptr;
            m_pTail = nullptr;
            
            --m_size;
            return pDelete;
        }

        //Lista con varios elementos
        Node *pTemp = m_pRoot;

        while (pTemp->getNext() != m_pTail)             //Recorrer toda la lista hasta el penultimo elemento
            pTemp = pTemp->getNext();

        auto pDelete = make_pair(m_pTail->getData(), m_pTail->getRef());
        
        delete pTemp->getNext();
        pTemp->setNext(nullptr);

        m_pTail = pTemp;
        --m_size;
        return pDelete;
    }
private:
            void    internal_insert(Node* &pParent, const value_type &value, Ref ref);
public:
    virtual void    insert(const value_type &value, Ref ref);
    
    virtual Node& operator[](const size_t index) const{
        
        if (index >= m_size)
        throw out_of_range("Index out of range");

        Node* pTemp = m_pRoot;
        for (size_t i = 0; i < index; ++i){
            pTemp = pTemp -> getNext();
        }
        return *pTemp;
    };

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

    //Agregar FirstThat
    template <typename Func, typename... Args>
    forward_iterator FirstThat(Func func, Args &&...  args){
        return ::FirstThat(begin(), end(), func, std::forward<Args>(args)... );
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
    using value_type = typename LinkedList<Traits>::value_type;
    string line;

    getline(is, line);

    for (char& c : line){
        if (c == '[' || c == ']' || c == '(' || c == ')' || c == ',')
            c = ' ';
    }

    value_type value;
    Ref ref;
    stringstream ss(line);

    while (ss >> value >> ref){
        list.push_back(value, ref);
    }

    return is;
}

#endif // __LINKEDLIST_H__