#include <iostream>
#include "containers/avltree.h"

using AVLTreeDemo = AVLTree<AscendingAVLTrait<TI>>;

void DemoAVLTree() {

    auto print = [](TI& val) { cout << val << " "; };

    // 1. Insertar
    cout << "=== Insertar ===" << endl;
    AVLTreeDemo tree;
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

    // 2. Inorder debe estar ordenado (igual que BST normal)
    cout << "\nInorder (debe ser ascendente):" << endl;
    for (auto& x : tree.ForEach()) print(x);
    cout << endl;

    // 3. Factor de balance de cada nodo
    // bf debe estar en [-1, 0, 1] para todo nodo → árbol balanceado
    cout << "\nNodos con altura y factor de balance:" << endl;
    cout << tree.ToStringVerbose() << endl;

    // 4. Caso LL — insertar secuencia ascendente forzaría rotaciones
    // En un BST normal quedaría una lista enlazada (altura = n)
    // En AVL la altura se mantiene O(log n)
    cout << "=== Caso LL (secuencia ascendente) ===" << endl;
    AVLTreeDemo ll;
    ll.insert(1, 0);
    ll.insert(2, 1);
    ll.insert(3, 2);   // RR -> rotate_left
    ll.insert(4, 3);
    ll.insert(5, 4);   // RR -> rotate_left

    cout << "Inorder: ";
    for (auto& x : ll.ForEach()) print(x);
    cout << endl;
    cout << ll.ToStringVerbose() << endl;

    // 5. Caso LR
    cout << "=== Caso LR ===" << endl;
    AVLTreeDemo lr;
    lr.insert(3, 0);
    lr.insert(1, 1);
    lr.insert(2, 2);   // LR: rotate_left(1) + rotate_right(3)

    cout << "Inorder: ";
    for (auto& x : lr.ForEach()) print(x);
    cout << endl;
    cout << lr.ToStringVerbose() << endl;

    // 6. Recorridos (heredados de BinaryTree)
    cout << "=== Recorridos ===" << endl;
    cout << "Inorder    forward : ";
    for (auto& x : tree.ForEach()) print(x);
    cout << endl;

    cout << "Inorder    backward: ";
    for (auto& x : tree.ReverseForEach()) print(x);
    cout << endl;

    cout << "Preorder   forward : ";
    for (auto& x : tree.PreorderForEach()) print(x);
    cout << endl;

    cout << "Postorder  forward : ";
    for (auto& x : tree.PostorderForEach()) print(x);
    cout << endl;

    cout << "Operador <<: " << endl;
    cout << tree;
}