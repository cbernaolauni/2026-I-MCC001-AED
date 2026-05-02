#include <thread>
#include <mutex>
#include "linkedlist.h"

template <typename T>
bool IsMultipleOf(LLNode<T> &node, T x){
    return node.getDataRef() % x == 0;
}

template <typename T>
void AddOne(LLNode<T> &node){    
    static mutex mtx;
    scoped_lock<mutex> lock(mtx);
    ++node;
}

template <typename T>
void Multiplicate(LLNode<T> &node, T x){    
    static mutex mtx;
    scoped_lock<mutex> lock(mtx);
    node*=x;
}

void LinkedListDemo(){
    // 
    LinkedList<DescendingLinkedListTrait<TI>> list1;
    list1.insert(6, 15);
    list1.insert(2, 25);
    list1.insert(9, 35);
    list1.insert(1, 45);
    list1.insert(7, 55);
    cout << "Lista1 descendente: " << list1 << endl;

    LinkedList<AscendingLinkedListTrait<TI>> list2;
    list2.insert(6, 15);
    list2.insert(2, 25);
    list2.insert(9, 35);
    list2.insert(1, 45);
    list2.insert(7, 55);
    cout << "Lista2 ascendente : " << list2 << endl;

    LinkedList<AscendingLinkedListTrait<TI>> list3;
    list3.push_front(6, 15);
    cout << "Lista3 sin orden : " << list3 << endl;
    list3.push_front(2, 25);
    cout << "Lista3 sin orden : " << list3 << endl;
    list3.pop_front();
    cout << "Lista3 sin orden : " << list3 << endl;
    list3.pop_front();
    cout << "Lista3 sin orden : " << list3 << endl;
    list3.push_front(9, 35);
    cout << "Lista3 sin orden : " << list3 << endl;
    list3.push_back(1, 45);
    cout << "Lista3 sin orden : " << list3 << endl;
    list3.push_back(7, 55);
    cout << "Lista3 sin orden : " << list3 << endl;
    list3.pop_back();
    cout << "Lista3 sin orden : " << list3 << endl;

    LinkedList<AscendingLinkedListTrait<TI>> list4 = list2; // Copy constructor
    cout << "Lista 4 (copia de lista 2): " << list4 << endl;

    list4.insert(8, 65);
    list4.insert(3, 75);
    cout << "Lista 4 (despues de insertar 2 valores): " << list4 << endl;
    cout << "Lista 2 (deberia ser igual a la original): " << list2 << endl;

    LinkedList<DescendingLinkedListTrait<TI>> list5 = std::move(list1); // Move constructor
    cout << "Lista 5 (move de lista 1): " << list5 << endl;
    cout << "Lista 1 (deberia estar vacia): " << list1 << endl;

    cout << "Nodo en indice 2 de lista 4: " << list4[2] << endl;
    cout << "Nodo en indice 0 de lista 2: " << list2[0] << endl;
    cout << "Nodo en indice 1 de lista 3: " << list3[1] << endl;

    cout << "Ingresa valor y ref lista 2: ";
    cin >> list2;
    cout << list2 << endl;

    cout << "Ingresa valores y refs lista 3: ";
    cin >> list3;
    cin >> list3;
    cout << list3 << endl;

    auto it = list4.FirstThat(IsMultipleOf<TI>, 4);
    if (it != list4.end())
        cout << "Primer multiplo de 4 de la lista 4: " << *it << endl;

    cout << "Iniciando prueba con threads..." << endl;
    cout << "Lista 3 antes de threads: " << list3 << endl;

    auto worker = [&list3](int thread_id){
        for(int i = 0; i < 4; i++)
            list3.ForEach(Multiplicate<TI>, 2);
        cout << "Thread " << thread_id << " terminado\n";
    };

    thread t1(worker, 1);
    thread t2(worker, 2);
    thread t3(worker, 3);

    t1.join(); t2.join(); t3.join();
    cout << "Resultado (esperado): " << list3 << endl;    
}

void ListsDemo(){
    LinkedListDemo();
    
}
