// btree.h

#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include "BTreePage.h"

#define DEFAULT_BTREE_ORDER 3

template <typename Traits>
class BTree
// this is the full version of the BTree
{
       /*struct Node
       {
               keyType first;
               long    second;
               Node *&operator->() { return this; }
       };*/
       using key_type = typename Traits::key_type;
       using ObjIDType = typename Traits::obj_type;
       using BTNode = CBTreePage<Traits>;

public:
       using Node = typename BTNode::Node;

       BTree(int order = DEFAULT_BTREE_ORDER, bool unique = true);
       ~BTree();
       // int           Open (char * name, int mode);
       // int           Create (char * name, int mode);
       // int           Close ();
       bool Insert(const key_type key, ObjIDType ObjID);
       bool Remove(const key_type key, ObjIDType ObjID);
       ObjIDType Search(const key_type key);
       Count size() { return m_NumKeys; }
       Capacity height() { return m_Height; }
       Order GetOrder() { return m_Order; }

       void Print(ostream &os);

       template <typename Func, typename... Args>
       void ForEach(Func func, Args &&...args)
       {
              m_Root.ForEach(func, 0, forward<Args>(args)...);
       }

       template <typename Func, typename... Args>
       Node *FirstThat(Func func, Args &&...args)
       {
              return m_Root.FirstThat([&func](Node& node, Count, auto&&... a) {
                     return func(node, a...);
                     }, 0, forward<Args>(args)...);
       }
       // typedef               Node iterator;

protected:
       BTNode m_Root;
       Capacity m_Height; // height of tree
       Order m_Order;     // order of tree
       Count m_NumKeys;   // number of keys
       bool m_Unique;     // Accept the elements only once ?
};

const int MaxHeight = 5;
template <typename Traits>
BTree<Traits>::BTree(Order order, bool unique)
    : m_Unique(unique),
      m_Order(order),
      m_Root(2 * order + 1, unique),
      m_Height(1),
      m_NumKeys(0)
{
       m_Root.SetMaxKeysForChilds(order);
}

template <typename Traits>
BTree<Traits>::~BTree()
{
}

template <typename Traits>
bool BTree<Traits>::Insert(const key_type key, const ObjIDType ObjID)
{
       bt_ErrorCode error = m_Root.Insert(key, ObjID);
       if (error == bt_duplicate)
              return false;
       m_NumKeys++;
       if (error == bt_overflow)
       {
              m_Root.SplitRoot();
              m_Height++;
       }
       return true;
}

template <typename Traits>
bool BTree<Traits>::Remove(const key_type key, const ObjIDType ObjID)
{
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if (error == bt_duplicate || error == bt_nofound)
              return false;
       m_NumKeys--;

       if (error == bt_rootmerged)
              m_Height--;
       return true;
}

template <typename Traits>
typename BTree<Traits>::ObjIDType
BTree<Traits>::Search(const key_type key)
{
       ObjIDType ObjID = -1;
       m_Root.Search(key, ObjID);
       return ObjID;
}

template <typename Traits>
void BTree<Traits>::Print(ostream &os)
{
       m_Root.Print(os);
}

#endif