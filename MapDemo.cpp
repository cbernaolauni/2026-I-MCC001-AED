#include <iostream>
#include <map>
#include <unordered_map>
#include <string>
#include "types.h"
 
using namespace std;
 
// -----------------------------------------------------------------------
// 1) std::map es un arbol balanceado (tipicamente red-black tree) que
//    mantiene sus claves SIEMPRE ordenadas. Insert/Find/Erase son O(log n).
//    Es el equivalente STL a lo que hemos construido a mano con BTree/AVL.
// -----------------------------------------------------------------------
void DemoMapBasico()
{
    cout << "\n=== 1. Operaciones basicas ===\n";
 
    map<Nombre, Edad> persona;
 
    // insert() - no sobreescribe si la clave ya existe
    persona.insert({"Cesar", 30});
    persona.insert(make_pair("Ana", 25));
 
    // operator[] - inserta con valor por defecto si no existe, y SI sobreescribe
    persona["Luis"] = 40;
    persona["Cesar"] = 31; // sobreescribe el valor anterior
 
    // emplace() - construye el par in-place, evita copias temporales
    persona.emplace("Marta", 28);
 
    // iteracion: SIEMPRE en orden ascendente de la clave, sin llamar a sort()
    for (const auto& [nombre, edad] : persona)
        cout << nombre << " -> " << edad << "\n";
}
 
// -----------------------------------------------------------------------
// 2) Buscar sin insertar accidentalmente
// -----------------------------------------------------------------------
void DemoMapBusqueda()
{
    cout << "\n=== 2. Busqueda: find() vs operator[] ===\n";
 
    map<Nombre, TI> stock{{"tornillos", 100}, {"tuercas", 50}};
 
    // PELIGRO: operator[] en un map const-incorrecto CREA la clave si no
    // existe (con valor default), incluso si solo querias consultar.
    // find() es la forma segura de comprobar existencia sin efectos secundarios.
    auto it = stock.find("clavos");
    if (it == stock.end())
        cout << "\"clavos\" no existe en el stock (find no lo crea)\n";
 
    // contains() (C++20) es mas legible que comparar con end()
    if (!stock.contains("clavos"))
        cout << "\"clavos\" no existe (via contains())\n";
 
    // ahora si consultamos con operator[], "clavos" queda insertado con valor 0
    cout << "stock[\"clavos\"] = " << stock["clavos"] << " (se creo la entrada)\n";
    cout << "tamano tras la consulta: " << stock.size() << "\n";
}
 
// -----------------------------------------------------------------------
// 3) Recorrido en rango: aprovechando el orden interno con lower_bound /
//    upper_bound, algo que un unordered_map NO puede ofrecer.
// -----------------------------------------------------------------------
void DemoMapRangos()
{
    cout << "\n=== 3. Consultas de rango (lower_bound / upper_bound) ===\n";
 
    map<TI, Tipo> calificaciones{
        {60, "Suficiente"}, {70, "Bien"}, {85, "Notable"}, {95, "Sobresaliente"}};
 
    // Todo lo que este entre 70 y 90 (inclusive el limite inferior)
    auto desde = calificaciones.lower_bound(70); // primer elemento >= 70
    auto hasta = calificaciones.upper_bound(90);  // primer elemento > 90
 
    cout << "Calificaciones entre 70 y 90:\n";
    for (auto it = desde; it != hasta; ++it)
        cout << "  " << it->first << " -> " << it->second << "\n";
}
 
// -----------------------------------------------------------------------
// 4) Comparador personalizado: orden descendente, igual que se hizo con
//    AscendingBTreeTrait / DescendingBTreeTrait usando std::less / std::greater.
// -----------------------------------------------------------------------
void DemoMapComparadorPersonalizado()
{
    cout << "\n=== 4. Comparador personalizado (orden descendente) ===\n";
 
    map<TI, Tipo, greater<TI>> ranking{{60, "Suficiente"}, {70, "Bien"}, {85, "Notable"}, {95, "Sobresaliente"}};
 
    for (const auto& [pos, nombre] : ranking)
        cout << pos << ": " << nombre << "\n";
}
 
// -----------------------------------------------------------------------
// 5) map vs unordered_map: mismo API basico, distinta estructura interna
//    (arbol balanceado vs tabla hash) y por tanto distintas garantias.
// -----------------------------------------------------------------------
void DemoMapVsUnorderedMap()
{
    cout << "\n=== 5. map vs unordered_map ===\n";
 
    map<Letra, TI> ordenado{{"z", 1}, {"a", 2}, {"m", 3}};
    unordered_map<Letra, TI> noOrdenado{{"z", 1}, {"a", 2}, {"m", 3}};
 
    cout << "map (orden garantizado por clave):\n";
    for (const auto& [k, v] : ordenado)
        cout << "  " << k << " -> " << v << "\n";
 
    cout << "unordered_map (orden NO garantizado, depende del hash):\n";
    for (const auto& [k, v] : noOrdenado)
        cout << "  " << k << " -> " << v << "\n";
 
    cout << "\nmap:            O(log n) insert/find/erase, ordenado, hojas enlazadas\n";
    cout << "unordered_map:   O(1) promedio insert/find/erase, sin orden\n";
}
 
// -----------------------------------------------------------------------
// 6) Erase seguro durante iteracion
// -----------------------------------------------------------------------
void DemoMapErase()
{
    cout << "\n=== 6. erase() seguro durante iteracion ===\n";
 
    map<TI, Letra> datos{{1, "a"}, {2, "b"}, {3, "c"}, {4, "d"}};
 
    // erase(iterator) devuelve el iterador al SIGUIENTE elemento valido,
    // por eso NO se debe hacer ++it manualmente cuando se borra en el mismo paso.
    for (auto it = datos.begin(); it != datos.end(); )
    {
        if (it->first % 2 == 0)
            it = datos.erase(it); // borra los pares
        else
            ++it;
    }
 
    cout << "Quedan las claves impares:\n";
    for (const auto& [k, v] : datos)
        cout << "  " << k << " -> " << v << "\n";
}
 
void DemoMap()
{
    cout << "########## DEMO std::map ##########\n";
    DemoMapBasico();
    DemoMapBusqueda();
    DemoMapRangos();
    DemoMapComparadorPersonalizado();
    DemoMapVsUnorderedMap();
    DemoMapErase();
}