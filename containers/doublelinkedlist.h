#ifndef __DOUBLELINKEDLIST_H__
#define __DOUBLELINKEDLIST_H__

#include "linkedlist.h"

template <typename T>
class DLLNode :  public LLNode<T>{
public:
    using value_type = T;
    using Node       = DLLNode<T>;
private:
    Node *m_pPrev;
public:
    DLLNode(T data, Ref ref, Node *pNext = nullptr, Node *pPrev = nullptr)
        : LLNode<T>(data, ref, pNext), m_pPrev(pPrev){}

    Node*  getNext() const override { 
        return static_cast<Node*>(this->m_pNext); 
    }

    Node*  getPrev() const { return m_pPrev; }
    Node*& getPrevRef() { return m_pPrev; }
    void   setPrev(Node* pPrev) { m_pPrev = pPrev; }
};

template <typename T>
struct BaseDoubleLinkedListTrait : public BaseContainerTrait<T, DLLNode<T>>{

};

template <typename T>
struct AscendingDoubleLinkedListTrait : public BaseDoubleLinkedListTrait<T>{
    using Comp = less<T>;
};

template <typename T>
struct DescendingDoubleLinkedListTrait : public BaseDoubleLinkedListTrait<T>{
    using Comp = greater<T>;
};

// Reutiizar el LinkedListForwardIterator de la linked list

// Backward iterator
template <typename Container>
class DoubleLinkedListBackwardIterator : public general_iterator<Container, 
                                        DoubleLinkedListBackwardIterator<Container>>{
    using MySelf = DoubleLinkedListBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
public:
    MySelf& operator++(){
        this->m_pNode = this->m_pNode->getPrev();
        return *this;
    }
};

template <typename Traits>
class DoubleLinkedList : public LinkedList<Traits>{
public:
    using value_type = typename Traits::value_type;
    using Node       = typename Traits::Node;
    using Comp       = typename Traits::Comp;
    using MySelf     = DoubleLinkedList<Traits>;

    using forward_iterator  = LinkedListForwardIterator<MySelf>;
    using backward_iterator = DoubleLinkedListBackwardIterator<MySelf>;
public:
    DoubleLinkedList() : LinkedList<Traits>() {}
    DoubleLinkedList(const DoubleLinkedList &other) : LinkedList<Traits>() {
        scoped_lock<mutex> lock(other.m_mtx);
        Node *pNode = static_cast<Node*>(other.m_pRoot);
        while (pNode) {
            push_back(pNode->getData(), pNode->getRef());
            pNode = static_cast<Node*>(pNode->getNext());
        }
    }
    DoubleLinkedList(DoubleLinkedList &&other){
        scoped_lock<mutex> lock(other.m_mtx);
        this->m_pRoot = exchange(other.m_pRoot, nullptr);
        this->m_pTail = exchange(other.m_pTail, nullptr);
        this->m_size = exchange(other.m_size, 0);
    }

    /*
    Destructor heredado de linked list
    ~DoubleLinkedList() { }
    */
    
    size_t size () const { return this->m_size; }
    bool isEmpty() const { return this->m_pRoot == nullptr; }
    
    void insert(const value_type &value, Ref ref) override {
        scoped_lock<mutex> lock(this->m_mtx);
        this->internal_insert(this->m_pRoot, value, ref);

        Node *pNode = static_cast<Node*>(this->m_pRoot);
        Node *pPrev = nullptr;
        while (pNode) {
            pNode->setPrev(pPrev);
            pPrev = pNode;
            pNode = static_cast<Node*>(pNode->getNext());
        }
    }
    void push_back(value_type value, Ref ref) override {
        scoped_lock<mutex> lock(this->m_mtx);
        Node* pNew = new Node(value, ref, nullptr, static_cast<Node*>(this->m_pTail));
        if (this->m_pTail)
            this->m_pTail->setNext(pNew);
        this->m_pTail = pNew;
        if (this->m_size == 0)
            this->m_pRoot = this->m_pTail;
        this->m_size++;
    }
    pair<value_type, Ref> pop_back() override {
        scoped_lock<mutex> lock(this->m_mtx); 
        if(!this->m_pRoot)
            throw std::out_of_range("pop_back(): empty list");
        
        if (this->m_pRoot == this->m_pTail) {
            auto result = std::make_pair(this->m_pRoot->getData(), this->m_pRoot->getRef());
            delete this->m_pRoot;
            this->m_pRoot = nullptr;
            this->m_pTail = nullptr;
            this->m_size--;
            return result;
        }

        Node* pPrev = static_cast<Node*>(this->m_pTail)->getPrev();
        auto result = std::make_pair(this->m_pTail->getData(), this->m_pTail->getRef());
        delete this->m_pTail;

        this->m_pTail = pPrev;
        this->m_pTail->setNext(nullptr);
        this->m_size--;
        return result;
    }

    forward_iterator begin()   { return forward_iterator(this, static_cast<Node*>(this->m_pRoot)); }
    forward_iterator end()     { return forward_iterator(this, nullptr); }
    backward_iterator rbegin() { return backward_iterator(this, static_cast<Node*>(this->m_pTail)); }
    backward_iterator rend()   { return backward_iterator(this, nullptr); }

    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&... args){
        scoped_lock<mutex> lock(this->m_mtx);
        ::ForEach(begin(), end(), func, forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args &&... args){
        scoped_lock<mutex> lock(this->m_mtx);
        ::ForEach(rbegin(), rend(), func, forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    forward_iterator FirstThat(Func func, Args &&... args){
        scoped_lock<mutex> lock(this->m_mtx);
        return ::FirstThat(begin(), end(), func, forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    backward_iterator ReverseFirstThat(Func func, Args &&... args){
        scoped_lock<mutex> lock(this->m_mtx);
        return ::FirstThat(rbegin(), rend(), func, forward<Args>(args)...);
    }
};

template <typename Traits>
ostream& operator<<(ostream& os, DoubleLinkedList<Traits>& list){ 
    return operator<<(os, static_cast<LinkedList<Traits>&>(list));
}

template <typename Traits>
istream& operator>>(istream& is, DoubleLinkedList<Traits>& list){
    return operator>>(is, static_cast<LinkedList<Traits>&>(list));
}

#endif // __DOUBLELINKEDLIST_H__