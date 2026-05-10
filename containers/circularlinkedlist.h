#ifndef _CIRCULARLINKEDLIST_H__
#define _CIRCULARLINKEDLIST_H__

#include "linkedlist.h"

using namespace std;

template <typename Container>
class CircularLinkedListForwardIterator : public general_iterator<Container,
                                          CircularLinkedListForwardIterator<Container>>{
    using MySelf = CircularLinkedListForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    typename Parent::Node *m_pStart = nullptr;
public:
    CircularLinkedListForwardIterator(Container *container, typename Parent::Node *node,
                                      typename Parent::Node *start)
        : Parent(container, node), m_pStart(start) {}
    
    MySelf& operator++(){
        if (!this->m_pNode)
            return *this;
        typename Parent::Node *pNext = this->m_pNode->getNext();
        if (pNext == m_pStart)
            this->m_pNode = nullptr;
        else
            this->m_pNode = pNext;
        return *this;
    }
};

template <typename Container>
class CircularLinkedListBackwardIterator : public general_iterator<Container,
                                           CircularLinkedListBackwardIterator<Container>>{
    using MySelf = CircularLinkedListBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    typename Parent::Node *m_pStart = nullptr;
    typename Parent::Node *m_pTail = nullptr;
    
    typename Parent::Node* getPrevious(typename Parent::Node *current) const {
        if (!current || !m_pStart) return nullptr;
        typename Parent::Node *pNode = m_pStart;
        while (pNode && pNode->getNext() != current) {
            pNode = pNode->getNext();
            if (pNode == m_pStart) return nullptr;
        }
        return pNode;
    }
    
public:
    CircularLinkedListBackwardIterator(Container *container, typename Parent::Node *node,
                                       typename Parent::Node *start, typename Parent::Node *tail)
        : Parent(container, node), m_pStart(start), m_pTail(tail) {}
    
    MySelf& operator++(){
        if (!this->m_pNode) {
            return *this;
        }
        if (this->m_pNode == m_pStart) {
            this->m_pNode = nullptr;
        } else {
            this->m_pNode = getPrevious(this->m_pNode);
        }
        return *this;
    }
};

template <typename T>
struct BaseCircularLinkedListTrait : public BaseContainerTrait<T, LLNode<T>>{

};

template <typename T>
struct AscendingCircularLinkedListTrait : public BaseCircularLinkedListTrait<T>{
    using Comp = less<T>;
};

template <typename T>
struct DescendingCircularLinkedListTrait : public BaseCircularLinkedListTrait<T>{
    using Comp = greater<T>;
};

template <typename Traits>
class CircularLinkedList : public LinkedList<Traits>{
public:
    using value_type = typename Traits::value_type;
    using Node       = typename Traits::Node;
    using Comp       = typename Traits::Comp;
    using MySelf     = CircularLinkedList<Traits>;

    using forward_iterator = CircularLinkedListForwardIterator<MySelf>;
    using backward_iterator = CircularLinkedListBackwardIterator<MySelf>;

public:
    CircularLinkedList() : LinkedList<Traits>() {}

    CircularLinkedList(const CircularLinkedList &other) : LinkedList<Traits>() {
        scoped_lock<mutex> lock(other.m_mtx);
        if(!other.m_pRoot) return;
        Node *pNode = other.m_pRoot;
        do {
            push_back(pNode->getData(), pNode->getRef());
            pNode = pNode->getNext();
        } while(pNode != other.m_pRoot);
    }

    CircularLinkedList(CircularLinkedList &&other) : LinkedList<Traits>() {
        scoped_lock<mutex> lock(other.m_mtx);
        this->m_pRoot = exchange(other.m_pRoot, nullptr);
        this->m_pTail = exchange(other.m_pTail, nullptr);
        this->m_size  = exchange(other.m_size, 0);
    }

    virtual ~CircularLinkedList(){
        scoped_lock<mutex> lock(this->m_mtx);
        if(this->m_pTail)
            this->m_pTail->setNext(nullptr);
        // LinkedList::~LinkedList() se encarga del resto
    }

    virtual void push_front(value_type value, Ref ref) override {
        scoped_lock<mutex> lock(this->m_mtx);
        Node* pNew = new Node(value, ref, this->m_pRoot);
        if(this->m_size == 0){
            this->m_pRoot = pNew;
            this->m_pTail = pNew;
            pNew->setNext(pNew);
        } else {
            this->m_pRoot = pNew;
            this->m_pTail->setNext(this->m_pRoot);
        }
        this->m_size++;
    }

    virtual void push_back(value_type value, Ref ref) override {
        scoped_lock<mutex> lock(this->m_mtx);
        Node* pNew = new Node(value, ref, this->m_pRoot);
        if(this->m_size == 0){
            this->m_pRoot = pNew;
            this->m_pTail = pNew;
            pNew->setNext(pNew);
        } else {
            this->m_pTail->setNext(pNew);
            this->m_pTail = pNew;
            this->m_pTail->setNext(this->m_pRoot);
        }
        this->m_size++;
    }

    virtual auto pop_front() -> pair<value_type, Ref> override {
        scoped_lock<mutex> lock(this->m_mtx);
        if(!this->m_pRoot)
            throw out_of_range("pop_front(): empty list");

        auto result = make_pair(this->m_pRoot->getData(), this->m_pRoot->getRef());
        Node* pTemp = this->m_pRoot;

        if(this->m_size == 1){
            this->m_pRoot = nullptr;
            this->m_pTail = nullptr;
        } else {
            this->m_pRoot = this->m_pRoot->getNext();
            this->m_pTail->setNext(this->m_pRoot); // cerrar ciclo
        }
        delete pTemp;
        this->m_size--;
        return result;
    }

    virtual auto pop_back() -> pair<value_type, Ref> override {
        scoped_lock<mutex> lock(this->m_mtx);
        if(!this->m_pRoot)
            throw out_of_range("pop_back(): empty list");

        auto result = make_pair(this->m_pTail->getData(), this->m_pTail->getRef());

        if(this->m_size == 1){
            delete this->m_pTail;
            this->m_pRoot = nullptr;
            this->m_pTail = nullptr;
        } else {
            Node* pPrev = this->m_pRoot;
            while(pPrev->getNext() != this->m_pTail)
                pPrev = pPrev->getNext();
            delete this->m_pTail;
            this->m_pTail = pPrev;
            this->m_pTail->setNext(this->m_pRoot); // cerrar ciclo
        }
        this->m_size--;
        return result;
    }

    void insert(const value_type &value, Ref ref) override {
        scoped_lock<mutex> lock(this->m_mtx);
        // romper ciclo temporalmente para reutilizar internal_insert
        if(this->m_pTail)
            this->m_pTail->setNext(nullptr);

        this->internal_insert(this->m_pRoot, value, ref);

        this->m_pTail->setNext(this->m_pRoot);
    }

    virtual string toString() override {
        scoped_lock<mutex> lock(this->m_mtx);
        stringstream ss;
        ss << "[";
        if(this->m_size > 0){
            Node* pNode = this->m_pRoot;
            for(size_t i = 0; i < this->m_size - 1; ++i){
                ss << *pNode << ",";
                pNode = pNode->getNext();
            }
            ss << *pNode;
        }
        ss << "]";
        return ss.str();
    }

    forward_iterator begin() { 
        return forward_iterator(this, this->m_pRoot, this->m_pRoot); 
    }
    forward_iterator end() { 
        return forward_iterator(this, nullptr, nullptr);
    }

    backward_iterator rbegin() { 
        return backward_iterator(this, this->m_pTail, this->m_pRoot, this->m_pTail); 
    }

    backward_iterator rend() { 
        return backward_iterator(this, nullptr, nullptr, this->m_pTail);
    }

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
ostream& operator<<(ostream& os, CircularLinkedList<Traits>& list){
    return os << list.toString();
}

template <typename Traits>
istream& operator>>(istream& is, CircularLinkedList<Traits>& list){
    return operator>>(is, static_cast<LinkedList<Traits>&>(list));
}

#endif // __CIRCULARLINKEDLIST_H__ 