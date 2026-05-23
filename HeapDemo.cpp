#include "containers/heap.h"
#include <fstream>

void HeapDemo(){
    cout << "=== Demo Heap ===\n\n";

    cout << "-- Min Heap (insert manual) --\n";
    Heap<AscendingHeapTrait<TI>> minHeap;
    minHeap.insert(10, 0);
    minHeap.insert(3,  1);
    minHeap.insert(7,  2);
    minHeap.insert(1,  3);
    minHeap.insert(5,  4);
    cout << "Heap: " << minHeap << "\n";
    cout << "Peek: " << minHeap.peek() << "\n\n";

    cout << "-- Extract --\n";
    minHeap.extract();
    cout << "Tras extract: " << minHeap << "\n";
    cout << "Nuevo peek:   " << minHeap.peek() << "\n\n";

    cout << "-- operator>> --\n";
    Heap<AscendingHeapTrait<TI>> h1, h2;
    istringstream ss2("[5, 2, 8, 1, 9, 13, 15, 4, 27]");   // con espacios
    ifstream file("lista1.txt");
    file >> h1;
    ss2 >> h2;

    cout << "Sin espacios: " << h1 << "\n";
    cout << "Con espacios: " << h2 << "\n\n";

    cout << "-- Constructor copia --\n";
    Heap<AscendingHeapTrait<TI>> hCopy(h1);
    cout << "Original: " << h1    << "\n";
    cout << "Copia:    " << hCopy << "\n\n";

    cout << "-- Constructor move --\n";
    Heap<AscendingHeapTrait<TI>> hMove = (std::move(hCopy));
    cout << "Movido:       " << hMove << "\n";
    cout << "Fuente vacia: " << hCopy << "\n\n";  // debe ser []


    cout << "-- operator<< --"<<endl;
    ofstream ofs;
    ofs.open("lista2.txt");
    ofs <<  hMove <<   endl;
    ofs.close();

    cout << "-- Max Heap --\n";
    Heap<DescendingHeapTrait<TI>> maxHeap;
    istringstream ss3("[5,2,8,1,9]");
    ss3 >> maxHeap;
    cout << "Max Heap: " << maxHeap << "\n";
    cout << "Peek max: " << maxHeap.peek() << "\n\n";

    maxHeap.insert(13, 5);
    maxHeap.insert(3, 6);
    maxHeap.insert(17, 7);
    cout << "Max Heap actualizado: " << maxHeap << "\n";

    cout << "-- Formato invalido --\n";
    Heap<AscendingHeapTrait<TI>> hBad;
    istringstream ssBad("1,2,3]");   // falta el '['
    ssBad >> hBad;
    cout << "Stream valido: " << (ssBad ? "si" : "no") << "\n";
    cout << "Heap:          " << hBad << "\n";
}