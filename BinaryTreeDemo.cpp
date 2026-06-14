#include <iostream>
#include <fstream>
#include "containers/binarytree.h"

using AscTree  = BinaryTree<AscendingBinaryTreeTrait<TI>>;
using DescTree = BinaryTree<DescendingBinaryTreeTrait<TI>>;

void DemoBinaryTree() {

    auto print  = [](auto* node) { cout << node->getData() << " "; };
    auto isEven = [](auto* node) { return node->getData() % 2 == 0; };

    // 1. Insertar
    cout << "=== Insertar ===" << endl;
    AscTree tree;
    tree.insert(5, 0);
    tree.insert(3, 1);
    tree.insert(7, 2);
    tree.insert(1, 3);
    tree.insert(4, 4);
    tree.insert(6, 5);
    tree.insert(8, 6);
    tree.insert(2, 7);
    tree.insert(9, 8);
    tree.insert(13, 9);
    tree.insert(21, 10);
    tree.insert(17, 11);
    tree.insert(7, 12);
    tree.insert(12, 13);

    // 2. Recorridos
    cout << "\n=== Recorridos ===" << endl;

    cout << "Inorder    forward  (asc): ";
    for (auto x : tree.ForEach()) print(x);
    cout << endl;

    cout << "Inorder    backward (desc): ";
    for (auto x : tree.ReverseForEach()) print(x);
    cout << endl;

    cout << "Preorder   forward : ";
    for (auto x : tree.PreorderForEach()) print(x);
    cout << endl;

    cout << "Preorder   backward : ";
    for (auto x : tree.ReversePreorderForEach()) print(x);
    cout << endl;

    cout << "Postorder  forward : ";
    for (auto x : tree.PostorderForEach()) print(x);
    cout << endl;

    cout << "Postorder  backward : ";
    for (auto x : tree.ReversePostorderForEach()) print(x);
    cout << endl;

    // 3. FirstThat usando Range Proxy
    cout << "\n=== FirstThat ===" << endl;
    auto first = tree.FirstThat(isEven);
        if (first.begin() != first.end())
            cout << "Primer par (inorder): " << (*first.begin())->getData()<< endl;

    auto reverse = tree.ReverseFirstThat(isEven);
        if (reverse.begin() != reverse.end())
            cout << "Primer par (reverse inorder): " << (*reverse.begin())->getData() << endl;

    // 4. Exportar a TXT
    cout << "\n=== Exportar a treelOut.txt ===" << endl;
    {
        ofstream file("treeOut.txt");
        file << tree;
    }

    // Mostrar el contenido del archivo para entender el formato
    cout << "Contenido de treeOut.txt:" << endl;
    {
        ifstream file("treeOut.txt");
        cout << file.rdbuf();
    }

    // 5. Leer desde TXT
    cout << "\n=== Leer desde tree.txt ===" << endl;
    AscTree tree2;
    {
        ifstream file("treeOut.txt");
        file >> tree2;
    }

    cout << "Inorder del árbol leído: ";
    for (auto x : tree2.ForEach()) print(x);
    cout << endl;

    // 6. Copy constructor
    cout << "\n=== Copy constructor ===" << endl;
    AscTree tree3 = tree;
    tree3.insert(9, 7);

    cout << "Original : ";
    for (auto x : tree.ForEach()) print(x);
    cout << endl;
    cout << "Copia    : ";
    for (auto x : tree3.ForEach()) print(x);
    cout << endl;

    // 7. Move constructor
    cout << "\n=== Move constructor ===" << endl;
    AscTree tree4 = std::move(tree3);
    cout << "Movido   : ";
    for (auto x : tree4.ForEach()) print(x);
    cout << endl;
    cout << "Vaciado  : ";
    for (auto x : tree3.ForEach()) print(x);
    cout << endl;

    // 8. Árbol descendente
    cout << "\n=== Árbol descendente ===" << endl;
    DescTree desc;
    desc.insert(5, 0);
    desc.insert(3, 1);
    desc.insert(7, 2);
    desc.insert(1, 3);
    desc.insert(8, 4);
    desc.insert(6, 5);
    desc.insert(4, 6);
    desc.insert(2, 7);

    cout << "Inorder desc: ";
    for (auto x : desc.ForEach()) print(x);
    cout << endl;

    cout << "Operador <<: " << endl;
    cout << desc;

    cout << "ToString: " << endl;
    cout << desc.ToString();
}
