#include <iostream>
#include <vector>
#include <string>
#include <numeric>
#include <algorithm>
#include <execution>
#include <variant>
#include <chrono>
#include <cmath>
#include "types.h"

using namespace std;

// -----------------------------------------------------------------------
// Patron "overload": junta varios lambdas en un solo objeto invocable con
// sobrecarga de operator(), para usarlo directo con std::visit sin tener
// que escribir una struct visitor con nombre para cada variant.
// -----------------------------------------------------------------------
template <typename... Ts>
struct Overload : Ts...
{
    using Ts::operator()...;
};
template <typename... Ts>
Overload(Ts...) -> Overload<Ts...>;

// -----------------------------------------------------------------------
// Demo 1: std::execution (algoritmos paralelos de la STL, C++17/20)
//
// Compara el mismo calculo (costoso por elemento) corrido con politicas
// de ejecucion secuencial vs paralela, sin escribir un solo std::thread
// ni std::mutex a mano: la STL reparte el trabajo entre nucleos por debajo.
// -----------------------------------------------------------------------
void demo1()
{
    cout << "std::execution: secuencial vs paralelo (sin threads manuales)\n\n";

    const size_t N = 20'000'000;
    vector<TD> datos(N);
    iota(datos.begin(), datos.end(), 1.0);
    vector<TD> resultado(N);

    auto costoso = [](TD x) { return sqrt(x) * sin(x) + log(x); };

    // --- transform secuencial ---
    auto t0 = chrono::high_resolution_clock::now();
    transform(execution::seq, datos.begin(), datos.end(), resultado.begin(), costoso);
    auto t1 = chrono::high_resolution_clock::now();
    auto msSeq = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();

    // --- transform paralelo (par_unseq: paralelo + vectorizado) ---
    t0 = chrono::high_resolution_clock::now();
    transform(execution::par_unseq, datos.begin(), datos.end(), resultado.begin(), costoso);
    t1 = chrono::high_resolution_clock::now();
    auto msPar = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();

    cout << "N = " << N << " elementos, funcion: sqrt(x)*sin(x)+log(x)\n";
    cout << "  transform(execution::seq)       : " << msSeq << " ms\n";
    cout << "  transform(execution::par_unseq) : " << msPar << " ms\n";
    if (msPar > 0)
        cout << "  speedup aproximado               : " << (TD)msSeq / msPar << "x\n";

    // --- reduce secuencial vs paralelo (suma de todo el vector) ---
    t0 = chrono::high_resolution_clock::now();
    TD sumaSeq = reduce(execution::seq, datos.begin(), datos.end(), 0.0);
    t1 = chrono::high_resolution_clock::now();
    auto msReduceSeq = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();

    t0 = chrono::high_resolution_clock::now();
    TD sumaPar = reduce(execution::par, datos.begin(), datos.end(), 0.0);
    t1 = chrono::high_resolution_clock::now();
    auto msReducePar = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();

    cout << "\n  reduce(execution::seq) : " << msReduceSeq << " ms (suma=" << sumaSeq << ")\n";
    cout << "  reduce(execution::par) : " << msReducePar << " ms (suma=" << sumaPar << ")\n";

    cout << "\n  Nota: el speedup real depende de los nucleos disponibles y de si\n"
         << "  la libstdc++ del sistema tiene TBB habilitado; sin TBB, las\n"
         << "  politicas paralelas pueden ejecutar de forma efectivamente\n"
         << "  secuencial (la API es la misma, es solo el backend el que cambia).\n";
}

// -----------------------------------------------------------------------
// Demo 2: std::variant + std::visit (visitor pattern type-safe)
//
// Un "contenedor heterogeneo" de lecturas de sensores con 3 formas
// distintas de resultado, procesado sin herencia, sin virtual, sin
// dynamic_cast y sin RTTI: el compilador verifica en tiempo de
// compilacion que TODOS los casos estan cubiertos en el visitor.
// -----------------------------------------------------------------------
struct LecturaValida   { Nombre sensor; TD valor; };
struct LecturaError    { Nombre sensor; Nombre motivo; };
struct LecturaEnEspera { Nombre sensor; };

using Lectura = variant<LecturaValida, LecturaError, LecturaEnEspera>;

void demo2()
{
    cout << "std::variant + std::visit: lecturas heterogeneas sin herencia\n\n";

    vector<Lectura> lecturas = {
        LecturaValida{"sensor_temp", 23.5},
        LecturaError{"sensor_presion", "timeout"},
        LecturaEnEspera{"sensor_humedad"},
        LecturaValida{"sensor_temp", 24.1},
        LecturaError{"sensor_gas", "fuera de rango"},
    };

    TI validas = 0, errores = 0, espera = 0;

    for (const auto &lectura : lecturas)
    {
        visit(Overload{
                  [&](const LecturaValida &l)
                  {
                      cout << "  [OK]      " << l.sensor << " = " << l.valor << "\n";
                      validas++;
                  },
                  [&](const LecturaError &l)
                  {
                      cout << "  [ERROR]   " << l.sensor << " -> " << l.motivo << "\n";
                      errores++;
                  },
                  [&](const LecturaEnEspera &l)
                  {
                      cout << "  [ESPERA]  " << l.sensor << " sin datos aun\n";
                      espera++;
                  }},
              lectura);
    }

    cout << "\nResumen: " << validas << " validas, " << errores << " errores, " << espera << " en espera\n";

    cout << "\n(bonus) filtrando solo las validas con get_if (sin visit completo):\n";
    for (const auto &lectura : lecturas)
        if (auto *v = get_if<LecturaValida>(&lectura))
            cout << "  " << v->sensor << ": " << v->valor << "\n";
}

void DemoStl()
{
    cout << "########## DEMO STL ##########\n";
    cout << "1) Algoritmos paralelos con std::execution\n";
    demo1();
    cout << "\n2) Lecturas heterogeneas con std::variant + std::visit\n";
    demo2();
}