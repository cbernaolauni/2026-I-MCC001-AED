/**
 * @file TrieNode.h
 * @brief Nodo del Trie (CTrieNode), equivalente en este contenedor a
 *        CBTreePage en BTree: aquí viven el almacenamiento y los
 *        algoritmos recursivos reales (Insert, Search, StartsWith, Remove,
 *        Print). El contenedor Trie<Traits> solo agrega sincronización
 *        (mutex) e iteradores, igual que BTree<Traits> hace sobre CBTreePage.
 */
#ifndef CTrieNode_H
#define CTrieNode_H

#include <map>
#include <memory>
#include <string>
#include <iostream>
#include "basetrait.h"
#include "../types.h"
#include "TrieIterator.h"

using namespace std;

/**
 * @brief Códigos de resultado devueltos por CTrieNode::Insert.
 *
 * (Remove no usa este enum: comunica su resultado mediante el out-param
 * @c wasFound de RemoveAt, ver más abajo).
 */
enum trie_ErrorCode
{
        trie_ok,       ///< Inserción completada sin condiciones especiales.
        trie_duplicate ///< La palabra ya existía en el Trie.
};

/**
 * @brief Entrada sintetizada (palabra completa + ObjID) que expone
 *        TrieIterator al dereferenciarse.
 *
 * A diferencia de tagNode (BTree), donde cada nodo YA contiene su clave
 * completa, aquí la "clave" (palabra) no vive en ningún nodo: se reconstruye
 * caracter a caracter siguiendo el camino raíz -> nodo. Por eso TrieEntry es
 * un valor sintetizado por el iterador en cada paso, no un puntero a
 * almacenamiento persistente dentro del árbol.
 *
 * @tparam CharType Tipo del símbolo/carácter individual (normalmente char).
 * @tparam ObjIDType Tipo del identificador de objeto asociado a la palabra.
 */
template <typename CharType, typename ObjIDType>
struct TrieEntry
{
        basic_string<CharType> word; ///< Palabra completa reconstruida por el iterador.
        ObjIDType ObjID;             ///< Identificador de objeto asociado a la palabra.
        Counter UseCounter;          ///< Número de veces que esta palabra fue encontrada en Search.

        TrieEntry() : ObjID(), UseCounter(0) {}
        TrieEntry(const basic_string<CharType> &_word, ObjIDType _ObjID, Counter _use = 0)
            : word(_word), ObjID(_ObjID), UseCounter(_use) {}

        /// @brief Conversión implícita a la palabra, análoga a tagNode::operator keyType().
        operator basic_string<CharType>() { return word; }
        Counter GetUseCounter() { return UseCounter; }
};

/**
 * @brief Traits para Trie / CTrieNode.
 *
 * Define @c key_type (el alfabeto: tipo de cada símbolo), @c word_type
 * (la clave completa: una cadena de símbolos) y @c obj_type (identificador
 * de objeto asociado a cada palabra), igual que BTreePageTrait define
 * key_type/obj_type para BTree.
 *
 * @tparam CharType Tipo del símbolo/carácter individual (por defecto char).
 * @tparam ObjIDType Tipo del identificador de objeto (por defecto KeyRef).
 */
template <typename CharType = char, typename ObjIDType = KeyRef>
struct TrieTrait : BaseContainerTrait<basic_string<CharType>, TrieEntry<CharType, ObjIDType>>
{
        using key_type  = CharType;
        using word_type = basic_string<CharType>;
        using obj_type  = ObjIDType;
};

template <typename Traits>
class Trie; // fwd decl (friend)

/**
 * @brief Nodo del Trie: almacena sus hijos ordenados por símbolo y si
 *        representa el final de una palabra completa.
 *
 * Cada CTrieNode representa un símbolo dentro del camino raíz->nodo; la
 * palabra completa NO se guarda aquí, está implícita en dicho camino.
 * Los hijos se guardan en un @c map (no @c unordered_map) para que el
 * recorrido de TrieIterator sea determinístico y lexicográfico, igual que
 * las hojas enlazadas y ordenadas de CBTreePage permiten el recorrido
 * in-order de BTreeIterator.
 *
 * Invariante estructural (garantizada por Insert): todo nodo hoja (sin
 * hijos) tiene @c m_IsEndOfWord == true. TrieIterator depende de esta
 * invariante para no tener que manejar "caminos muertos" sin palabra.
 *
 * @tparam Traits Struct de traits que define key_type, word_type y obj_type.
 */
template <typename Traits>
class CTrieNode
{
        using CharType  = typename Traits::key_type;
        using word_type = typename Traits::word_type;
        using ObjIDType = typename Traits::obj_type;
        using TNode     = CTrieNode<Traits>;

        friend class Trie<Traits>;
        template <typename, TrieTraversalDirection> friend class TrieIterator;

public:
        using Node   = TrieEntry<CharType, ObjIDType>; ///< Tipo de entrada sintetizada por los iteradores.
        using TriePage = TNode;                        ///< Alias público del tipo de nodo, usado por TrieIterator.
        using ChildMap = map<CharType, unique_ptr<TNode>>;
        using ChildIterator = typename ChildMap::iterator;
        using ChildReverseIterator = typename ChildMap::reverse_iterator;

        CTrieNode() = default;
        /// @brief Los unique_ptr en m_Children liberan recursivamente todo el subárbol.
        ~CTrieNode() = default;

        /**
         * @brief Inserta @p word (a partir del símbolo en @p pos) en el
         *        subárbol enraizado en este nodo.
         *
         * Caso base (pos == word.size()): marca este nodo como fin de
         * palabra y guarda @p ObjID. Caso recursivo: crea el hijo para
         * word[pos] si no existe, y recurre en él con pos+1.
         *
         * @param word Palabra completa a insertar.
         * @param pos Índice del símbolo actual dentro de @p word.
         * @param ObjID Identificador de objeto asociado a la palabra.
         * @return trie_duplicate si la palabra ya existía; trie_ok en otro caso.
         */
        trie_ErrorCode Insert(const word_type &word, Count pos, ObjIDType ObjID);

        /**
         * @brief Busca @p word (a partir del símbolo en @p pos) en el
         *        subárbol enraizado en este nodo. Incrementa el UseCounter
         *        del nodo final si la búsqueda tiene éxito.
         *
         * @param word Palabra a buscar.
         * @param pos Índice del símbolo actual dentro de @p word.
         * @param[out] ObjID Se establece con el ObjID asociado si se encuentra.
         * @return @c true si la palabra existe completa en el Trie.
         */
        bool Search(const word_type &word, Count pos, ObjIDType &ObjID);

        /**
         * @brief Comprueba si algún camino desde este nodo cubre el resto
         *        de @p prefix (a partir de @p pos), sin exigir que ese
         *        camino sea además el final de una palabra completa.
         *
         * @param prefix Prefijo a verificar.
         * @param pos Índice del símbolo actual dentro de @p prefix.
         * @return @c true si el prefijo existe como camino en el Trie.
         */
        bool StartsWith(const word_type &prefix, Count pos) const;

        /**
         * @brief Elimina @p word (a partir del símbolo en @p pos) del
         *        subárbol enraizado en este nodo, podando ("lazy deletion")
         *        las ramas que quedan sin hijos y sin ser fin de palabra.
         *
         * @param word Palabra a eliminar.
         * @param pos Índice del símbolo actual dentro de @p word.
         * @param[out] wasFound Se establece en @c true si la palabra existía
         *                      y fue eliminada; @c false en otro caso.
         * @return @c true si el LLAMADOR (el nodo padre) debe borrar el hijo
         *         que corresponde a este nodo, porque quedó sin hijos y sin
         *         ser fin de otra palabra.
         */
        bool RemoveAt(const word_type &word, Count pos, bool &wasFound);

        /**
         * @brief Imprime el subárbol como un arbol de prefijos indentado,
         *        marcando con "*" los nodos que son fin de palabra.
         * @param os Stream de salida.
         * @param prefix Prefijo acumulado hasta este nodo (reservado; no se imprime directamente).
         * @param level Nivel de profundidad, usado para la indentación.
         */
        void Print(ostream &os, const word_type &prefix, Count level) const;

        /// @brief Número de hijos directos de este nodo.
        Capacity GetNumChildren() const { return static_cast<Capacity>(m_Children.size()); }
        /// @brief @c true si este nodo representa el final de una palabra completa.
        bool IsEndOfWord() const { return m_IsEndOfWord; }
        /// @brief ObjID asociado a la palabra que termina en este nodo (válido solo si IsEndOfWord()).
        ObjIDType GetObjID() const { return m_ObjID; }
        /// @brief Número de veces que la palabra que termina aquí fue encontrada por Search.
        Counter GetUseCounter() const { return m_UseCounter; }

        ChildIterator ChildrenBegin() { return m_Children.begin(); }
        ChildIterator ChildrenEnd()   { return m_Children.end(); }
        ChildReverseIterator ChildrenRBegin() { return m_Children.rbegin(); }
        ChildReverseIterator ChildrenREnd()   { return m_Children.rend(); }

protected:
        ChildMap  m_Children;             ///< Hijos ordenados por símbolo.
        bool      m_IsEndOfWord = false;   ///< @c true si el camino hasta aquí es una palabra completa.
        ObjIDType m_ObjID{};               ///< ObjID asociado (válido solo si m_IsEndOfWord).
        Counter   m_UseCounter = 0;        ///< Contador de búsquedas exitosas terminadas en este nodo.
};

template <typename Traits>
trie_ErrorCode CTrieNode<Traits>::Insert(const word_type &word, Count pos, ObjIDType ObjID)
{
        if (pos == static_cast<Count>(word.size()))
        {
                if (m_IsEndOfWord)
                        return trie_duplicate;
                m_IsEndOfWord = true;
                m_ObjID = ObjID;
                return trie_ok;
        }

        CharType c = word[pos];
        auto it = m_Children.find(c);
        if (it == m_Children.end())
                it = m_Children.emplace(c, make_unique<TNode>()).first;

        return it->second->Insert(word, pos + 1, ObjID);
}

template <typename Traits>
bool CTrieNode<Traits>::Search(const word_type &word, Count pos, ObjIDType &ObjID)
{
        if (pos == static_cast<Count>(word.size()))
        {
                if (!m_IsEndOfWord)
                        return false;
                m_UseCounter++;
                ObjID = m_ObjID;
                return true;
        }

        auto it = m_Children.find(word[pos]);
        if (it == m_Children.end())
                return false;

        return it->second->Search(word, pos + 1, ObjID);
}

template <typename Traits>
bool CTrieNode<Traits>::StartsWith(const word_type &prefix, Count pos) const
{
        if (pos == static_cast<Count>(prefix.size()))
                return true;

        auto it = m_Children.find(prefix[pos]);
        if (it == m_Children.end())
                return false;

        return it->second->StartsWith(prefix, pos + 1);
}

// Misma logica que el removeHelper original: elimina la palabra y poda
// hacia arriba las ramas que quedan sin hijos y sin ser fin de otra
// palabra. Unico agregado: el out-param wasFound, para que Trie::Remove
// pueda mantener m_NumWords sin repetir la busqueda.
template <typename Traits>
bool CTrieNode<Traits>::RemoveAt(const word_type &word, Count pos, bool &wasFound)
{
        if (pos == static_cast<Count>(word.size()))
        {
                if (!m_IsEndOfWord)
                {
                        wasFound = false;
                        return false;
                }
                m_IsEndOfWord = false;
                wasFound = true;
                return m_Children.empty();
        }

        CharType c = word[pos];
        auto it = m_Children.find(c);
        if (it == m_Children.end())
        {
                wasFound = false;
                return false;
        }

        bool shouldDeleteChild = it->second->RemoveAt(word, pos + 1, wasFound);

        if (shouldDeleteChild)
                m_Children.erase(it);

        return !m_IsEndOfWord && m_Children.empty();
}

template <typename Traits>
void CTrieNode<Traits>::Print(ostream &os, const word_type &prefix, Count level) const
{
        for (const auto &[c, child] : m_Children)
        {
                for (Count i = 0; i < level; i++)
                        os << "|   ";

                os << "+-- " << c;

                if (child->m_IsEndOfWord)
                        os << " *";

                os << "\n";

                child->Print(os, prefix + c, level + 1);
        }
}

#endif
