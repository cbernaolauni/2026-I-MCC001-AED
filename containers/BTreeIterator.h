/**
 * @file BTreeIterator.h
 * @brief Iterador in-order bidireccional (forward/backward) para BTree.
 *
 * Define BTreeTraversalDirection, BTreeFrame y la plantilla BTreeIterator,
 * que recorre las claves de un B-Tree en orden ascendente o descendente
 * usando una pila explícita en lugar de recursión.
 */

#ifndef __BTREE_ITERATOR_H__
#define __BTREE_ITERATOR_H__

#include <stack>
#include <utility>
#include "../types.h"

using namespace std;

/**
 * @brief Sentido de recorrido in-order de un BTreeIterator.
 *
 * @c Forward recorre las claves de menor a mayor (in-order izquierda->derecha).
 * @c Backward recorre las claves de mayor a menor (in-order derecha->izquierda).
 */
enum class BTreeTraversalDirection { Forward, Backward };

/**
 * @brief Cuadro de pila usado por BTreeIterator para simular la recursión
 *        del recorrido in-order de forma iterativa.
 *
 * Cada frame representa una página del B-Tree junto con el índice de la
 * clave que se procesará a continuación dentro de esa página.
 *
 * @tparam BTPage Tipo de página del B-Tree (CBTreePage<Traits>).
 */
template <typename BTPage>
struct BTreeFrame {
    BTPage* page;
    Count   index;

    /**
     * @brief Construye un frame para una página y un índice de clave dados.
     * @param p Puntero a la página.
     * @param i Índice inicial de clave dentro de la página.
     */
    BTreeFrame(BTPage* p, Count i) : page(p), index(i) {}
};

/**
 * @brief Iterador in-order (bidireccional) sobre las claves de un BTree.
 *
 * Implementa el recorrido in-order de forma iterativa mediante una pila
 * de BTreeFrame, evitando recursión explícita. La dirección de recorrido
 * (ascendente o descendente) se selecciona en tiempo de compilación con
 * el parámetro de plantilla @p Dir.
 *
 * @tparam BTreeType Tipo del BTree contenedor (expone los alias Node y BTPage).
 * @tparam Dir Dirección de recorrido (Forward o Backward).
 */
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

    /**
     * @brief Desciende desde @p page hasta la hoja más extrema, apilando
     *        un frame por cada página visitada en el camino.
     *
     * Para Forward desciende siempre por el hijo izquierdo (índice 0);
     * para Backward desciende siempre por el hijo derecho (último índice).
     *
     * @param page Página raíz del subárbol desde el cual descender.
     */
    void push_leftmost(BTPage* page) {
        while (page) {
            Count idx = IsForward ? 0 : page->GetNumberOfKeys() - 1;
            m_stack.push(Frame(page, idx));
            Count childIdx = IsForward ? 0 : page->GetNumberOfKeys();
            page = page->m_SubPages[childIdx];
        }
    }

    /**
     * @brief Avanza el iterador a la siguiente clave en orden de recorrido.
     *
     * Consume el frame en la cima de la pila, actualiza m_pCurrent con la
     * clave correspondiente, mueve el índice del frame en la dirección de
     * recorrido, y si el frame quedó agotado lo desapila. Si existe un
     * hijo pendiente por el lado de avance, desciende por él con
     * push_leftmost() antes de continuar. Si la pila queda vacía, deja
     * m_pCurrent en nullptr (equivalente al iterador end()).
     */
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

    /**
     * @brief Construye el iterador "end" (fuera de rango), equivalente a
     *        BTree::end() / BTree::rend().
     */
    explicit BTreeIterator(nullptr_t) : m_pCurrent(nullptr) {}

    /**
     * @brief Construye el iterador "begin" a partir de la página raíz del árbol.
     *
     * Desciende hasta la primera clave a visitar según la dirección @p Dir
     * y la deja lista en m_pCurrent. Si el árbol está vacío (raíz nula o
     * sin claves), el iterador queda equivalente al iterador end().
     *
     * @param root Puntero a la página raíz del BTree.
     */
    explicit BTreeIterator(BTPage* root) {
        if (!root || root->GetNumberOfKeys() == 0) return;
        push_leftmost(root);
        advance();
    }

    /// @brief Dereferencia el iterador, retornando el nodo/clave actual.
    Node&  operator*()  const { return *m_pCurrent; }
    /// @brief Acceso tipo puntero al nodo/clave actual.
    Node*  operator->() const { return m_pCurrent;  }
    /// @brief Retorna el puntero crudo al nodo/clave actual (o nullptr si es end()).
    Node*  getNode()    const { return m_pCurrent;  }

    /// @brief Pre-incremento: avanza el iterador a la siguiente clave en orden.
    MySelf& operator++() { advance(); return *this; }

    /// @brief Compara dos iteradores por igualdad de la clave actual apuntada.
    bool operator==(const MySelf& o) const { return m_pCurrent == o.m_pCurrent; }
    /// @brief Compara dos iteradores por desigualdad.
    bool operator!=(const MySelf& o) const { return !(*this == o); }
};

#endif // __BTREE_ITERATOR_H__
