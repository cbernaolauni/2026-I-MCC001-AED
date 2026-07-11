/**
 * @file BTreeDemo.cpp
 * @brief Demo de ejercitación de BTree: inserción masiva de caracteres,
 *        recorrido con iteradores forward/backward, y búsqueda con FirstThat.
 */
//#include <iostream.h>
#include <time.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include "BTree.h"

//const char * keys="CDAMPIWNBKEHOLJYQZFXVRTSGU";
const char * keys1 = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
const char * keys2 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const char * keys3 = "DYZakHIUwxVJ203ejOP9Qc8AdtuEop1XvTRghSNbW567BfiCqrs4FGMyzKLlmn";

const Order BTreeSize = 3;

/**
 * @brief Ejercita BTree<BTreePageTrait<char>>: inserta todas las claves de
 *        @c keys1, imprime el árbol, recorre con los iteradores forward y
 *        backward, y ejecuta un FirstThat buscando la clave 'q'.
 */
void BtreeDemo()
{
        using CharTrait = BTreePageTrait<char>;
        BTree<CharTrait> bt(BTreeSize);
       
        for (Order i = 0; keys1[i]; i++) {
                bt.Insert(keys1[i], i * i);
        }
        bt.Print(cout);

        cout << "\n--- Forward iterator ---\n";
        for (auto it = bt.begin(); it != bt.end(); ++it)
                cout << (*it).key << "->" << (*it).ObjID << " ";
        cout << "\n";
 
        cout << "\n--- Backward iterator ---\n";
        for (auto it = bt.rbegin(); it != bt.rend(); ++it)
                cout << (*it).key << "->" << (*it).ObjID << " ";
        cout << "\n";
        
        cout << "\n--- FirstThat ---\n";
        using Node = BTree<CharTrait>::Node;
        auto* node = bt.FirstThat([](Node& n, char target) {
                return n.key == target;
        }, 'q');
        if (node)
                cout << "Encontro: " << node->key << " -> " << node->ObjID << "\n";
        else
                cout << "No encontrado\n";

       /*for (i = 0; keys2[i]; i++)
       {
               cout << "Searching " << keys2[i] << " ";
               long ObjID = bt.Search(keys2[i]);
               if( ObjID != -1 )
                       cout << "Achei " << keys2[i] << " ID = " << ObjID << endl;
               else
                       cout <<"Nao achei!" << keys2[i] << endl;
       }*/
       /*cout.flush();

       for (i = 0; keys3[i]; i++)
       {
               cout << "Removing " << keys3[i] << " ";
               if( bt.Remove(keys3[i], -1) )
                       cout << keys3[i] << " removido !" << endl;
               else
                       cout <<"Nao achei!" << keys3[i] << endl;
               bt.Print(cout);
       }
       bt.Print(cout);
       cout.flush();*/
}

/*const char * keys="CDAMPIWNBKEHOLJYQZFXVRTSGU";
const char * keys2="CDAMPIWNBKEHOLJYQZFXVRTSGU";
const int BTreeSize = 3;
main (int argc, char * argv)
{
       //__int64 li;
       BTree <__int64> bt (BTreeSize);
       for (register int i = 0; i < 1000000; i++)
       {
               //cout<<"Inserting "<<keys[i]<<endl;
               bt.Insert(i, i-1);
               //bt.Print(cout);
       }

       for (i = 0; i < 1000; i++)
       {
               __int64 key = 975000+(::rand()%50000);
               //cout << "Searching " << (long)key << " ";
               long ObjID = bt.Search(key);
               if( ObjID != -1 )
                       cout << "Achei " << (long)key << " ID = " << ObjID << endl;
               else
                       cout <<"  Nao achei!" << (long)key << endl;
       }
       cout.flush();

       return 1;
}*/



/*const int BTreeSize = 3;
main (int argc, char * argv)
{
       int result, i;
       BTree <LONGLONG> bt(BTreeSize);
       result = bt.Create ("ernesto3-string-btree-start.dat",ios::in|ios::out);
       if (!result) { cout<<"Please delete testbt.dat"<<endl;return 0; }
       srand( (unsigned)time( NULL ) );
       LARGE_INTEGER key;
       for (i = 0; i < 1000000; i++)
       {
               //cout<<"Inserting "<<keys[i]<<endl;
               char strTmp[50];
               key.LowPart = rand();
               key.HighPart = rand();
               std::string str(strTmp);
               result = bt.Insert(key.QuadPart, i);
               //bt.Print(cout);
               if( i % 100000 == 0 )
               {       cout << i << endl; cout.flush();        }
       }
       //cout << "Searching D " << bt.Search();
       //bt.Search(1,1);
       cout.flush();
       return 1;
}*/
