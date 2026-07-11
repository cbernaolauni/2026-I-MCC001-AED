/**
 * @file BTree.h
 * @brief Contenedor BTree genérico, thread-safe, basado en Traits.
 *
 * Envuelve una CBTreePage<Traits> raíz y expone la API pública del árbol
 * (Insert, Remove, Search, Print), iteradores in-order (forward/backward)
 * e integración con las funciones libres ::ForEach y ::FirstThat.
 */
// btree.h

#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <mutex>
#include <utility>
#include "BTreePage.h"
#include "BTreeIterator.h"
#include "../foreach.h"

#define DEFAULT_BTREE_ORDER 3

/**
 * @brief Árbol B (B-Tree) thread-safe, parametrizado por Traits.
 *
 * BTree es la capa "lógica" del árbol: delega el almacenamiento y los
 * algoritmos de inserción/eliminación/split/merge en una página raíz
 * (CBTreePage<Traits>), y añade encima:
 *  - Sincronización con std::mutex en toda la API pública.
 *  - Iteradores in-order bidireccionales (forward_iterator / backward_iterator).
 *  - Integración con las funciones libres ::ForEach y ::FirstThat.
 *
 * @tparam Traits Struct de traits que define @c key_type y @c obj_type
 *                (ver BTreePageTrait, AscendingBTreeTrait, DescendingBTreeTrait).
 */
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
       using key_type = typename Traits::key_type;   ///< Tipo de la clave de búsqueda.
       using ObjIDType = typename Traits::obj_type;   ///< Tipo del identificador de objeto asociado a cada clave.
       using BTNode = CBTreePage<Traits>;             ///< Tipo de página interna del árbol.

public:
       using Node = typename BTNode::Node; ///< Tipo de nodo/clave almacenado en las páginas (tagNode).
       using BTPage  = BTNode;             ///< Alias público del tipo de página, usado por BTreeIterator.
       using MySelf  = BTree<Traits>;      ///< Alias del propio tipo BTree.

       using forward_iterator  = BTreeIterator<MySelf, BTreeTraversalDirection::Forward>;  ///< Iterador in-order ascendente.
       using backward_iterator = BTreeIterator<MySelf, BTreeTraversalDirection::Backward>; ///< Iterador in-order descendente.

       /**
        * @brief Construye un BTree vacío.
        * @param order  Orden del árbol (grado mínimo de ramificación).
        * @param unique Si es @c true, rechaza la inserción de claves duplicadas.
        */
       BTree(int order = DEFAULT_BTREE_ORDER, bool unique = true);
       /// @brief Destructor. Libera la página raíz y, recursivamente, todas sus páginas hijas.
       ~BTree();
       // int           Open (char * name, int mode);
       // int           Create (char * name, int mode);
       // int           Close ();
       
       /**
        * @brief Inserta una clave con su identificador de objeto asociado.
        * @param key   Clave a insertar.
        * @param ObjID Identificador de objeto asociado a la clave.
        * @return @c true si la inserción tuvo éxito; @c false si la clave ya
        *         existía y el árbol es @c unique.
        */
       bool Insert(const key_type key, ObjIDType ObjID);
       /**
        * @brief Elimina una clave del árbol.
        * @param key   Clave a eliminar.
        * @param ObjID Identificador de objeto (reservado para uso futuro).
        * @return @c true si la clave fue eliminada; @c false si no se encontró.
        */
       bool Remove(const key_type key, ObjIDType ObjID);
       /**
        * @brief Busca una clave en el árbol.
        * @param key Clave a buscar.
        * @return El ObjID asociado a la clave, o -1 si no se encontró.
        */
       ObjIDType Search(const key_type key);
       /// @brief Retorna el número total de claves almacenadas en el árbol.
       Count size() { scoped_lock lock(m_mutex); return m_NumKeys; }
       /// @brief Retorna la altura del árbol.
       Capacity height() { scoped_lock lock(m_mutex); return m_Height; }
       /// @brief Retorna el orden del árbol.
       Order GetOrder() { scoped_lock lock(m_mutex); return m_Order; }

       /**
        * @brief Imprime el árbol en orden in-order, indentado por nivel.
        * @param os Stream de salida sobre el que se imprime.
        */
       void Print(ostream &os);

       /**
        * @brief Aplica @p func a cada nodo del árbol en orden ascendente,
        *        delegando en la función libre ::ForEach sobre begin()/end().
        *
        * Adquiere el mutex del árbol durante todo el recorrido.
        *
        * @tparam Func Tipo del callable invocado por cada nodo.
        * @tparam Args Tipos de los argumentos extra reenviados a @p func.
        * @param func Callable con firma compatible con (Node&, Args...).
        * @param args Argumentos adicionales reenviados a @p func en cada llamada.
        */
       template <typename Func, typename... Args>
       void ForEach(Func func, Args &&...args)
       {
              scoped_lock lock(m_mutex);
              ::ForEach(begin(), end(), func, forward<Args>(args)...);
       }

       /**
        * @brief Busca el primer nodo (en orden ascendente) para el cual
        *        @p func retorna @c true, delegando en la función libre ::FirstThat.
        *
        * Adquiere el mutex del árbol durante toda la búsqueda.
        *
        * @tparam Func Tipo del callable predicado invocado por cada nodo.
        * @tparam Args Tipos de los argumentos extra reenviados a @p func.
        * @param func Predicado con firma compatible con (Node&, Args...) -> bool.
        * @param args Argumentos adicionales reenviados a @p func en cada llamada.
        * @return Puntero al primer Node que satisface @p func, o @c nullptr si ninguno lo hace.
        */
       template <typename Func, typename... Args>
       Node *FirstThat(Func func, Args &&...args)
       {
              scoped_lock lock(m_mutex);
              auto it = ::FirstThat(begin(), end(), func, forward<Args>(args)...);
              return (it != end()) ? &(*it) : nullptr;
       }
       // typedef               Node iterator;

       // Iteradores
       /// @brief Iterador in-order ascendente al inicio del árbol.
       forward_iterator  begin()  { return forward_iterator(&m_Root);   }
       /// @brief Iterador in-order ascendente "fuera de rango" (fin).
       forward_iterator  end()    { return forward_iterator(nullptr);    }
       /// @brief Iterador in-order descendente al inicio del árbol (clave mayor).
       backward_iterator rbegin() { return backward_iterator(&m_Root);  }
       /// @brief Iterador in-order descendente "fuera de rango" (fin).
       backward_iterator rend()   { return backward_iterator(nullptr);  }

protected:
       BTNode m_Root;        ///< Página raíz del árbol.
       Capacity m_Height;    ///< Altura actual del árbol.
       Order m_Order;        ///< Orden con el que fue creado el árbol.
       Count m_NumKeys;      ///< Número total de claves almacenadas.
       bool m_Unique;        ///< Si es @c true, no se permiten claves duplicadas.
       mutable mutex  m_mutex; ///< Mutex que serializa el acceso a la API pública del árbol.
};

const Order MaxHeight = 5;

/// @brief Inicializa la página raíz con capacidad @c 2*order+1 y altura 1.
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

/// @brief La liberación real de páginas ocurre en el destructor de CBTreePage (m_Root).
template <typename Traits>
BTree<Traits>::~BTree()
{
}

/**
 * @brief Inserta @p key en la página raíz; si esta reporta overflow,
 *        divide la raíz (SplitRoot) e incrementa la altura del árbol.
 */
template <typename Traits>
bool BTree<Traits>::Insert(const key_type key, ObjIDType ObjID)
{
       scoped_lock lock(m_mutex);
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

/**
 * @brief Elimina @p key desde la página raíz; si esta reporta que la
 *        raíz quedó fusionada (bt_rootmerged), decrementa la altura.
 */
template <typename Traits>
bool BTree<Traits>::Remove(const key_type key, ObjIDType ObjID)
{
       scoped_lock lock(m_mutex);
       bt_ErrorCode error = m_Root.Remove(key, ObjID);
       if (error == bt_duplicate || error == bt_nofound)
              return false;
       m_NumKeys--;

       if (error == bt_rootmerged)
              m_Height--;
       return true;
}

/// @brief Delega la búsqueda en la página raíz; retorna -1 si no se encuentra.
template <typename Traits>
typename BTree<Traits>::ObjIDType
BTree<Traits>::Search(const key_type key)
{
       scoped_lock lock(m_mutex);
       ObjIDType ObjID = -1;
       m_Root.Search(key, ObjID);
       return ObjID;
}

/// @brief Delega la impresión en la página raíz (recorrido in-order indentado).
template <typename Traits>
void BTree<Traits>::Print(ostream &os)
{
       scoped_lock lock(m_mutex);
       m_Root.Print(os);
}

#endif