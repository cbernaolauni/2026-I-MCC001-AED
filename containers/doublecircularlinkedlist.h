#include "doublelinkedlist.h"

template <typename T>
struct BaseCircularDoubleLinkedListTrait : public BaseContainerTrait<T, DLLNode<T>>{};

template <typename T>
struct AscendingCircularDoubleLinkedListTrait : public BaseCircularDoubleLinkedListTrait<T>{
    using Comp = less<T>;
};

template <typename T>
struct DescendingCircularDoubleLinkedListTrait : public BaseCircularDoubleLinkedListTrait<T>{
    using Comp = greater<T>;
};

// Forward iterator circular
template <typename Container>
class CircularDoubleLinkedListForwardIterator : public general_iterator<Container,
                                          CircularDoubleLinkedListForwardIterator<Container>>{
    using MySelf = CircularDoubleLinkedListForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    typename Parent::Node *m_pStart = nullptr;
public:
    CircularDoubleLinkedListForwardIterator(Container *container, typename Parent::Node *node,
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

// Backward iterator circular
template <typename Container>
class CircularDoubleLinkedListBackwardIterator : public general_iterator<Container,
                                          CircularDoubleLinkedListBackwardIterator<Container>>{
    using MySelf = CircularDoubleLinkedListBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    typename Parent::Node *m_pStart = nullptr;
public:
    CircularDoubleLinkedListBackwardIterator(Container *container, typename Parent::Node *node,
                                            typename Parent::Node *start)
        : Parent(container, node), m_pStart(start) {}

    MySelf& operator++(){
        if (!this->m_pNode)
            return *this;
        typename Parent::Node *pPrev = this->m_pNode->getPrev();
        if (pPrev == m_pStart)
            this->m_pNode = nullptr;
        else
            this->m_pNode = pPrev;
        return *this;
    }
};

template <typename Traits>
class CircularDoubleLinkedList : public DoubleLinkedList<Traits>{
public:
    using value_type = typename Traits::value_type;
    using Node       = typename Traits::Node;
    using Comp       = typename Traits::Comp;
    using MySelf     = CircularDoubleLinkedList<Traits>;

    using forward_iterator  = CircularDoubleLinkedListForwardIterator<MySelf>;
    using backward_iterator = CircularDoubleLinkedListBackwardIterator<MySelf>;

public:
    CircularDoubleLinkedList() : DoubleLinkedList<Traits>() {}

    CircularDoubleLinkedList(const CircularDoubleLinkedList &other) : DoubleLinkedList<Traits>() {
        scoped_lock<mutex> lock(other.m_mtx);
        if(!other.m_pRoot) return;
        Node *pNode = static_cast<Node*>(other.m_pRoot);
        do {
            push_back(pNode->getData(), pNode->getRef());
            pNode = pNode->getNext();
        } while(pNode != other.m_pRoot);
    }

    CircularDoubleLinkedList(CircularDoubleLinkedList &&other) : DoubleLinkedList<Traits>() {
        scoped_lock<mutex> lock(other.m_mtx);
        this->m_pRoot = exchange(other.m_pRoot, nullptr);
        this->m_pTail = exchange(other.m_pTail, nullptr);
        this->m_size  = exchange(other.m_size, 0);
    }

    virtual ~CircularDoubleLinkedList(){
        scoped_lock<mutex> lock(this->m_mtx);
        if(this->m_pTail){
            this->m_pTail->setNext(nullptr); 
            static_cast<Node*>(this->m_pRoot)->setPrev(nullptr);
        }
        // DoubleLinkedList::~DoubleLinkedList() -> LinkedList::~LinkedList() limpia el resto
    }

    void push_front(value_type value, Ref ref) override {
        scoped_lock<mutex> lock(this->m_mtx);
        Node* pNew = new Node(value, ref, static_cast<Node*>(this->m_pRoot), 
                              static_cast<Node*>(this->m_pTail));
        if(this->m_size == 0){
            this->m_pRoot = pNew;
            this->m_pTail = pNew;
            pNew->setNext(pNew);
            pNew->setPrev(pNew);
        } else {
            static_cast<Node*>(this->m_pRoot)->setPrev(pNew);
            this->m_pRoot = pNew;
            this->m_pTail->setNext(this->m_pRoot);
        }
        this->m_size++;
    }

    void push_back(value_type value, Ref ref) override {
        scoped_lock<mutex> lock(this->m_mtx);
        Node* pNew = new Node(value, ref, static_cast<Node*>(this->m_pRoot),
                              static_cast<Node*>(this->m_pTail));
        if(this->m_size == 0){
            this->m_pRoot = pNew;
            this->m_pTail = pNew;
            pNew->setNext(pNew);
            pNew->setPrev(pNew);
        } else {
            this->m_pTail->setNext(pNew);
            this->m_pTail = pNew;
            this->m_pTail->setNext(this->m_pRoot);
            static_cast<Node*>(this->m_pRoot)->setPrev(static_cast<Node*>(this->m_pTail));
        }
        this->m_size++;
    }

    pair<value_type, Ref> pop_front() override {
        scoped_lock<mutex> lock(this->m_mtx);
        if(!this->m_pRoot)
            throw out_of_range("pop_front(): empty list");

        auto result = make_pair(this->m_pRoot->getData(), this->m_pRoot->getRef());
        Node* pTemp = static_cast<Node*>(this->m_pRoot);

        if(this->m_size == 1){
            this->m_pRoot = nullptr;
            this->m_pTail = nullptr;
        } else {
            this->m_pRoot = pTemp->getNext();
            this->m_pTail->setNext(this->m_pRoot);
            static_cast<Node*>(this->m_pRoot)->setPrev(static_cast<Node*>(this->m_pTail));
        }
        delete pTemp;
        this->m_size--;
        return result;
    }

    pair<value_type, Ref> pop_back() override {
        scoped_lock<mutex> lock(this->m_mtx);
        if(!this->m_pRoot)
            throw out_of_range("pop_back(): empty list");

        Node* pTail = static_cast<Node*>(this->m_pTail);
        auto result = make_pair(pTail->getData(), pTail->getRef());

        if(this->m_size == 1){
            delete pTail;
            this->m_pRoot = nullptr;
            this->m_pTail = nullptr;
        } else {
            Node* pPrev = pTail->getPrev();
            delete pTail;
            this->m_pTail = pPrev;
            this->m_pTail->setNext(this->m_pRoot);
            static_cast<Node*>(this->m_pRoot)->setPrev(static_cast<Node*>(this->m_pTail));
        }
        this->m_size--;
        return result;
    }

    void insert(const value_type &value, Ref ref) override {
        scoped_lock<mutex> lock(this->m_mtx);
        // romper ciclo temporalmente
        if(this->m_pTail){
            this->m_pTail->setNext(nullptr);
            static_cast<Node*>(this->m_pRoot)->setPrev(nullptr);
        }

        this->internal_insert(this->m_pRoot, value, ref);

        Node *pNode = static_cast<Node*>(this->m_pRoot);
        Node *pPrev = static_cast<Node*>(this->m_pTail);
        do {
            pNode->setPrev(pPrev);
            pPrev = pNode;
            pNode = pNode->getNext();
        } while(pNode);

        this->m_pTail->setNext(this->m_pRoot);
        static_cast<Node*>(this->m_pRoot)->setPrev(static_cast<Node*>(this->m_pTail));
    }

    virtual string toString() override {
        scoped_lock<mutex> lock(this->m_mtx);
        stringstream ss;
        ss << "[";
        if(this->m_size > 0){
            Node* pNode = static_cast<Node*>(this->m_pRoot);
            for(size_t i = 0; i < this->m_size - 1; ++i){
                ss << *pNode << ",";
                pNode = pNode->getNext();
            }
            ss << *pNode;
        }
        ss << "]";
        return ss.str();
    }

    forward_iterator begin()   { return forward_iterator(this, static_cast<Node*>(this->m_pRoot), static_cast<Node*>(this->m_pRoot)); }
    forward_iterator end()     { return forward_iterator(this, nullptr, nullptr); }
    backward_iterator rbegin() { return backward_iterator(this, static_cast<Node*>(this->m_pTail), static_cast<Node*>(this->m_pTail)); }
    backward_iterator rend()   { return backward_iterator(this, nullptr, nullptr); }

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

