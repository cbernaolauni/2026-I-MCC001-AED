/**
 * @file BTreePage.h
 * @brief Página del B-Tree (CBTreePage) y utilidades libres asociadas.
 *
 * Contiene el almacenamiento y los algoritmos internos del árbol B:
 * inserción, eliminación, redistribución entre hermanos, split y merge
 * de páginas. También define los Traits (BTreePageTrait y variantes)
 * y el tipo de nodo tagNode que almacena cada clave junto a su ObjID.
 */
// BTreePage.h

#ifndef CBTreePage_H
#define CBTreePage_H

#include <vector>
#include <iostream>
#include <utility>
#include <assert.h>
#include "basetrait.h"
#include "../types.h"
#include "BTreeIterator.h"

using namespace std;

/**
 * @brief Códigos de resultado devueltos por las operaciones internas
 *        de CBTreePage (Insert, Remove, Merge, MergeRoot).
 */
enum bt_ErrorCode
{
        bt_ok,          ///< Operación completada sin condiciones especiales.
        bt_overflow,    ///< La página excedió su capacidad máxima tras una inserción.
        bt_underflow,   ///< La página quedó por debajo del mínimo de claves tras una eliminación.
        bt_duplicate,   ///< La clave ya existía y el árbol requiere unicidad.
        bt_nofound,     ///< La clave a eliminar no fue encontrada.
        bt_rootmerged   ///< La página raíz fue fusionada con sus hijos (el árbol pierde un nivel).
};

/**
 * @brief Búsqueda binaria de @p object dentro de @p container en el rango
 *        [first, last).
 *
 * Si no encuentra el elemento, retorna la posición donde debería insertarse
 * para mantener el orden ascendente (no retorna -1 ni un valor centinela).
 *
 * @tparam Container Tipo indexable (p. ej. std::vector) que almacena elementos
 *                    convertibles a @p ObjType.
 * @tparam ObjType Tipo comparable de la clave buscada.
 * @param container Contenedor donde buscar.
 * @param first Índice inicial (inclusive) del rango de búsqueda.
 * @param last  Índice final (exclusivo) del rango de búsqueda.
 * @param object Clave a buscar.
 * @return Índice del elemento si se encuentra, o la posición de inserción si no.
 */
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

/**
 * @brief Inserta @p object en la posición @p pos de @p container,
 *        desplazando una posición a la derecha los elementos existentes
 *        desde @c size-2 hasta @p pos.
 *
 * @note Asume que @p container ya tiene reservado espacio para el nuevo
 *       elemento (no realiza push_back/resize).
 *
 * @tparam Container Tipo indexable con tamaño fijo/reservado (p. ej. std::vector).
 * @tparam ObjType Tipo del elemento a insertar.
 * @param container Contenedor donde insertar.
 * @param object Elemento a insertar.
 * @param pos Posición destino del nuevo elemento.
 */
template <typename Container, typename ObjType>
void insert_at(Container &container, const ObjType &object, Count pos)
{
        Count size = container.size();
        for (Count i = size - 2; i >= pos; i--)
                container[i + 1] = container[i];
        container[pos] = object;
}

/**
 * @brief Elimina lógicamente el elemento en la posición @p pos de
 *        @p container, desplazando una posición a la izquierda los
 *        elementos posteriores.
 *
 * @note No reduce el tamaño del contenedor; el llamador es responsable
 *       de llevar la cuenta de elementos válidos (p. ej. m_KeyCount).
 *
 * @tparam Container Tipo indexable (p. ej. std::vector).
 * @param container Contenedor del que se elimina.
 * @param pos Posición del elemento a eliminar.
 */
template <typename Container>
void remove(Container &container, Count pos)
{
        Count size = container.size();
        for (Count i = pos + 1; i < size; i++)
                container[i - 1] = container[i];
}

template <typename Traits>
class BTree;

enum class BTreeTraversalDirection;
template <typename, BTreeTraversalDirection> class BTreeIterator;

/*template <typename keyType>
bool operator>=(const _Node<keyType>& object1, const _Node<keyType>& object2)
{ return object1.key >= object2.key;    }

template <typename keyType>
bool operator<=(const _Node<keyType>& object1, const _Node<keyType>& object2)
{ return object1.key <= object2.key;    }*/

/**
 * @brief Nodo/clave almacenado en una página del B-Tree.
 *
 * Empareja una clave de búsqueda (@c key) con el identificador de objeto
 * (@c ObjID) al que referencia, y lleva un contador de accesos (@c UseCounter)
 * incrementado en cada Search exitoso.
 *
 * @tparam keyType Tipo de la clave de búsqueda.
 * @tparam ObjIDType Tipo del identificador de objeto asociado.
 */
template <typename keyType, typename ObjIDType>
struct tagNode
{
        keyType key;         ///< Clave de búsqueda.
        ObjIDType ObjID;      ///< Identificador del objeto asociado a la clave.
        Counter UseCounter;   ///< Número de veces que esta clave fue encontrada en una búsqueda.

        /// @brief Construye un nodo con clave y ObjID dados; UseCounter inicia en 0.
        tagNode(const keyType &_key, ObjIDType _ObjID)
            : key(_key), ObjID(_ObjID), UseCounter(0) {}
        /// @brief Constructor por defecto (miembros sin inicializar salvo los que tengan valor por defecto).
        tagNode() {}

        /// @brief Conversión implícita a @c keyType, permite comparar/ordenar nodos por su clave.
        operator keyType() { return key; }
        /// @brief Retorna el número de veces que la clave fue encontrada en Search.
        Counter GetUseCounter() { return UseCounter; }
};

/**
 * @brief Traits base para BTree / CBTreePage.
 *
 * Define los alias @c key_type (tipo de clave) y @c obj_type (tipo de
 * identificador de objeto) requeridos por BTree y CBTreePage.
 *
 * @tparam keyType Tipo de la clave de búsqueda.
 * @tparam ObjIDType Tipo del identificador de objeto (por defecto KeyRef).
 */
template <typename keyType, typename ObjIDType = KeyRef>
struct BTreePageTrait : BaseContainerTrait<keyType, tagNode<keyType, ObjIDType>>
{
        using key_type = keyType;
        using obj_type = ObjIDType;
};

/**
 * @brief Variante de BTreePageTrait con comparador ascendente (@c std::less).
 *
 * @note El comparador @c Comp está definido pero actualmente no está
 *       conectado a binary_search (que usa operator> / operator<= directos).
 */
template <typename keyType, typename ObjIDType = KeyRef>
struct AscendingBTreeTrait : BTreePageTrait<keyType, ObjIDType>
{
        using Comp = less<keyType>;
};

/**
 * @brief Variante de BTreePageTrait con comparador descendente (@c std::greater).
 *
 * @note El comparador @c Comp está definido pero actualmente no está
 *       conectado a binary_search (que usa operator> / operator<= directos).
 */
template <typename keyType, typename ObjIDType = KeyRef>
struct DescendingBTreeTrait : BTreePageTrait<keyType, ObjIDType>
{
        using Comp = greater<keyType>;
};


/**
 * @brief Página en memoria de un B-Tree: nodo interno del árbol que
 *        almacena hasta @c m_MaxKeys claves y @c m_MaxKeys+1 punteros a
 *        subpáginas.
 *
 * Implementa los algoritmos centrales del B-Tree: inserción con propagación
 * de overflow (SplitChild), eliminación con propagación de underflow
 * (Redistribute1/2, RedistributeR2L/L2R, Merge/MergeRoot), búsqueda binaria
 * dentro de la página y recorridos in-order (Traverse/ForEach/FirstThat).
 *
 * @tparam Traits Struct de traits que define @c key_type y @c obj_type.
 */
template <typename Traits>
class CBTreePage
// this is the in-memory version of the CBTreePage
{
        using key_type = typename Traits::key_type;   ///< Tipo de la clave de búsqueda.
        using ObjIDType = typename Traits::obj_type;   ///< Tipo del identificador de objeto asociado.

        friend class BTree<Traits>;
        template <typename, BTreeTraversalDirection> friend class BTreeIterator;
        using BTPage = CBTreePage<Traits>; ///< Alias del propio tipo de página.

public:
        using Node = tagNode<key_type, ObjIDType>; ///< Tipo de nodo/clave almacenado en la página.

        /**
         * @brief Construye una página vacía.
         * @param maxKeys Capacidad máxima de claves de esta página.
         * @param unique  Si es @c true, no se permiten claves duplicadas.
         */
        CBTreePage(Capacity maxKeys, bool unique = true);
        /// @brief Libera recursivamente todas las subpáginas (ver Reset()).
        virtual ~CBTreePage();

        /**
         * @brief Inserta @p key/@p ObjID en el subárbol enraizado en esta página.
         *
         * Si la página es hoja, inserta directamente; si no, recurre por el
         * hijo correspondiente y, ante un overflow del hijo, intenta
         * redistribuir con un hermano (Redistribute1) o divide el hijo
         * (SplitChild). Puede propagar bt_overflow hacia el llamador.
         *
         * @param key Clave a insertar.
         * @param ObjID Identificador de objeto asociado.
         * @return bt_duplicate si la clave ya existe y unique está activo;
         *         bt_overflow si esta página quedó por encima de su capacidad;
         *         bt_ok en cualquier otro caso.
         */
        bt_ErrorCode Insert(const key_type &key, ObjIDType ObjID);
        
        /**
         * @brief Elimina @p key del subárbol enraizado en esta página.
         *
         * Distingue cuatro casos clásicos de eliminación en B-Trees: clave en
         * una hoja, clave en un nodo interno (se sustituye por el sucesor vía
         * GetFirstNode), underflow tratable por redistribución
         * (TreatUnderflow), y underflow que requiere fusión (Merge/MergeRoot).
         *
         * @param key Clave a eliminar.
         * @param ObjID Identificador de objeto (reservado para uso futuro).
         * @return bt_nofound si la clave no existe; bt_rootmerged si esta
         *         página (raíz) fue fusionada con sus hijos; bt_ok en otro caso.
         */
        bt_ErrorCode Remove(const key_type &key, ObjIDType ObjID);
        
        /**
         * @brief Busca @p key en el subárbol enraizado en esta página.
         * @param key Clave a buscar.
         * @param[out] ObjID Se establece con el ObjID asociado si se encuentra.
         * @return @c true si la clave fue encontrada; @c false en otro caso.
         */
        bool Search(const key_type &key, ObjIDType &ObjID);
        
        /**
         * @brief Imprime el subárbol en orden in-order, indentando cada clave
         *        según su nivel de profundidad.
         * @param os Stream de salida.
         */
        void Print(ostream &os);

        /**
         * @brief Recorrido in-order recursivo del subárbol, deteniéndose en
         *        el primer nodo para el que @p func retorna @c true.
         *
         * Es la base común de ForEach() (predicado siempre falso) y
         * FirstThat() (predicado real del usuario).
         *
         * @tparam Func Callable con firma (Node&, Count level, Args...) -> bool.
         * @tparam Args Tipos de los argumentos extra reenviados a @p func.
         * @param func Callable/predicado invocado por cada nodo visitado.
         * @param level Nivel de profundidad de esta página (para indentación/contexto).
         * @param args Argumentos adicionales reenviados a @p func.
         * @return Puntero al nodo donde @p func retornó @c true, o @c nullptr
         *         si se recorrió todo el subárbol sin coincidencias.
         */
        template <typename Func, typename... Args>
        Node* Traverse(Func func, Count level, Args&&... args)
        {
                for (Count i = 0; i < m_KeyCount; i++)
                {
                        if (m_SubPages[i]) {
                                Node* result = m_SubPages[i]->Traverse(func, level + 1, forward<Args>(args)...);
                                if (result) return result;
                        }
                        if (func(m_Keys[i], level, forward<Args>(args)...))
                                return &m_Keys[i];
                }
                if (m_SubPages[m_KeyCount])
                        return m_SubPages[m_KeyCount]->Traverse(func, level + 1, forward<Args>(args)...);
                return nullptr;
        }

        // ForEach variadic template
        /**
         * @brief Aplica @p func a cada nodo del subárbol en orden in-order.
         *
         * Envuelve @p func en un lambda que siempre retorna @c false, de
         * modo que Traverse() nunca se detiene anticipadamente y visita
         * todos los nodos. El lambda además absorbe el parámetro @c level
         * vía @c auto&... antes de invocar la @p func original.
         *
         * @tparam Func Callable con firma (Node&, Count level, Args...).
         * @tparam Args Tipos de los argumentos extra reenviados a @p func.
         * @param func Callable invocado por cada nodo visitado.
         * @param level Nivel de profundidad inicial (normalmente 0).
         * @param args Argumentos adicionales reenviados a @p func.
         */
        template <typename Func, typename... Args>
        void ForEach(Func func, Count level, Args&&... args)
        {
                Traverse([&func](Node& node, Count lvl, auto&... a) { func(node, lvl, a...); return false; }, level, forward<Args>(args)...);
        }

        // FirstThat variadic template
        /**
         * @brief Busca el primer nodo del subárbol (en orden in-order) para
         *        el cual @p func retorna @c true.
         *
         * @tparam Func Callable/predicado con firma (Node&, Count level, Args...) -> bool.
         * @tparam Args Tipos de los argumentos extra reenviados a @p func.
         * @param func Predicado invocado por cada nodo visitado.
         * @param level Nivel de profundidad inicial (normalmente 0).
         * @param args Argumentos adicionales reenviados a @p func.
         * @return Puntero al primer nodo que satisface @p func, o @c nullptr si ninguno lo hace.
         */
        template <typename Func, typename... Args>
        Node *FirstThat(Func func, Count level, Args&&... args)
        {
                return Traverse(func, level, forward<Args>(args)...);
        }

protected:
        Capacity m_MinKeys;          ///< Número mínimo de claves permitido (calculado en Create()).
        Capacity m_MaxKeys;          ///< Número máximo de claves permitido en esta página.
        Capacity m_MaxKeysForChilds; ///< Capacidad máxima con la que se crean las páginas hijas; distinta de m_MaxKeys solo en la raíz.
        bool m_Unique;               ///< Si es @c true, no se permiten claves duplicadas.
        bool m_isRoot;               ///< Reservado; no usado actualmente (ver IsRoot()).
        vector<Node> m_Keys;         ///< Claves almacenadas en esta página.
        vector<BTPage *> m_SubPages; ///< Punteros a subpáginas; m_SubPages[i] agrupa claves menores que m_Keys[i].
        Count m_KeyCount;            ///< Número de claves válidas actualmente en m_Keys.

        /// @brief Reserva el almacenamiento de m_Keys/m_SubPages y calcula m_MinKeys.
        void Create();
        /// @brief Libera recursivamente las subpáginas y vacía esta página.
        void Reset();
        /// @brief Libera esta página (Reset() + delete this).
        void Destroy()
        {
                Reset();
                delete this;
        }
        /// @brief Vacía lógicamente la página (pone m_KeyCount a 0) sin liberar memoria.
        void clear() { m_KeyCount = 0; }

        /**
         * @brief Primer intento de resolver un underflow/overflow del hijo en
         *        @p pos redistribuyendo claves con un hermano adyacente.
         * @param[in,out] pos Índice del hijo problemático; puede ajustarse si
         *                    la redistribución no es posible.
         * @return @c true si se logró redistribuir; @c false si se requiere Merge/SplitChild.
         */
        bool Redistribute1(Count &pos);
        /**
         * @brief Segundo intento de resolver un underflow considerando ambos
         *        hermanos de la página en @p pos (rotaciones combinadas).
         * @param pos Índice del hijo (o página) problemático.
         * @return @c true si se logró redistribuir; @c false si es necesario fusionar (Merge).
         */
        bool Redistribute2(Count pos);
        /// @brief Mueve claves del hijo derecho (pos) hacia el izquierdo (pos-1) a través de esta página.
        void RedistributeR2L(Count pos);
        /// @brief Mueve claves del hijo izquierdo (pos) hacia el derecho (pos+1) a través de esta página.
        void RedistributeL2R(Count pos);

        /// @brief Intenta resolver un underflow probando primero Redistribute1 y luego Redistribute2.
        bool TreatUnderflow(Count &pos) { return Redistribute1(pos) || Redistribute2(pos); }

        /**
         * @brief Fusiona los hijos en pos-1, pos y pos+1 (junto con las dos
         *        claves separadoras de esta página) en dos páginas resultantes.
         * @param pos Índice de la clave/página central a fusionar.
         * @return bt_underflow si tras la fusión esta página quedó por debajo
         *         del mínimo de claves; bt_ok en otro caso.
         */
        bt_ErrorCode Merge(Count pos);
        
        /**
         * @brief Caso especial de Merge() cuando esta página es la raíz y
         *        solo le quedan 2 claves: fusiona sus tres hijos en sí misma,
         *        reduciendo la altura del árbol.
         * @return Siempre bt_rootmerged.
         */
        bt_ErrorCode MergeRoot();

        /**
         * @brief Divide en tres el par de páginas hijas llenas adyacentes a
         *        @p pos (usando SplitPageInto3) y promueve dos claves nuevas
         *        a esta página.
         * @param pos Índice de la clave/página de referencia para decidir qué par dividir.
         */
        void SplitChild(Count pos);

        /// @brief Retorna la primera clave (menor) del subárbol enraizado en esta página.
        Node &GetFirstNode();

        /// @brief @c true si el número de claves excede m_MaxKeys.
        bool Overflow() { return m_KeyCount > m_MaxKeys; }
        /// @brief @c true si el número de claves es menor que MinNumberOfKeys().
        bool Underflow() { return m_KeyCount < MinNumberOfKeys(); }
        /// @brief @c true si la página está en (o por encima de) su capacidad máxima.
        bool IsFull() { return m_KeyCount >= m_MaxKeys; }
        /// @brief Número mínimo de claves permitido (2/3 de m_MaxKeys).
        Capacity MinNumberOfKeys() { return 2 * m_MaxKeys / 3.0; }
        /// @brief Número de celdas libres respecto a la capacidad máxima.
        Capacity GetFreeCells() { return m_MaxKeys - m_KeyCount; }
        /// @brief Referencia mutable al contador de claves (permite incrementar/decrementar in-place).
        Count &NumberOfKeys() { return m_KeyCount; }
        /// @brief Número actual de claves en esta página.
        Count GetNumberOfKeys() { return m_KeyCount; }
        /// @brief @c true si esta página es la raíz (su capacidad para hijos difiere de la propia).
        bool IsRoot() { return m_MaxKeysForChilds != m_MaxKeys; }
        /// @brief Establece la capacidad con la que se crearán las páginas hijas.
        void SetMaxKeysForChilds(Order orderforchilds) { m_MaxKeysForChilds = orderforchilds; }

        /// @brief Celdas libres en el hermano izquierdo de la subpágina en @p pos, o 0 si no existe.
        Capacity GetFreeCellsOnLeft(Count pos);
        /// @brief Celdas libres en el hermano derecho de la subpágina en @p pos, o 0 si no existe.
        Capacity GetFreeCellsOnRight(Count pos);

private:
        /**
         * @brief Divide esta página (cuando es la raíz y está en overflow) en
         *        tres páginas hijas nuevas, dejando en la raíz solo las dos
         *        claves promovidas.
         * @return Siempre @c true.
         */
        bool SplitRoot();

        /**
         * @brief Reparte el contenido combinado de dos páginas llenas
         *        (más una clave separadora) en tres páginas balanceadas,
         *        devolviendo las dos claves que deben promoverse al padre.
         *
         * @param tmpKeys Vector temporal con todas las claves a repartir.
         * @param tmpSubPages Vector temporal con todos los punteros a subpáginas a repartir.
         * @param[in,out] pChild1 Primera página resultante (se crea si es @c nullptr).
         * @param[in,out] pChild2 Segunda página resultante (se crea si es @c nullptr).
         * @param[out] pChild3 Tercera página resultante (siempre se crea).
         * @param[out] oi1 Primera clave promovida al padre.
         * @param[out] oi2 Segunda clave promovida al padre.
         */
        void SplitPageInto3(vector<Node> &tmpKeys, vector<BTPage *> &tmpSubPages,
                            BTPage *&pChild1, BTPage *&pChild2, BTPage *&pChild3,
                            Node &oi1, Node &oi2);
        
        /**
         * @brief Vacía @p pChildPage, acumulando todas sus claves y punteros
         *        a subpáginas (incluyendo el último puntero "sobrante") en
         *        @p tmpKeys / @p tmpSubPages.
         * @param pChildPage Página fuente a vaciar.
         * @param[out] tmpKeys Vector destino donde se acumulan las claves.
         * @param[out] tmpSubPages Vector destino donde se acumulan los punteros a subpáginas.
         */
        void MovePage(BTPage *pChildPage, vector<Node> &tmpKeys, vector<BTPage *> &tmpSubPages);
};

/// @brief Reserva espacio para @p maxKeys claves y, por defecto, iguala la capacidad de los hijos a la propia.
template <typename Traits>
CBTreePage<Traits>::CBTreePage(Capacity maxKeys, bool unique)
    : m_MaxKeys(maxKeys), m_Unique(unique), m_KeyCount(0)
{
        Create();
        SetMaxKeysForChilds(m_MaxKeys);
}

/// @brief Libera recursivamente todas las subpáginas.
template <typename Traits>
CBTreePage<Traits>::~CBTreePage() { Reset(); }

/**
 * @brief Inserta @p key en esta página (caso base) o recurre por el hijo
 *        correspondiente, resolviendo el overflow resultante con
 *        Redistribute1() o, si falla, con SplitChild().
 */
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

/**
 * @brief Resuelve el underflow/overflow del hijo en @p pos moviendo claves
 *        desde el hermano con más espacio disponible (o más claves, en el
 *        caso de underflow), eligiendo entre RedistributeL2R() y
 *        RedistributeR2L() según corresponda.
 */
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

/**
 * @brief Segundo intento de resolver un underflow, considerando ambos
 *        hermanos m_SubPages[pos-1] y m_SubPages[pos+1] simultáneamente
 *        mediante rotaciones dobles. Si falla, la única salida es fusionar (Merge).
 */
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

/**
 * @brief Rota claves de derecha a izquierda: mueve la clave separadora
 *        de esta página hacia el hijo izquierdo (pos-1) y sube la clave
 *        más a la izquierda del hijo derecho (pos) para reemplazarla.
 *        Repite mientras el hijo derecho tenga excedente de claves.
 */
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

/**
 * @brief Rota claves de izquierda a derecha: mueve la clave separadora
 *        de esta página hacia el hijo derecho (pos+1) y sube la clave
 *        más a la derecha del hijo izquierdo (pos) para reemplazarla.
 *        Repite mientras el hijo izquierdo tenga excedente de claves.
 */
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

/**
 * @brief Localiza el par de páginas hijas llenas adyacentes a @p pos,
 *        combina su contenido (más la clave separadora de esta página)
 *        y lo reparte en tres páginas nuevas vía SplitPageInto3(),
 *        promoviendo dos claves a esta página.
 */
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

/**
 * @brief Reparte en tres el contenido combinado de @p tmpKeys/@p tmpSubPages
 *        en tercios aproximadamente iguales, creando las páginas hijas que
 *        no existan aún y extrayendo las dos claves centrales (@p oi1, @p oi2)
 *        que deben promoverse al padre.
 */
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

/**
 * @brief Divide el contenido completo de esta página (la raíz, en overflow)
 *        en tres páginas hijas nuevas vía SplitPageInto3(), dejando en la
 *        raíz únicamente las dos claves promovidas.
 */
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

/**
 * @brief Busca @p key en esta página mediante binary_search() y, si no
 *        está aquí, recurre por la subpágina correspondiente. Incrementa
 *        el UseCounter del nodo encontrado.
 */
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

/**
 * @brief Elimina @p key del subárbol enraizado en esta página, cubriendo
 *        los cuatro casos clásicos de eliminación en B-Trees:
 *         1. Clave encontrada en una hoja: se elimina directamente.
 *         2. Clave encontrada en un nodo interno: se sustituye por el
 *            sucesor (GetFirstNode() del hijo derecho) y se elimina
 *            recursivamente el sucesor.
 *         3. Underflow tras eliminar: se intenta resolver con TreatUnderflow().
 *         4. Underflow no resoluble por redistribución: se recurre a
 *            Merge() o, si esta página es la raíz con solo 2 claves, a MergeRoot().
 */
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

/**
 * @brief Fusiona los tres hijos adyacentes a @p pos (junto con las dos
 *        claves separadoras de esta página) en dos páginas resultantes,
 *        reutilizando pChild1 y pChild2 y destruyendo pChild3.
 */
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

/**
 * @brief Fusiona los tres hijos de la raíz (cuando esta solo tiene 2 claves)
 *        directamente dentro de la propia página raíz, reduciendo la
 *        altura del árbol en uno.
 */
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

/// @brief Desciende siempre por el hijo más a la izquierda hasta encontrar la clave más pequeña del subárbol.
template <typename Traits>
typename CBTreePage<Traits>::Node &
CBTreePage<Traits>::GetFirstNode()
{
        if (m_SubPages[0])
                return m_SubPages[0]->GetFirstNode();
        return m_Keys[0];
}

/// @brief Recorre el subárbol in-order (vía ForEach) imprimiendo cada clave con indentación proporcional a su nivel.
template <typename Traits>
void CBTreePage<Traits>::Print(ostream &os)
{
        ForEach([](Node &info, Count level, ostream &out)
                {
        for (Count i = 0; i < level; i++)
            out << "\t";
        out << info.key << "->" << info.ObjID << "\n"; }, 0, os);
}

/// @brief Reserva el almacenamiento de m_Keys (m_MaxKeys+1) y m_SubPages (m_MaxKeys+2), y calcula m_MinKeys.
template <typename Traits>
void CBTreePage<Traits>::Create()
{
        Reset();
        m_Keys.resize(m_MaxKeys + 1);
        m_SubPages.resize(m_MaxKeys + 2, nullptr);
        m_KeyCount = 0;
        m_MinKeys = 2 * m_MaxKeys / 3;
}

/// @brief Elimina recursivamente (delete) todas las subpáginas actuales y vacía esta página.
template <typename Traits>
void CBTreePage<Traits>::Reset()
{
        for (Count i = 0; i < m_KeyCount; i++)
                delete m_SubPages[i];
        clear();
}

/**
 * @brief Vacía @p pChildPage acumulando todas sus claves y punteros a
 *        subpáginas (incluyendo el puntero final) en @p tmpKeys / @p tmpSubPages,
 *        dejando @p pChildPage lista para ser reutilizada o destruida.
 */
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

/// @brief Celdas libres en el hermano izquierdo de la subpágina en @p pos, o 0 si no hay hermano izquierdo.
template <typename Traits>
Capacity CBTreePage<Traits>::GetFreeCellsOnLeft(Count pos)
{
        if (pos > 0) // there is some page on left ?
                return m_SubPages[pos - 1]->GetFreeCells();
        return 0;
}

/// @brief Celdas libres en el hermano derecho de la subpágina en @p pos, o 0 si no hay hermano derecho.
template <typename Traits>
Capacity CBTreePage<Traits>::GetFreeCellsOnRight(Count pos)
{
        if (pos < GetNumberOfKeys()) // there is some page on right ?
                return m_SubPages[pos + 1]->GetFreeCells();
        return 0;
}

#endif // CBTreePage_H