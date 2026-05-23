#ifndef __HEAP_H__
#define __HEAP_H__
#include <vector>
#include <mutex>
#include "vector.h"

template <typename T>
class HeapNode{
    private:
        T m_data;
        Ref m_ref;
    public:
        HeapNode() : m_data{}, m_ref{} {}
        HeapNode(T data, Ref ref) : m_data(data), m_ref(ref) {}
        
        T GetData() const { return m_data; }
        Ref GetRef()  const { return m_ref;  }
};

template <typename T>
struct AscendingHeapTrait : public BaseContainerTrait<T, HeapNode<T> >,
                            public AscendingTrait<T>
{
};

template <typename T>
struct DescendingHeapTrait : public BaseContainerTrait<T, HeapNode<T> >,
                            public DescendingTrait<T>
{
};

// Revisar: https://www.cs.usfca.edu/~galles/visualization/Heap.html
// Pero en este ejercicio empezamos el la posicion [0]
template <typename Traits>
class Heap {
public:
    using value_type = typename Traits::value_type;
    using Node       = typename Traits::Node;
    using Comp       = typename Traits::Comp;
    using MySelf     = Heap<Traits>;
private:
    vector<Node> m_heap;
    Comp         m_comp;
    mutable mutex m_mtx;
public:

    Heap() {}

    Heap(const Heap &other) {
        scoped_lock lock(other.m_mtx);
        m_heap = other.m_heap;
    }

    Heap(Heap &&other) {
        scoped_lock lock(other.m_mtx);
        m_heap = move(other.m_heap);
    }

    Heap& operator=(const Heap &other) {
        if (this != &other) {
            scoped_lock lock(m_mtx, other.m_mtx);
            m_heap = other.m_heap;
        }
        return *this;
    }

    Heap& operator=(Heap &&other) {
        if (this != &other) {
            scoped_lock lock(m_mtx, other.m_mtx);
            m_heap = move(other.m_heap);
        }
        return *this;
    }

    ~Heap() {};

    void insert(const value_type &value, Ref ref) {
        scoped_lock lock(m_mtx);
        m_heap.push_back( Node(value, ref) );
        heapify_up(m_heap.size() - 1);
    }

    // Revisar completamente
    void extract() {
        scoped_lock lock(m_mtx);
        if (_empty()) {
            throw std::out_of_range("Heap is empty");
        }
        if (_size() == 1) {
            m_heap.pop_back();
            return;
        }
        m_heap[0] = move(m_heap.back());
        m_heap.pop_back();
        heapify_down(0);
    }

    value_type peek() const {
        scoped_lock lock(m_mtx);
        if (_empty()) {
            throw std::out_of_range("Heap is empty");
        }
        return m_heap[0].GetData();
    }

    bool empty() const {
        scoped_lock lock(m_mtx);
        return _empty();
    }

    size_t size() const {
        scoped_lock lock(m_mtx);
        return _size();
    }

    string toString() const;

private:
    bool _empty() const {
        return m_heap.empty();
    }

    size_t _size() const {
        return m_heap.size();
    }


    void heapify_up(size_t index) {
        while (index > 0) {
            size_t parent = (index - 1) / 2;
            if ( m_comp(m_heap[index].GetData(), m_heap[parent].GetData()) ) {
                std::swap(m_heap[index], m_heap[parent]);
                index = parent;
            } else {
                break;
            }
        }
    }

    void heapify_down(size_t index) {
        size_t left = 2 * index + 1;
        size_t right = 2 * index + 2;
        size_t smallest = index;

        if (left < m_heap.size() && m_comp(m_heap[left].GetData(), m_heap[smallest].GetData()) ) {
            smallest = left;
        }
        if (right < m_heap.size() && m_comp(m_heap[right].GetData(), m_heap[smallest].GetData()) ) {
            smallest = right;
        }

        if (smallest != index) {
            std::swap(m_heap[index], m_heap[smallest]);
            heapify_down(smallest);
        }
    }
};

template <typename Traits>
string Heap<Traits>::toString() const {
    scoped_lock lock(m_mtx);
    ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < m_heap.size(); i++)
        oss << m_heap[i].GetData() << (i < m_heap.size() - 1 ? "," : "");
    oss << "]";
    return oss.str();
}   

template <typename Traits>
ostream& operator<<(ostream& os, const Heap<Traits>& heap){
    return os << heap.toString();
}

template <typename Traits>
istream& operator>>(istream& is, Heap<Traits>& heap){
    using value_type = typename Heap<Traits>::value_type;

    char c;

    if (!(is >> c) || c != '[') {
        is.setstate(std::ios::failbit);
        return is;
    }

    value_type value;
    Ref ref = 0;

    while (is >> value) {
        heap.insert(value, ref++);
        
        if (!(is >> c)) {
            is.setstate(std::ios::failbit);
            return is;
        }
        if (c == ']') {
            break;
        } else if (c != ',') {
            is.setstate(std::ios::failbit);
            return is;
        }
    }

    return is;
}

#endif // __HEAP_H__