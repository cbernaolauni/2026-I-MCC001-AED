#include <iostream>
#include <fstream>
#include "containers/binarytree.h"

using AscTree  = BinaryTree<AscendingBinaryTreeTrait<TI>>;
using DescTree = BinaryTree<DescendingBinaryTreeTrait<TI>>;

void DemoBinaryTree() {

    auto print = [](TI& val) { cout << val << " "; };
    auto isEven = [](TI& val) { return val % 2 == 0; };

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
    tree.ForEach(print);
    cout << endl;

    cout << "Inorder    backward (desc): ";
    tree.ReverseForEach(print);
    cout << endl;

    cout << "Preorder   forward : ";
    tree.PreorderForEach(print);
    cout << endl;

    cout << "Preorder   backward : ";
    tree.ReversePreorderForEach(print);
    cout << endl;

    cout << "Postorder  forward : ";
    tree.PostorderForEach(print);
    cout << endl;

    cout << "Postorder  backward : ";
    tree.ReversePostorderForEach(print);
    cout << endl;

    // 3. FirstThat
    cout << "\n=== FirstThat ===" << endl;

    auto it = tree.FirstThat(isEven);
    if (it != tree.inorder_end())
        cout << "Primer par (inorder): " << *it << endl;

    auto rit = tree.ReverseFirstThat(isEven);
    if (rit != tree.rinorder_end()) 
        cout << "Primer par (reverse inorder): " << *rit << endl;

    // 4. Exportar a TXT
    cout << "\n=== Exportar a treeOut.txt ===" << endl;
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
        ifstream file("tree.txt");
        file >> tree2;
    }

    cout << "Inorder del árbol leído: ";
    tree2.ForEach(print);
    cout << endl;

    // 6. Copy constructor
    cout << "\n=== Copy constructor ===" << endl;
    AscTree tree3 = tree;
    tree3.insert(9, 7);

    cout << "Original : ";  tree.ForEach(print);  cout << endl;
    cout << "Copia    : ";  tree3.ForEach(print); cout << endl;

    // 7. Move constructor
    cout << "\n=== Move constructor ===" << endl;
    AscTree tree4 = std::move(tree3);
    cout << "Movido   : ";  tree4.ForEach(print); cout << endl;
    cout << "Vaciado  : ";  tree3.ForEach(print); cout << endl;

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
    desc.ForEach(print);
    cout << endl;

    cout << "Operador <<: " << endl;
    cout << desc;

    cout << "ToString: " << endl;
    cout << desc.ToString();
}
