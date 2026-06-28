// BTreePage.h

#ifndef CBTreePage_H
#define CBTreePage_H

#include <vector>
#include <iostream>
#include <utility>
#include <assert.h>
#include "basetrait.h"
#include "../types.h"

using namespace std;

enum bt_ErrorCode
{
        bt_ok,
        bt_overflow,
        bt_underflow,
        bt_duplicate,
        bt_nofound,
        bt_rootmerged
};

// Si no lo encuentra, deberia decirme:
// cual es la posicion donde deberia estar
template <typename Container, typename ObjType>
Count binary_search(Container &container, Count first, Count last, ObjType &object)
{
        if (first >= last)
                return first;
        while (first < last)
        {
                Count mid = (first + last) / 2;
                if (object == (ObjType)container[mid])
                        return mid;
                if (object > (ObjType)container[mid])
                        first = mid + 1;
                else
                        last = mid;
        }
        if (object <= (ObjType)container[first])
                return first;
        return last;
}

template <typename Container, typename ObjType>
void insert_at(Container &container, const ObjType &object, Count pos)
{
        Count size = container.size();
        for (Count i = size - 2; i >= pos; i--)
                container[i + 1] = container[i];
        container[pos] = object;
}

template <typename Container>
void remove(Container &container, Count pos)
{
        Count size = container.size();
        for (Count i = pos + 1; i < size; i++)
                container[i - 1] = container[i];
}

template <typename Traits>
class BTree;

/*template <typename keyType>
bool operator>=(const _Node<keyType>& object1, const _Node<keyType>& object2)
{ return object1.key >= object2.key;    }

template <typename keyType>
bool operator<=(const _Node<keyType>& object1, const _Node<keyType>& object2)
{ return object1.key <= object2.key;    }*/

template <typename keyType, typename ObjIDType>
struct tagNode
{
        keyType key;
        ObjIDType ObjID;
        Counter UseCounter;

        tagNode(const keyType &_key, ObjIDType _ObjID)
            : key(_key), ObjID(_ObjID), UseCounter(0) {}
        tagNode() {}

        operator keyType() { return key; }
        Counter GetUseCounter() { return UseCounter; }
};

template <typename keyType, typename ObjIDType = KeyRef>
struct BTreePageTrait : BaseContainerTrait<keyType, tagNode<keyType, ObjIDType>>
{
        using key_type = keyType;
        using obj_type = ObjIDType;
};

template <typename Traits>
class CBTreePage
// this is the in-memory version of the CBTreePage
{
        using key_type = typename Traits::key_type;
        using ObjIDType = typename Traits::obj_type;

        friend class BTree<Traits>;
        using BTPage = CBTreePage<Traits>;

public:
        using Node = tagNode<key_type, ObjIDType>;

        CBTreePage(Capacity maxKeys, bool unique = true);
        virtual ~CBTreePage();

        bt_ErrorCode Insert(const key_type &key, ObjIDType ObjID);
        bt_ErrorCode Remove(const key_type &key, ObjIDType ObjID);
        bool Search(const key_type &key, ObjIDType &ObjID);
        void Print(ostream &os);

        // ForEach variadic template
        template <typename Func, typename... Args>
        void ForEach(Func func, Count level, Args&&... args)
        {
                for (Count i = 0; i < m_KeyCount; i++)
                {
                        if (m_SubPages[i])
                                m_SubPages[i]->ForEach(func, level + 1, forward<Args>(args)...);
                        func(m_Keys[i], level, forward<Args>(args)...);
                }
                if (m_SubPages[m_KeyCount])
                        m_SubPages[m_KeyCount]->ForEach(func, level + 1, forward<Args>(args)...);
        }

        // FirstThat variadic template
        template <typename Func, typename... Args>
        Node *FirstThat(Func func, Count level, Args&&... args)
        {
                for (Count i = 0; i < m_KeyCount; i++)
                {
                        if (m_SubPages[i])
                        {
                                Node *result = m_SubPages[i]->FirstThat(func, level + 1, forward<Args>(args)...);
                                if (result)
                                        return result;
                        }
                        if (func(m_Keys[i], level, forward<Args>(args)...))
                                return &m_Keys[i];
                }
                if (m_SubPages[m_KeyCount])
                        return m_SubPages[m_KeyCount]->FirstThat(func, level + 1, forward<Args>(args)...);
                return nullptr;
        }

protected:
        Capacity m_MinKeys;
        Capacity m_MaxKeys;
        Capacity m_MaxKeysForChilds;
        bool m_Unique;
        bool m_isRoot;
        vector<Node> m_Keys;
        vector<BTPage *> m_SubPages;
        Count m_KeyCount;

        void Create();
        void Reset();
        void Destroy()
        {
                Reset();
                delete this;
        }
        void clear() { m_KeyCount = 0; }

        bool Redistribute1(Count &pos);
        bool Redistribute2(Count pos);
        void RedistributeR2L(Count pos);
        void RedistributeL2R(Count pos);

        bool TreatUnderflow(Count &pos) { return Redistribute1(pos) || Redistribute2(pos); }

        bt_ErrorCode Merge(Count pos);
        bt_ErrorCode MergeRoot();
        void SplitChild(Count pos);

        Node &GetFirstNode();

        bool Overflow() { return m_KeyCount > m_MaxKeys; }
        bool Underflow() { return m_KeyCount < MinNumberOfKeys(); }
        bool IsFull() { return m_KeyCount >= m_MaxKeys; }
        Capacity MinNumberOfKeys() { return 2 * m_MaxKeys / 3.0; }
        Capacity GetFreeCells() { return m_MaxKeys - m_KeyCount; }
        Count &NumberOfKeys() { return m_KeyCount; }
        Count GetNumberOfKeys() { return m_KeyCount; }
        bool IsRoot() { return m_MaxKeysForChilds != m_MaxKeys; }
        void SetMaxKeysForChilds(Order orderforchilds) { m_MaxKeysForChilds = orderforchilds; }

        Capacity GetFreeCellsOnLeft(Count pos);
        Capacity GetFreeCellsOnRight(Count pos);

private:
        bool SplitRoot();
        void SplitPageInto3(vector<Node> &tmpKeys, vector<BTPage *> &tmpSubPages,
                            BTPage *&pChild1, BTPage *&pChild2, BTPage *&pChild3,
                            Node &oi1, Node &oi2);
        void MovePage(BTPage *pChildPage, vector<Node> &tmpKeys, vector<BTPage *> &tmpSubPages);
};

template <typename Traits>
CBTreePage<Traits>::CBTreePage(Capacity maxKeys, bool unique)
    : m_MaxKeys(maxKeys), m_Unique(unique), m_KeyCount(0)
{
        Create();
        SetMaxKeysForChilds(m_MaxKeys);
}

template <typename Traits>
CBTreePage<Traits>::~CBTreePage() { Reset(); }

template <typename Traits>
bt_ErrorCode CBTreePage<Traits>::Insert(const key_type &key, ObjIDType ObjID)
{
        Count pos = binary_search(m_Keys, 0, m_KeyCount, key);
        bt_ErrorCode error = bt_ok;

        if (pos < m_KeyCount && (key_type)m_Keys[pos] == key && m_Unique)
                return bt_duplicate; // this key is duplicate

        if (!m_SubPages[pos]) // this is a leave
        {
                ::insert_at(m_Keys, Node(key, ObjID), pos);
                NumberOfKeys()++;
                if (Overflow())
                        return bt_overflow;
                return bt_ok;
        }
        // recursive insertion
        error = m_SubPages[pos]->Insert(key, ObjID);
        if (error == bt_overflow)
        {
                if (!Redistribute1(pos))
                        SplitChild(pos);
                if (Overflow()) // Propagate overflow
                        return bt_overflow;
                return bt_ok;
        }
        return bt_ok;
}

template <typename Traits>
bool CBTreePage<Traits>::Redistribute1(Count &pos)
{
        if (m_SubPages[pos]->Underflow())
        {
                // nkol = Number of keys on left brother, nkor = Number of keys on right brother
                Count nkol = 0, nkor = 0;
                // is this the first element or there are more elements on right brother
                if (pos > 0)
                        nkol = m_SubPages[pos - 1]->NumberOfKeys();
                if (pos < NumberOfKeys())
                        nkor = m_SubPages[pos + 1]->NumberOfKeys();

                if (nkol > nkor)
                {
                        if (m_SubPages[pos - 1]->NumberOfKeys() > m_SubPages[pos - 1]->MinNumberOfKeys())
                                RedistributeL2R(pos - 1); // bring elements from left brother
                        else
                        {
                                if (pos == NumberOfKeys())
                                        --pos;
                                return false;
                        }
                }
                else
                { // nkol <= nkor
                        if (m_SubPages[pos + 1]->NumberOfKeys() > m_SubPages[pos + 1]->MinNumberOfKeys())
                                RedistributeR2L(pos + 1); // bring elements from right brother
                        else
                        {
                                if (pos == 0)
                                        ++pos;
                                return false;
                        }
                }
        }
        else
        { // it is due to overflow
                Capacity fcol = GetFreeCellsOnLeft(pos); // Free Cells On Left
                Capacity fcor = GetFreeCellsOnRight(pos); // Free Cells On Left

                if (!fcol && !fcor && m_SubPages[pos]->IsFull())
                        return false;
                if (fcol > fcor) // There is more space on left
                        RedistributeR2L(pos);
                else
                        RedistributeL2R(pos);
        }
        return true;
}

// Redistribute2 function
// it considers two brothers m_SubPages[pos-1] && m_SubPages[pos+1]
// if it fails the only way is merge !
template <typename Traits>
bool CBTreePage<Traits>::Redistribute2(Count pos)
{
        assert(pos > 0 && pos < NumberOfKeys());
        assert(m_SubPages[pos - 1] != nullptr && m_SubPages[pos] != nullptr && m_SubPages[pos + 1] != nullptr);
        assert(m_SubPages[pos - 1]->Underflow() ||
               m_SubPages[pos]->Underflow() ||
               m_SubPages[pos + 1]->Underflow());

        if (m_SubPages[pos - 1]->Underflow())
        {
                // Rotate R2L
                RedistributeR2L(pos + 1);
                RedistributeR2L(pos);
                if (m_SubPages[pos - 1]->Underflow())
                        return false;
        }
        else if (m_SubPages[pos + 1]->Underflow())
        {
                // Rotate L2R
                RedistributeL2R(pos - 1);
                RedistributeL2R(pos);
                if (m_SubPages[pos + 1]->Underflow())
                        return false;
        }
        else // The problem is exactly at pos !
        { 
                // Rotate L2R
                RedistributeL2R(pos - 1);
                RedistributeR2L(pos + 1);
                if (m_SubPages[pos]->Underflow())
                        return false;
        }
        return true;
}

template <typename Traits>
void CBTreePage<Traits>::RedistributeR2L(Count pos)
{
        BTPage *pSource = m_SubPages[pos];
        BTPage *pTarget = m_SubPages[pos - 1];

        while (pSource->GetNumberOfKeys() > pSource->MinNumberOfKeys() &&
               pTarget->GetNumberOfKeys() < pSource->GetNumberOfKeys())
        {
                // Move from this page to the down-left page \/
                ::insert_at(pTarget->m_Keys, m_Keys[pos - 1], pTarget->NumberOfKeys()++);
                // Move the leftest pointer to the rightest position
                ::insert_at(pTarget->m_SubPages, pSource->m_SubPages[0], pTarget->NumberOfKeys());

                // Move the leftest element to the root
                m_Keys[pos - 1] = pSource->m_Keys[0];

                // Remove the leftest element from right page
                ::remove(pSource->m_Keys, 0);
                ::remove(pSource->m_SubPages, 0);
                pSource->NumberOfKeys()--;
        }
}

template <typename Traits>
void CBTreePage<Traits>::RedistributeL2R(Count pos)
{
        BTPage *pSource = m_SubPages[pos];
        BTPage *pTarget = m_SubPages[pos + 1];

        while (pSource->GetNumberOfKeys() > pSource->MinNumberOfKeys() &&
               pTarget->GetNumberOfKeys() < pSource->GetNumberOfKeys())
        {
                // Move from this page to the down-RIGHT page \/
                ::insert_at(pTarget->m_Keys, m_Keys[pos], 0);
                // Move the rightest pointer to the leftest position
                ::insert_at(pTarget->m_SubPages, pSource->m_SubPages[pSource->NumberOfKeys()], 0);
                pTarget->NumberOfKeys()++;

                // Move the rightest element to the root
                m_Keys[pos] = pSource->m_Keys[pSource->NumberOfKeys() - 1];

                // Remove the rightest element from left page
                // it is not necessary erase because m_KeyCount controls
                pSource->NumberOfKeys()--;
        }
}

template <typename Traits>
void CBTreePage<Traits>::SplitChild(Count pos)
{
        // FIRST: deciding the second page to split
        BTPage *pChild1 = nullptr, *pChild2 = nullptr;

        if (pos > 0) // is left page full ?
                if (m_SubPages[pos - 1]->IsFull())
                {
                        pChild1 = m_SubPages[pos - 1];
                        pChild2 = m_SubPages[pos--];
                }
        if (pos < GetNumberOfKeys()) // is right page full ?
                if (m_SubPages[pos + 1]->IsFull())
                {
                        pChild1 = m_SubPages[pos];
                        pChild2 = m_SubPages[pos + 1];
                }

        // SECOND: copy both pages to a temporal one
        // Create two tmp vector
        vector<Node> tmpKeys;
        vector<BTPage *> tmpSubPages;

        // Prepara el vectpor unificado de las 2 paginas a ser divididas en 3
        // copy from left child
        MovePage(pChild1, tmpKeys, tmpSubPages);
        // copy a key from parent
        tmpKeys.push_back(m_Keys[pos]);

        // copy from right child
        MovePage(pChild2, tmpKeys, tmpSubPages);

        BTPage *pChild3 = nullptr;
        Node oi1, oi2;
        SplitPageInto3(tmpKeys, tmpSubPages, pChild1, pChild2, pChild3, oi1, oi2);

        // copy the first element to the root
        m_Keys[pos] = oi1;
        m_SubPages[pos] = pChild1;

        // copy the second element to the root
        ::insert_at(m_Keys, oi2, pos + 1);
        ::insert_at(m_SubPages, pChild2, pos + 1);
        NumberOfKeys()++;

        m_SubPages[pos + 2] = pChild3;
}

template <typename Traits>
void CBTreePage<Traits>::SplitPageInto3(vector<Node> &tmpKeys, vector<BTPage *> &tmpSubPages,
                                        BTPage *&pChild1, BTPage *&pChild2, BTPage *&pChild3,
                                        Node &oi1, Node &oi2)
{
        assert(tmpKeys.size() >= 8);
        assert(tmpSubPages.size() >= 9);

        if (!pChild1)
                pChild1 = new BTPage(m_MaxKeysForChilds, m_Unique);
        
        // Split tmpKeys page into 3 pages
        // copy 1/3 elements to the first child
        pChild1->clear();
        Count nKeys = (tmpKeys.size() - 2) / 3;
        Count i = 0;
        for (; i < nKeys; i++)
        {
                pChild1->m_Keys[i] = tmpKeys[i];
                pChild1->m_SubPages[i] = tmpSubPages[i];
                pChild1->NumberOfKeys()++;
        }
        pChild1->m_SubPages[i] = tmpSubPages[i];

        // first element to go up !
        oi1 = tmpKeys[i++];

        if (!pChild2)
                pChild2 = new BTPage(m_MaxKeysForChilds, m_Unique);
        pChild2->clear();
        // copy 1/3 to the second child
        nKeys += (tmpKeys.size() - 2) / 3 + 1;
        Count j = 0;
        for (; i < nKeys; i++, j++)
        {
                pChild2->m_Keys[j] = tmpKeys[i];
                pChild2->m_SubPages[j] = tmpSubPages[i];
                pChild2->NumberOfKeys()++;
        }
        pChild2->m_SubPages[j] = tmpSubPages[i];

        // copy the second element to the root
        oi2 = tmpKeys[i++];

        // copy 1/3 to the third child
        if (!pChild3)
                pChild3 = new BTPage(m_MaxKeysForChilds, m_Unique);
        pChild3->clear();
        nKeys = tmpKeys.size();
        for (j = 0; i < nKeys; i++, j++)
        {
                pChild3->m_Keys[j] = tmpKeys[i];
                pChild3->m_SubPages[j] = tmpSubPages[i];
                pChild3->NumberOfKeys()++;
        }
        pChild3->m_SubPages[j] = tmpSubPages[i];
}

template <typename Traits>
bool CBTreePage<Traits>::SplitRoot()
{
        BTPage *pChild1 = nullptr, *pChild2 = nullptr, *pChild3 = nullptr;
        Node oi1, oi2;
        SplitPageInto3(m_Keys, m_SubPages, pChild1, pChild2, pChild3, oi1, oi2);
        clear();

        // copy the first element to the root
        m_Keys[0] = oi1;
        m_SubPages[0] = pChild1;
        NumberOfKeys()++;

        // copy the second element to the root
        m_Keys[1] = oi2;
        m_SubPages[1] = pChild2;
        NumberOfKeys()++;

        m_SubPages[2] = pChild3;
        return true;
}

template <typename Traits>
bool CBTreePage<Traits>::Search(const key_type &key, ObjIDType &ObjID)
{
        Count pos = binary_search(m_Keys, 0, m_KeyCount, key);
        if (pos >= m_KeyCount)
        {
                if (m_SubPages[pos])
                        return m_SubPages[pos]->Search(key, ObjID);
                else
                        return false;
        }
        if (key == m_Keys[pos].key)
        {
                ObjID = m_Keys[pos].ObjID;
                m_Keys[pos].UseCounter++;
                return true;
        }
        if (key < m_Keys[pos].key)
                if (m_SubPages[pos])
                        return m_SubPages[pos]->Search(key, ObjID);
        return false;
}

/*template <typename keyType, typename ObjIDType>
void CBTreePage<keyType, ObjIDType>::ForEachReverse(lpfnForEach2 lpfn, int level, void *pExtra1)
{
       if( m_SubPages[m_KeyCount] )
               m_SubPages[m_KeyCount]->ForEach(lpfn, level+1, pExtra1);
       for( int i = m_KeyCount-1 ; i >= 0  ; i--)
       {
               lpfn(m_Keys[i], level, pExtra1);
               if( m_SubPages[i] )
                       m_SubPages[i]->ForEach(lpfn, level+1, pExtra1);
       }
}*/

template <typename Traits>
bt_ErrorCode CBTreePage<Traits>::Remove(const key_type &key, ObjIDType ObjID)
{
        bt_ErrorCode error = bt_ok;
        Count pos = binary_search(m_Keys, 0, m_KeyCount, key);

        if (pos < NumberOfKeys() && key == m_Keys[pos].key)
        {
                // This is a leave: First
                if (!m_SubPages[pos + 1]) // This is a leave ? FIRST CASE !
                {
                        ::remove(m_Keys, pos);
                        NumberOfKeys()--;
                        if (Underflow())
                                return bt_underflow;
                        return bt_ok;
                }

                // We FOUND IT BUT it is NOT a leave ? SECOND CASE !
                {
                        // Get the first element from right branch
                        Node &rFirstFromRight = m_SubPages[pos + 1]->GetFirstNode();
                        // change with a leave
                        swap(m_Keys[pos], rFirstFromRight);
                        // Remove it from this leave

                        // Print(cout);
                        error = m_SubPages[++pos]->Remove(key, ObjID);
                }
        }
        else if (pos == NumberOfKeys()) // it is not here, go by the last branch
                error = m_SubPages[pos]->Remove(key, ObjID);
        else if (key <= m_Keys[pos].key)
        { // = is because identical keys are inserted on left (see Insert)
                if (m_SubPages[pos])
                        error = m_SubPages[pos]->Remove(key, ObjID);
                else
                        return bt_nofound;
        }

        if (error == bt_underflow)
        {
                // THIRD CASE: After removing the element we have an underflow
                // Print(cout);
                if (TreatUnderflow(pos))
                        return bt_ok;
                // FOURTH CASE: it was not possible to redistribute -> Merge
                if (IsRoot() && NumberOfKeys() == 2)
                        return MergeRoot();
                return Merge(pos);
        }
        if (error == bt_nofound)
                return bt_nofound;
        return bt_ok;
}


template <typename Traits>
bt_ErrorCode CBTreePage<Traits>::Merge(Count pos)
{
        assert(m_SubPages[pos - 1]->NumberOfKeys() +
                   m_SubPages[pos]->NumberOfKeys() +
                   m_SubPages[pos + 1]->NumberOfKeys() ==
               3 * m_SubPages[pos]->MinNumberOfKeys() - 1);

        // FIRST: Put all the elements into a vector
        vector<Node> tmpKeys;
        // tmpKeys.resize(nKeys);
        vector<BTPage *> tmpSubPages;

        BTPage *pChild1 = m_SubPages[pos - 1];
        BTPage *pChild2 = m_SubPages[pos];
        BTPage *pChild3 = m_SubPages[pos + 1];

        MovePage(pChild1, tmpKeys, tmpSubPages);
        tmpKeys.push_back(m_Keys[pos - 1]);
        MovePage(pChild2, tmpKeys, tmpSubPages);
        tmpKeys.push_back(m_Keys[pos]);
        MovePage(pChild3, tmpKeys, tmpSubPages);
        pChild3->Destroy();

        // Move 1/2 elements to pChild1
        Capacity nKeys = pChild1->GetFreeCells();
        Count i = 0;
        for (; i < nKeys; i++)
        {
                pChild1->m_Keys[i] = tmpKeys[i];
                pChild1->m_SubPages[i] = tmpSubPages[i];
                pChild1->NumberOfKeys()++;
        }
        pChild1->m_SubPages[i] = tmpSubPages[i];

        m_Keys[pos - 1] = tmpKeys[i];
        m_SubPages[pos - 1] = pChild1;

        ::remove(m_Keys, pos);
        ::remove(m_SubPages, pos);
        NumberOfKeys()--;

        nKeys = pChild2->GetFreeCells();
        Count j = ++i;
        for (i = 0; i < nKeys; i++, j++)
        {
                pChild2->m_Keys[i] = tmpKeys[j];
                pChild2->m_SubPages[i] = tmpSubPages[j];
                pChild2->NumberOfKeys()++;
        }
        pChild2->m_SubPages[i] = tmpSubPages[j];
        m_SubPages[pos] = pChild2;

        if (Underflow())
                return bt_underflow;
        return bt_ok;
}

template <typename Traits>
bt_ErrorCode CBTreePage<Traits>::MergeRoot()
{
        Count pos = 1;
        assert(m_SubPages[pos - 1]->NumberOfKeys() +
                   m_SubPages[pos]->NumberOfKeys() +
                   m_SubPages[pos + 1]->NumberOfKeys() ==
               3 * m_SubPages[pos]->MinNumberOfKeys() - 1);

        BTPage *pChild1 = m_SubPages[pos - 1];
        BTPage *pChild2 = m_SubPages[pos];
        BTPage *pChild3 = m_SubPages[pos + 1];
        Count nKeys = pChild1->NumberOfKeys() + pChild2->NumberOfKeys() + pChild3->NumberOfKeys() + 2;

        // FIRST: Put all the elements into a vector
        vector<Node> tmpKeys;
        // tmpKeys.resize(nKeys);
        vector<BTPage *> tmpSubPages;

        MovePage(pChild1, tmpKeys, tmpSubPages);
        tmpKeys.push_back(m_Keys[pos - 1]);
        MovePage(pChild2, tmpKeys, tmpSubPages);
        tmpKeys.push_back(m_Keys[pos]);
        MovePage(pChild3, tmpKeys, tmpSubPages);

        clear();
        Count i = 0;
        for (; i < nKeys; i++)
        {
                m_Keys[i] = tmpKeys[i];
                m_SubPages[i] = tmpSubPages[i];
                NumberOfKeys()++;
        }
        m_SubPages[i] = tmpSubPages[i];

        // Print(cout);
        pChild1->Destroy();
        pChild2->Destroy();
        pChild3->Destroy();

        return bt_rootmerged;
}

template <typename Traits>
typename CBTreePage<Traits>::Node &
CBTreePage<Traits>::GetFirstNode()
{
        if (m_SubPages[0])
                return m_SubPages[0]->GetFirstNode();
        return m_Keys[0];
}

template <typename Traits>
void CBTreePage<Traits>::Print(ostream &os)
{
        ForEach([](Node &info, Count level, ostream &out)
                {
        for (Count i = 0; i < level; i++)
            out << "\t";
        out << info.key << "->" << info.ObjID << "\n"; }, 0, os);
}

template <typename Traits>
void CBTreePage<Traits>::Create()
{
        Reset();
        m_Keys.resize(m_MaxKeys + 1);
        m_SubPages.resize(m_MaxKeys + 2, nullptr);
        m_KeyCount = 0;
        m_MinKeys = 2 * m_MaxKeys / 3;
}

template <typename Traits>
void CBTreePage<Traits>::Reset()
{
        for (Count i = 0; i < m_KeyCount; i++)
                delete m_SubPages[i];
        clear();
}

template <typename Traits>
void CBTreePage<Traits>::MovePage(BTPage *pChildPage, vector<Node> &tmpKeys, vector<BTPage *> &tmpSubPages)
{
        Count i = 0;
        for (; i < pChildPage->GetNumberOfKeys(); i++)
        {
                tmpKeys.push_back(pChildPage->m_Keys[i]);
                tmpSubPages.push_back(pChildPage->m_SubPages[i]);
        }
        tmpSubPages.push_back(pChildPage->m_SubPages[i]);
        pChildPage->clear();
}

template <typename Traits>
Capacity CBTreePage<Traits>::GetFreeCellsOnLeft(Count pos)
{
        if (pos > 0) // there is some page on left ?
                return m_SubPages[pos - 1]->GetFreeCells();
        return 0;
}

template <typename Traits>
Capacity CBTreePage<Traits>::GetFreeCellsOnRight(Count pos)
{
        if (pos < GetNumberOfKeys()) // there is some page on right ?
                return m_SubPages[pos + 1]->GetFreeCells();
        return 0;
}

#endif // CBTreePage_H