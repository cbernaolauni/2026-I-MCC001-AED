/**
 * @file TrieDemo.cpp
 * @brief Demo del Trie con patrones BTree: insercion, busqueda exacta,
 *        prefijos, eliminacion y recorrido lexicografico con iteradores.
 *
 * Reproduce exactamente los mismos casos del demo original (DigitalTrieDemo.cpp),
 * ahora sobre la implementacion basada en Traits/CTrieNode/mutex.
 */
#include <iostream>
#include "Trie.h"

using namespace std;

void DemoDigitalTrie()
{
        using CharTrait = TrieTrait<char>;
        Trie<CharTrait> trie;

        cout << "========== INSERT ==========\n";

        trie.Insert("gato");
        trie.Insert("auto");
        trie.Insert("seguro");
        trie.Insert("perro");
        trie.Insert("manzana");
        trie.Insert("aplicacion");
        trie.Insert("casa");
        trie.Insert("caceria");

        cout << "\n========== SEARCH ==========\n";

        cout << boolalpha;
        cout << "gato         : " << trie.Contains("gato") << "\n";
        cout << "auto         : " << trie.Contains("auto") << "\n";
        cout << "seguro       : " << trie.Contains("seguro") << "\n";
        cout << "perro        : " << trie.Contains("perro") << "\n";
        cout << "manzana      : " << trie.Contains("manzana") << "\n";
        cout << "aplicacion   : " << trie.Contains("aplicacion") << "\n";
        cout << "app          : " << trie.Contains("app") << "\n";
        cout << "casa         : " << trie.Contains("casa") << "\n";

        cout << "\n========== PREFIX ==========\n";

        cout << "ca   : " << trie.StartsWith("ca") << "\n";
        cout << "seg  : " << trie.StartsWith("seg") << "\n";
        cout << "app  : " << trie.StartsWith("app") << "\n";
        cout << "per  : " << trie.StartsWith("per") << "\n";
        cout << "man  : " << trie.StartsWith("man") << "\n";

        cout << "\n========== REMOVE ==========\n";

        trie.Remove("ca");

        cout << "ca          : " << trie.Contains("ca") << "\n";
        cout << "ma          : " << trie.Contains("ma") << "\n";
        cout << "ga          : " << trie.Contains("ga") << "\n";

        trie.Remove("seguro");

        cout << "seguro      : " << trie.Contains("seguro") << "\n";
        cout << "a           : " << trie.Contains("a") << "\n";

        cout << "\n========== PRINT (arbol de prefijos) ==========\n";
        trie.Print(cout);

        cout << "\n========== FOREACH (orden lexicografico, via iteradores) ==========\n";
        trie.ForEach([](Trie<CharTrait>::Node &entry) {
                cout << "  " << entry.word << "\n";
        });

        cout << "\n========== RECORRIDO DESCENDENTE (backward_iterator) ==========\n";
        for (auto it = trie.rbegin(); it != trie.rend(); ++it)
                cout << "  " << (*it).word << "\n";

        cout << "\n========== FIRSTTHAT ==========\n";
        auto encontrado = trie.FirstThat([](Trie<CharTrait>::Node &entry, char letra) {
                return !entry.word.empty() && entry.word.front() == letra;
        }, 'm');
        if (encontrado)
                cout << "Primera palabra que empieza con 'm': " << encontrado->word << "\n";
        else
                cout << "No encontrado\n";

        cout << "\n========== FIN ==========\n";
}
