#ifndef __FOREACH_H__
#define __FOREACH_H__
#include <iostream>
#include <utility> // forward
#include <shared_mutex>

using namespace std;

template <typename Iterator, typename Func, typename... Args>
void ForEach(Iterator begin, Iterator end, Func func, Args &&... args){
    for (auto it = begin; it != end; ++it)
        func(*it, forward<Args>(args)...);
    // cout<<endl;
}

// Variadic templates: template <typename ...Args>
// Variadic templates allow a function or class to accept an arbitrary
// number of arguments.
// Example: template <typename ...Args> func() { // ... }
template <typename Iterator, typename Func, typename... Args>
Iterator FirstThat(Iterator begin, Iterator end, Func func, Args &&... args){
    for (auto it = begin; it != end; ++it){
        if (func(*it, forward<Args>(args)...))
            return it;
    }
    return end;
}

template <typename Container, typename Func, typename... Args>
void ForEach(Container& v1, Func func, Args &&... args){
    ForEach(v1.begin(), v1.end(), func, forward<Args>(args)...);
}

// Range proxy para iteradores seguros (mantiene el lock durante el uso)
template <typename Iterator, typename Mutex>
class SafeIteratorRange {
private:
    Iterator m_begin, m_end;
    std::shared_lock<std::shared_mutex> m_lock;
    
public:
    SafeIteratorRange(Iterator begin, Iterator end, Mutex& mutex)
        : m_begin(begin), m_end(end), m_lock(mutex) {}
    
    SafeIteratorRange(SafeIteratorRange&& other) noexcept
        : m_begin(other.m_begin), m_end(other.m_end), m_lock(std::move(other.m_lock)) {}
    
    SafeIteratorRange(const SafeIteratorRange&) = delete;
    SafeIteratorRange& operator=(const SafeIteratorRange&) = delete;
    
    ~SafeIteratorRange() {
        // El lock se libera automáticamente al destruir m_lock
    }
    
    Iterator begin() { return m_begin; }
    Iterator end()   { return m_end; }
};

// FirstThat seguro con lock
template <typename Mutex, typename Iterator, typename Func, typename... Args>
SafeIteratorRange<Iterator, Mutex> FirstThatSafeRange(Mutex& mutex, Iterator begin, Iterator end, Func func, Args&&... args) {
    std::shared_lock lock(mutex);
    for (auto it = begin; it != end; ++it) {
        if (func(*it, std::forward<Args>(args)...)) {
            auto it_next = it; ++it_next;
            return SafeIteratorRange<Iterator, Mutex>(it, it_next, mutex);
        }
    }
    return SafeIteratorRange<Iterator, Mutex>(end, end, mutex);
}

#endif // __FOREACH_H__