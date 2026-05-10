#include "containers/doublelinkedlist.h"
#include "containers/circularlinkedlist.h"
#include "containers/doublecircularlinkedlist.h"
#include <fstream>

template <typename Node>
void Print(Node &node, ostream& os){
    os << node << ",";
}

template <typename Node, typename T>
bool IsGreaterThan(Node &node, T x){
    return node.getDataRef() > x;
}

void DoubleListDemo(){
    cout << "=== Demo DoubleLinkedList ===" << endl;

    // --- 1. push_back ---
    cout << "\n[1] push_back" << endl;
    DoubleLinkedList<AscendingDoubleLinkedListTrait<TI>> dll;
    dll.push_back(10, 0);
    dll.push_back(20, 1);
    dll.push_back(30, 2);
    cout << "Lista: " << dll << endl;

    // --- 2. insert ordenado ---
    cout << "\n[2] insert (ordenado ascendente)" << endl;
    DoubleLinkedList<AscendingDoubleLinkedListTrait<TI>> dll2;
    dll2.insert(30, 0);
    dll2.insert(10, 1);
    dll2.insert(20, 2);
    cout << "Lista: " << dll2 << endl;

    // --- 3. ForEach forward ---
    cout << "\n[3] ForEach (forward)" << endl;
    using DLL2Node = DoubleLinkedList<AscendingDoubleLinkedListTrait<TI>>::Node;
    dll2.ForEach(Print<DLL2Node>, cout);
    cout << endl;

    // --- 4. ForEach backward ---
    cout << "\n[4] ForEach (backward)" << endl;
    dll2.ReverseForEach(Print<DLL2Node>, cout);
    cout << endl;

    // --- 5. FirstThat ---
    cout << "\n[5] FirstThat (buscar > 15)" << endl;
    auto it = dll2.FirstThat(IsGreaterThan<DLL2Node, TI>, 15);
    if (it != dll2.end())
        cout << "Primer mayor a 15   : " << *it << endl;
    else   cout << "No se encontró elemento mayor a 15" << endl;

    // --- 6. ReverseFirstThat ---
    cout << "\n[6] ReverseFirstThat (buscar > 15)" << endl;
    auto rit = dll2.ReverseFirstThat(IsGreaterThan<DLL2Node, TI>, 15);
    if (rit != dll2.rend())
        cout << "Último mayor a 15   : " << *rit << endl;
    else
        cout << "No se encontró elemento mayor a 15" << endl;

    // --- 7. pop_back ---
    cout << "\n[7] pop_back" << endl;
    auto [val, ref] = dll.pop_back();
    cout << "Extraido: (" << val << ", " << ref << ")" << endl;
    cout << "Lista: " << dll << endl;

    // --- 8. Copy constructor ---
    cout << "\n[8] Copy constructor" << endl;
    DoubleLinkedList<AscendingDoubleLinkedListTrait<TI>> dll3(dll);
    cout << "Copia: " << dll3 << endl;

    // --- 9. Move constructor ---
    cout << "\n[9] Move constructor" << endl;
    DoubleLinkedList<AscendingDoubleLinkedListTrait<TI>> dll4(std::move(dll3));
    cout << "Movido: " << dll4 << endl;
    cout << "Original tras move: " << dll3 << endl;

    // --- 10. operator>> ---
    cout << "\n[10] operator>>" << endl;
    DoubleLinkedList<AscendingDoubleLinkedListTrait<TI>> dll5;
    istringstream input("[(10, 0),(20, 1),(30, 2), (40, 3)]");
    input >> dll5;
    cout << "Leido: " << dll5 << endl;

    // --- 11. isEmpty ---
    cout << "\n[11] isEmpty" << endl;
    DoubleLinkedList<AscendingDoubleLinkedListTrait<TI>> dll6;
    cout << "dll6 vacia: " << (dll6.isEmpty() ? "si" : "no") << endl;
    dll6.push_back(1, 0);
    cout << "dll6 vacia tras push_back: " << (dll6.isEmpty() ? "si" : "no") << endl;
}

void CircularListDemo(){
    cout << "=== Demo CircularLinkedList ===" << endl;

    CircularLinkedList<AscendingCircularLinkedListTrait<TI>> cll;
    cll.push_back(10, 0);
    cll.push_back(20, 1);
    cll.push_back(30, 2);
    cout << "Lista circular: " << cll << endl;

    cll.pop_front();
    cout << "Lista tras pop_front: " << cll << endl;

    cll.pop_back();
    cout << "Lista tras pop_back: " << cll << endl;

    cout << "\n[3] ForEach (forward)" << endl;
    ifstream file("lista1.txt");
    CircularLinkedList<AscendingCircularLinkedListTrait<TI>> cll2;
    file >> cll2;
    cout << "Lista cll2: " << cll2 << endl;
    using LLNode = CircularLinkedList<AscendingCircularLinkedListTrait<TI>>::Node;
    cll2.insert(6, 15);
    cll2.insert(2, 25);
    cll2.insert(9, 35);
    cll2.insert(1, 45);
    cll2.insert(7, 55);
    cll2.ForEach(Print<LLNode>, cout);
    cout << endl;

    cout << "\n[4] ForEach (backward)" << endl;
    cll2.ReverseForEach(Print<LLNode>, cout);
    cout << endl;

    cout << "\n[5] FirstThat" << endl;
    auto it = cll2.FirstThat(IsGreaterThan<LLNode, TI>, 6);
    if (it != cll2.end())
        cout << "Primer mayor a 6   : " << *it << endl;
    else
        cout << "No se encontró elemento mayor a 6" << endl;

    cout << "\n[6] ReverseFirstThat" << endl;
    auto rit = cll2.ReverseFirstThat(IsGreaterThan<LLNode, TI>, 6);
    if (rit != cll2.rend())
        cout << "Último mayor a 6   : " << *rit << endl;
    else
        cout << "No se encontró elemento mayor a 6" << endl;
}

void CircularDoubleListDemo (){
    cout << "=== Demo CircularDoubleLinkedList ===" << endl;

    // --- 1. push_back ---
    cout << "\n[1] push_back" << endl;
    CircularDoubleLinkedList<AscendingCircularDoubleLinkedListTrait<TI>> cdll;
    cdll.push_back(10, 0);
    cdll.push_back(20, 1);
    cdll.push_back(30, 2);
    cout << "Lista: " << cdll << endl;

    // --- 2. push_front ---
    cout << "\n[2] push_front" << endl;
    cdll.push_front(5, 3);
    cout << "Lista: " << cdll << endl;

    // --- 3. insert ordenado ---
    cout << "\n[3] insert (ordenado ascendente)" << endl;
    CircularDoubleLinkedList<AscendingCircularDoubleLinkedListTrait<TI>> cdll2;
    cdll2.insert(30, 0);
    cdll2.insert(5,  3);
    cdll2.insert(25, 4);
    cout << "Lista: " << cdll2 << endl;

    // --- 4. ForEach forward ---
    cout << "\n[4] ForEach (forward)" << endl;
    using CDLLNode = CircularDoubleLinkedList<AscendingCircularDoubleLinkedListTrait<TI>>::Node;
    cdll2.ForEach(Print<CDLLNode>, cout);
    cout << endl;

    // --- 5. ReverseForEach backward ---
    cout << "\n[5] ReverseForEach (backward)" << endl;
    cdll2.ReverseForEach(Print<CDLLNode>, cout);
    cout << endl;

    // --- 6. FirstThat ---
    cout << "\n[6] FirstThat" << endl;
    auto it = cdll2.FirstThat(IsGreaterThan<CDLLNode, TI>, 6);
    if (it != cdll2.end())
        cout << "Primer mayor a 6   : " << *it << endl;
    
    // --- 7. ReverseFirstThat ---
    cout << "\n[7] ReverseFirstThat" << endl;
    auto rit = cdll2.ReverseFirstThat(IsGreaterThan<CDLLNode, TI>, 6);
    if (rit != cdll2.rend())
        cout << "Ultimo mayor a 6   : " << *rit << endl;

    // --- 8. pop_back ---
    cout << "\n[8] pop_back" << endl;
    cdll.pop_back();
    cout << "Lista: " << cdll << endl;

    // --- 9. pop_front ---
    cout << "\n[9] pop_front" << endl;
    cdll.pop_front();
    cout << "Lista: " << cdll << endl;

    // --- 10. Copy constructor ---
    cout << "\n[10] Copy constructor" << endl;
    CircularDoubleLinkedList<AscendingCircularDoubleLinkedListTrait<TI>> cdll3(cdll2);
    cout << "Copia: " << cdll3 << endl;

    // --- 11. Move constructor ---
    cout << "\n[11] Move constructor" << endl;
    CircularDoubleLinkedList<AscendingCircularDoubleLinkedListTrait<TI>> cdll4(move(cdll3));
    cout << "Movido: " << cdll4 << endl;
    cout << "Original tras move: " << cdll3 << endl;

    // --- 12. operator>> ---
    cout << "\n[12] operator>>" << endl;
    CircularDoubleLinkedList<AscendingCircularDoubleLinkedListTrait<TI>> cdll5;
    istringstream input("[(5, 0),(10, 1),(20, 2),(25, 3),(30, 4)]");
    input >> cdll5;
    cout << "Leido: " << cdll5 << endl;

    // --- 13. isEmpty ---
    cout << "\n[13] isEmpty" << endl;
    CircularDoubleLinkedList<AscendingCircularDoubleLinkedListTrait<TI>> cdll6;
    cout << "cdll6 vacia: " << (cdll6.isEmpty() ? "si" : "no") << endl;
    cdll6.push_back(1, 0);
    cout << "cdll6 tras push_back: " << (cdll6.isEmpty() ? "si" : "no") << endl;

    // --- 14. Descending ---
    cout << "\n[14] insert (ordenado descendente)" << endl;
    CircularDoubleLinkedList<DescendingCircularDoubleLinkedListTrait<TI>> cdll7;
    cdll7.insert(30, 0);
    cdll7.insert(10, 1);
    cdll7.insert(20, 2);
    cout << "Lista descendente: " << cdll7 << endl;
}