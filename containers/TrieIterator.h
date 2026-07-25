/**
 * @file TrieIterator.h
 * @brief Iterador lexicográfico (bidireccional) sobre las palabras de un Trie.
 *
 * A diferencia de BTreeIterator (donde cada clave vive físicamente dentro
 * de un nodo y basta una pila de {página, índice}), en un Trie una palabra
 * NO está almacenada en ningún nodo: está implícita en el *camino* desde la
 * raíz. Por eso el iterador debe reconstruir la palabra actual (m_word) a
 * medida que desciende/asciende por el árbol, y el valor devuelto por
 * operator*() es un TrieEntry sintetizado (no un puntero a almacenamiento
 * persistente).
 *
 * El recorrido se implementa como una máquina de pila de "tareas"
 * (Descend / Leave), equivalente a una recursión pre-order (Forward) o
 * post-order con hijos en reversa (Backward) pero ejecutada de forma
 * perezosa: cada llamada a advance() produce como máximo una palabra.
 */
#ifndef __TRIE_ITERATOR_H__
#define __TRIE_ITERATOR_H__

#include <stack>
#include <string>
#include <utility>
#include "../types.h"

using namespace std;

/**
 * @brief Sentido de recorrido lexicográfico de un TrieIterator.
 *
 * @c Forward recorre las palabras en orden ascendente (como strcmp).
 * @c Backward recorre las palabras en orden descendente.
 */
enum class TrieTraversalDirection { Forward, Backward };

/**
 * @brief Iterador lexicográfico sobre las palabras almacenadas en un Trie.
 *
 * @tparam TrieType Tipo del Trie contenedor (expone los alias Node, TriePage y key_type).
 * @tparam Dir Dirección de recorrido (Forward o Backward).
 */
template <typename TrieType, TrieTraversalDirection Dir>
class TrieIterator {
public:
    using Node     = typename TrieType::Node;      ///< Entrada sintetizada (palabra + ObjID) devuelta por operator*().
    using TriePage = typename TrieType::TriePage;   ///< Tipo de nodo del Trie (CTrieNode<Traits>).
    using CharType = typename TrieType::key_type;   ///< Tipo del símbolo/carácter individual.
    using MySelf   = TrieIterator<TrieType, Dir>;   ///< Alias del propio tipo iterador.

private:
    static constexpr bool IsForward = (Dir == TrieTraversalDirection::Forward);

    /// @brief Tarea pendiente en la pila: descender a un nodo o abandonar su ámbito (des-apilar su carácter).
    enum class TaskKind { Descend, Leave };

    struct Task {
        TaskKind kind;
        TriePage* page;
        bool      hasChar; ///< false únicamente para la raíz (no le corresponde ningún carácter).
        CharType  ch;
    };

    stack<Task>      m_tasks;
    basic_string<CharType> m_word;      ///< Camino acumulado desde la raíz hasta el nodo actual.
    TriePage*         m_pCurrentPage = nullptr; ///< Nodo cuya palabra fue sintetizada en m_current.
    Node              m_current;        ///< Copia de la palabra + ObjID actual (ver nota de clase).

    /// @brief Reconstruye m_current a partir de m_word y m_pCurrentPage.
    void sync_current() {
        m_current = Node(m_word, m_pCurrentPage->GetObjID(), m_pCurrentPage->GetUseCounter());
    }

    /**
     * @brief Avanza el iterador hasta la siguiente palabra en el orden
     *        de recorrido, procesando tareas de la pila hasta emitir una
     *        palabra o vaciar la pila (equivalente a llegar a end()).
     *
     * Forward (pre-order): al entrar a un nodo se revisa inmediatamente si
     * es fin de palabra (se emite ahí mismo, ANTES de descender a sus hijos,
     * dejando pendiente en la pila la tarea "Leave" que hará pop del
     * carácter una vez agotados los hijos). Los hijos se apilan en orden
     * descendente para que, al desapilarlos, salgan en orden ascendente.
     *
     * Backward (post-order con hijos en reversa): se apilan primero los
     * hijos (en orden ascendente, para que se desapilen en orden
     * descendente) y solo al agotarlos —en la tarea "Leave"— se revisa si
     * el nodo es fin de palabra y se emite, siempre después de haber
     * recorrido completamente sus hijos.
     */
    void advance() {
        while (!m_tasks.empty()) {
            Task t = m_tasks.top();
            m_tasks.pop();

            if (t.kind == TaskKind::Leave) {
                if constexpr (!IsForward) {
                    // Backward: el nodo se emite recien aqui, despues de
                    // haber agotado todos sus hijos (post-order).
                    if (t.page->IsEndOfWord()) {
                        m_pCurrentPage = t.page;
                        sync_current();
                        if (t.hasChar) m_word.pop_back();
                        return;
                    }
                }
                if (t.hasChar) m_word.pop_back();
                continue;
            }

            // TaskKind::Descend
            if (t.hasChar) m_word.push_back(t.ch);
            m_tasks.push(Task{TaskKind::Leave, t.page, t.hasChar, t.ch});

            if constexpr (IsForward) {
                // Empujar hijos en orden descendente -> se desapilan ascendente.
                for (auto it = t.page->ChildrenRBegin(); it != t.page->ChildrenREnd(); ++it)
                    m_tasks.push(Task{TaskKind::Descend, it->second.get(), true, it->first});

                if (t.page->IsEndOfWord()) {
                    m_pCurrentPage = t.page;
                    sync_current();
                    return; // la tarea Leave de este nodo queda pendiente para mas adelante
                }
            } else {
                // Empujar hijos en orden ascendente -> se desapilan descendente.
                for (auto it = t.page->ChildrenBegin(); it != t.page->ChildrenEnd(); ++it)
                    m_tasks.push(Task{TaskKind::Descend, it->second.get(), true, it->first});
                // El posible emit de este nodo se resuelve en su propia tarea Leave.
            }
        }
        m_pCurrentPage = nullptr;
    }

public:
    /// @brief Construye el iterador "end" (fuera de rango).
    explicit TrieIterator(nullptr_t) : m_pCurrentPage(nullptr) {}

    /**
     * @brief Construye el iterador "begin" a partir de la página raíz del Trie.
     * @param root Puntero a la raíz del Trie (nunca nullptr en uso normal).
     */
    explicit TrieIterator(TriePage* root) {
        if (!root) return;
        m_tasks.push(Task{TaskKind::Descend, root, false, CharType()});
        advance();
    }

    /// @brief Dereferencia el iterador; retorna una COPIA sintetizada (palabra + ObjID), no un puntero persistente.
    Node& operator*()  { return m_current; }
    Node* operator->() { return &m_current; }

    /// @brief Pre-incremento: avanza a la siguiente palabra en el orden de recorrido.
    MySelf& operator++() { advance(); return *this; }

    /// @brief Compara dos iteradores por el nodo actualmente referenciado.
    bool operator==(const MySelf& o) const { return m_pCurrentPage == o.m_pCurrentPage; }
    bool operator!=(const MySelf& o) const { return !(*this == o); }
};

#endif // __TRIE_ITERATOR_H__
