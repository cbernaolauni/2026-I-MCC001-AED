#ifndef __BTREE_ITERATOR_H__
#define __BTREE_ITERATOR_H__

#include <stack>
#include <utility>
#include "../types.h"

using namespace std;

enum class BTreeTraversalDirection { Forward, Backward };

template <typename BTPage>
struct BTreeFrame {
    BTPage* page;
    Count   index;
    BTreeFrame(BTPage* p, Count i) : page(p), index(i) {}
};

template <typename BTreeType, BTreeTraversalDirection Dir>
class BTreeIterator {
public:
    using Node   = typename BTreeType::Node;
    using BTPage = typename BTreeType::BTPage;
    using MySelf = BTreeIterator<BTreeType, Dir>;
    using Frame  = BTreeFrame<BTPage>;

private:
    static constexpr bool IsForward = (Dir == BTreeTraversalDirection::Forward);

    stack<Frame> m_stack;
    Node*        m_pCurrent = nullptr;

    // Desciende al primer nodo hoja por la izquierda (forward) o derecha (backward)
    void push_leftmost(BTPage* page) {
        while (page) {
            Count idx = IsForward ? 0 : page->GetNumberOfKeys() - 1;
            m_stack.push(Frame(page, idx));
            Count childIdx = IsForward ? 0 : page->GetNumberOfKeys();
            page = page->m_SubPages[childIdx];
        }
    }

    void advance() {
        if (m_stack.empty()) { m_pCurrent = nullptr; return; }

        Frame& f = m_stack.top();
        m_pCurrent = &f.page->m_Keys[f.index];

        Count nextChild = IsForward ? f.index + 1 : f.index;
        BTPage* child   = f.page->m_SubPages[nextChild];

        if (IsForward) f.index++;
        else           f.index--;

        bool exhausted = IsForward
            ? f.index >= f.page->GetNumberOfKeys()
            : f.index < 0;

        if (exhausted) m_stack.pop();

        if (child) push_leftmost(child);
    }

public:
    // end iterator
    explicit BTreeIterator(nullptr_t) : m_pCurrent(nullptr) {}

    // begin iterator — recibe la página raíz
    explicit BTreeIterator(BTPage* root) {
        if (!root || root->GetNumberOfKeys() == 0) return;
        push_leftmost(root);
        advance();
    }

    Node&  operator*()  const { return *m_pCurrent; }
    Node*  operator->() const { return m_pCurrent;  }
    Node*  getNode()    const { return m_pCurrent;  }

    MySelf& operator++() { advance(); return *this; }

    bool operator==(const MySelf& o) const { return m_pCurrent == o.m_pCurrent; }
    bool operator!=(const MySelf& o) const { return !(*this == o); }
};

#endif // __BTREE_ITERATOR_H__
