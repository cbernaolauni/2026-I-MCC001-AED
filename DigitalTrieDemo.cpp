#include <iostream>
#include <memory>
#include <unordered_map>
#include <string>

using namespace std;

class Trie {
private:

    struct TrieNode {
        unordered_map<char, unique_ptr<TrieNode>> children;
        bool isEndOfWord = false;
    };

    unique_ptr<TrieNode> root;

    bool removeHelper(TrieNode* node, const string& word, size_t depth) {

        if (depth == word.length()) {

            if (!node->isEndOfWord)
                return false;

            node->isEndOfWord = false;

            return node->children.empty();
        }

        char c = word[depth];

        auto it = node->children.find(c);

        if (it == node->children.end())
            return false;

        bool shouldDeleteChild =
            removeHelper(it->second.get(), word, depth + 1);

        if (shouldDeleteChild) {
            node->children.erase(c);
        }

        return !node->isEndOfWord && node->children.empty();
    }

    void printHelper(const TrieNode* node,
                 const string& prefix,
                 size_t depth) const
    {
        for (const auto& [c, child] : node->children)
        {
            for (size_t i = 0; i < depth; i++)
                cout << "|   ";

            cout << "+-- " << c;

            if (child->isEndOfWord)
                cout << " *";

            cout << endl;

            printHelper(child.get(), prefix + c, depth + 1);
        }
    }

public:

    Trie() : root(make_unique<TrieNode>()) {}

    // Insertar
    void insert(const string& word) {

        TrieNode* current = root.get();

        for (char c : word) {

            if (!current->children.count(c))
                current->children[c] = make_unique<TrieNode>();

            current = current->children[c].get();
        }

        current->isEndOfWord = true;
    }

    // Buscar palabra exacta
    bool search(const string& word) const {

        const TrieNode* current = root.get();

        for (char c : word) {

            auto it = current->children.find(c);

            if (it == current->children.end())
                return false;

            current = it->second.get();
        }

        return current->isEndOfWord;
    }

    // Buscar prefijo
    bool startsWith(const string& prefix) const {

        const TrieNode* current = root.get();

        for (char c : prefix) {

            auto it = current->children.find(c);

            if (it == current->children.end())
                return false;

            current = it->second.get();
        }

        return true;
    }

    // Eliminar palabra
    void remove(const string& word) {
        removeHelper(root.get(), word, 0);
    }

    void print() const {
        cout << "(root)" << endl;
        printHelper(root.get(), "", 0);
    }
};

void DemoDigitalTree() {

    Trie trie;

    cout << "========== INSERT ==========\n";

    trie.insert("gato");
    trie.insert("auto");
    trie.insert("seguro");
    trie.insert("perro");
    trie.insert("manzana");
    trie.insert("aplicacion");
    trie.insert("casa");
    trie.insert("caceria");

    cout << "\n========== SEARCH ==========\n";

    cout << "gato         : " << boolalpha << trie.search("gato") << endl;
    cout << "auto         : " << trie.search("auto") << endl;
    cout << "seguro       : " << trie.search("seguro") << endl;
    cout << "perro        : " << trie.search("perro") << endl;
    cout << "manzana      : " << trie.search("manzana") << endl;
    cout << "aplicacion   : " << trie.search("aplicacion") << endl;
    cout << "app          : " << trie.search("app") << endl;
    cout << "casa        : " << trie.search("casa") << endl;

    cout << "\n========== PREFIX ==========\n";

    cout << "ca   : " << trie.startsWith("ca") << endl;
    cout << "seg  : " << trie.startsWith("seg") << endl;
    cout << "app  : " << trie.startsWith("app") << endl;
    cout << "per   : " << trie.startsWith("per") << endl;
    cout << "man   : " << trie.startsWith("man") << endl;

    cout << "\n========== REMOVE ==========\n";

    trie.remove("ca");

    cout << "ca          : " << trie.search("ca") << endl;
    cout << "ma         : " << trie.search("ma") << endl;
    cout << "ga          : " << trie.search("ga") << endl;

    trie.remove("seguro");

    cout << "seguro         : " << trie.search("seguro") << endl;
    cout << "a          : " << trie.search("a") << endl;

    cout << "\n========== PRINT ==========\n";
    trie.print();

    cout << "\n========== FIN ==========\n";
}