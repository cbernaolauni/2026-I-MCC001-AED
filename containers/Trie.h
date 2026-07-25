/**
 * @file Trie.h
 * @brief Contenedor Trie genérico, thread-safe, basado en Traits.
 *
 * Envuelve un CTrieNode<Traits> raíz y expone la API pública (Insert,
 * Remove, Search, Contains, StartsWith, Print), iteradores lexicográficos
 * (forward/backward) e integración con las funciones libres ::ForEach y
 * ::FirstThat — exactamente el mismo esquema que BTree<Traits> aplica
 * sobre CBTreePage.
 */
#ifndef TRIE_H
#define TRIE_H

#include <iostream>
#include <mutex>
#include <optional>
#include <utility>
#include "TrieNode.h"
#include "TrieIterator.h"
#include "../foreach.h"

using namespace std;

/**
 * @brief Trie (árbol digital / árbol de prefijos) thread-safe, parametrizado por Traits.
 *
 * Al igual que BTree delega el almacenamiento y los algoritmos en
 * CBTreePage, Trie delega en CTrieNode (Insert/Search/StartsWith/Remove/
 * Print recursivos) y añade encima:
 *  - Sincronización con std::mutex en toda la API pública.
 *  - Iteradores lexicográficos bidireccionales (forward_iterator / backward_iterator).
 *  - Integración con las funciones libres ::ForEach y ::FirstThat.
 *
 * @tparam Traits Struct de traits que define @c key_type (símbolo), @c word_type
 *                (palabra completa) y @c obj_type (ver TrieTrait).
 */
template <typename Traits>
class Trie
{
        using CharType  = typename Traits::key_type;
        using word_type = typename Traits::word_type;
        using ObjIDType = typename Traits::obj_type;
        using TNode     = CTrieNode<Traits>;

public:
        using Node     = typename TNode::Node; ///< Entrada sintetizada (palabra + ObjID), ver TrieEntry.
        using TriePage = TNode;                ///< Alias público del tipo de nodo, usado por TrieIterator.
        using MySelf   = Trie<Traits>;          ///< Alias del propio tipo Trie.
        using key_type = CharType;              ///< Requerido por TrieIterator (tipo de símbolo).

        using forward_iterator  = TrieIterator<MySelf, TrieTraversalDirection::Forward>;  ///< Orden lexicográfico ascendente.
        using backward_iterator = TrieIterator<MySelf, TrieTraversalDirection::Backward>; ///< Orden lexicográfico descendente.

        Trie() = default;
        ~Trie() = default;

        /**
         * @brief Inserta @p word con su identificador de objeto asociado.
         * @param word Palabra a insertar.
         * @param ObjID Identificador de objeto asociado (opcional; por defecto ObjIDType()).
         * @return @c true si la inserción tuvo éxito; @c false si la palabra ya existía.
         */
        bool Insert(const word_type &word, ObjIDType ObjID = ObjIDType())
        {
                scoped_lock lock(m_mutex);
                trie_ErrorCode error = m_Root.Insert(word, 0, ObjID);
                if (error == trie_duplicate)
                        return false;
                m_NumWords++;
                return true;
        }

        /**
         * @brief Elimina @p word del Trie, podando las ramas que queden
         *        sin hijos y sin ser fin de otra palabra.
         * @param word Palabra a eliminar.
         * @return @c true si la palabra existía y fue eliminada.
         */
        bool Remove(const word_type &word)
        {
                scoped_lock lock(m_mutex);
                bool wasFound = false;
                m_Root.RemoveAt(word, 0, wasFound);
                if (wasFound)
                        m_NumWords--;
                return wasFound;
        }

        /**
         * @brief Busca @p word en el Trie.
         * @param word Palabra a buscar.
         * @param[out] ObjID Se establece con el ObjID asociado si se encuentra.
         * @return @c true si la palabra existe completa en el Trie.
         */
        bool Search(const word_type &word, ObjIDType &ObjID)
        {
                scoped_lock lock(m_mutex);
                return m_Root.Search(word, 0, ObjID);
        }

        /// @brief Comprueba la existencia de @p word sin necesitar su ObjID.
        bool Contains(const word_type &word)
        {
                ObjIDType dummy;
                return Search(word, dummy);
        }

        /**
         * @brief Comprueba si @p prefix existe como camino en el Trie
         *        (sin exigir que sea, además, una palabra completa).
         */
        bool StartsWith(const word_type &prefix)
        {
                scoped_lock lock(m_mutex);
                return m_Root.StartsWith(prefix, 0);
        }

        /// @brief Número de palabras completas actualmente almacenadas.
        Count size() { scoped_lock lock(m_mutex); return m_NumWords; }

        /// @brief Imprime el Trie como árbol de prefijos indentado (nodos fin de palabra marcados con "*").
        void Print(ostream &os)
        {
                scoped_lock lock(m_mutex);
                os << "(root)\n";
                m_Root.Print(os, word_type(), 0);
        }

        /**
         * @brief Aplica @p func a cada palabra del Trie en orden lexicográfico
         *        ascendente, delegando en la función libre ::ForEach sobre begin()/end().
         *
         * @tparam Func Tipo del callable invocado por cada entrada (Node&, Args...).
         * @tparam Args Tipos de los argumentos extra reenviados a @p func.
         */
        template <typename Func, typename... Args>
        void ForEach(Func func, Args &&...args)
        {
                scoped_lock lock(m_mutex);
                ::ForEach(begin(), end(), func, forward<Args>(args)...);
        }

        /**
         * @brief Busca la primera palabra (en orden lexicográfico) para la
         *        cual @p func retorna @c true, delegando en ::FirstThat.
         *
         * @note A diferencia de BTree::FirstThat (que retorna @c Node*
         *       apuntando a almacenamiento persistente dentro de una
         *       página), aquí cada Node es sintetizado por el iterador
         *       sobre la marcha — la palabra no vive en ningún nodo del
         *       árbol, sino que se reconstruye a partir del camino
         *       recorrido. Por eso se retorna una copia (optional<Node>)
         *       en vez de un puntero, evitando devolver la dirección de
         *       un valor temporal que dejaría de ser válido.
         *
         * @tparam Func Tipo del predicado invocado por cada entrada (Node&, Args...) -> bool.
         * @tparam Args Tipos de los argumentos extra reenviados a @p func.
         * @return Copia de la primera entrada que satisface @p func, o @c nullopt si ninguna lo hace.
         */
        template <typename Func, typename... Args>
        optional<Node> FirstThat(Func func, Args &&...args)
        {
                scoped_lock lock(m_mutex);
                auto it = ::FirstThat(begin(), end(), func, forward<Args>(args)...);
                if (it == end())
                        return nullopt;
                return *it;
        }

        // Iteradores
        /// @brief Iterador lexicográfico ascendente al inicio del Trie.
        forward_iterator  begin()  { return forward_iterator(&m_Root);  }
        /// @brief Iterador lexicográfico ascendente "fuera de rango" (fin).
        forward_iterator  end()    { return forward_iterator(nullptr);  }
        /// @brief Iterador lexicográfico descendente al inicio del Trie (última palabra).
        backward_iterator rbegin() { return backward_iterator(&m_Root); }
        /// @brief Iterador lexicográfico descendente "fuera de rango" (fin).
        backward_iterator rend()   { return backward_iterator(nullptr); }

protected:
        TNode m_Root;              ///< Nodo raíz del Trie (representa la palabra vacía).
        Count m_NumWords = 0;      ///< Número de palabras completas almacenadas.
        mutable mutex m_mutex;     ///< Mutex que serializa el acceso a la API pública del Trie.
};

#endif
