#include <iostream>
#include <fstream>
#include "containers/binarytree.h"

using IntAscTree  = BinaryTree<AscendingBinaryTreeTrait<TI>>;
using IntDescTree = BinaryTree<DescendingBinaryTreeTrait<TI>>;

void DemoBinaryTree() {

    // 1. Insertar
    cout << "=== Insertar ===" << endl;
    IntAscTree tree;
    tree.insert(5, 0);
    tree.insert(3, 1);
    tree.insert(7, 2);
    tree.insert(1, 3);
    tree.insert(4, 4);
    tree.insert(6, 5);
    tree.insert(8, 6);

    // 2. Recorridos
    cout << "\n=== Recorridos ===" << endl;

    auto print = [](TI& val) { cout << val << " "; };

    cout << "Inorder    forward  (asc): ";
    tree.ForEach(print);                        // 1 3 4 5 6 7 8
    cout << endl;

    cout << "Inorder    backward (desc): ";
    tree.ReverseForEach(print);                 // 8 7 6 5 4 3 1
    cout << endl;

    cout << "Preorder   forward : ";
    tree.PreorderForEach(print);                // 5 3 1 4 7 6 8
    cout << endl;

    cout << "Postorder  forward : ";
    tree.PostorderForEach(print);               // 1 4 3 6 8 7 5
    cout << endl;

    // 3. FirstThat
    cout << "\n=== FirstThat ===" << endl;
    auto isEven = [](TI& val) { return val % 2 == 0; };

    auto it = tree.FirstThat(isEven);
    if (it != tree.inorder_end())
        cout << "Primer par (inorder): " << *it << endl;  // 4

    auto rit = tree.ReverseFirstThat(isEven);
    if (rit != tree.rinorder_end()) 
        cout << "Primer par (reverse inorder): " << *rit << endl;  // 8

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
    IntAscTree tree2;
    {
        ifstream file("tree.txt");
        file >> tree2;
    }

    cout << "Inorder del árbol leído: ";
    tree2.ForEach(print);
    cout << endl;

    // 6. Copy constructor
    cout << "\n=== Copy constructor ===" << endl;
    IntAscTree tree3 = tree;
    tree3.insert(9, 7);

    cout << "Original : ";  tree.ForEach(print);  cout << endl;
    cout << "Copia    : ";  tree3.ForEach(print); cout << endl;

    // 7. Move constructor
    cout << "\n=== Move constructor ===" << endl;
    IntAscTree tree4 = std::move(tree3);
    cout << "Movido   : ";  tree4.ForEach(print); cout << endl;
    cout << "Vaciado  : ";  tree3.ForEach(print); cout << endl;

    // 8. Árbol descendente
    cout << "\n=== Árbol descendente ===" << endl;
    IntDescTree desc;
    desc.insert(5, 0);
    desc.insert(3, 1);
    desc.insert(7, 2);
    desc.insert(1, 3);
    desc.insert(8, 4);

    cout << "Inorder desc (desc): ";
    desc.ForEach(print);
    cout << endl;

    cout << "Operador <<: " << endl;
    cout << tree;

    cout << "ToString: " << endl;
    cout << tree.ToString();
}
