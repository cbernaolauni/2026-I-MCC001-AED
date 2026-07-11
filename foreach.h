/**
 * @file foreach.h
 * @brief Funciones libres genéricas ::ForEach y ::FirstThat, usadas por
 *        todos los contenedores del proyecto para exponer sus miembros
 *        ForEach/FirstThat delegando en estas plantillas.
 */
#ifndef __FOREACH_H__
#define __FOREACH_H__
#include <iostream>
#include <utility> // forward

using namespace std;

/**
 * @brief Aplica @p func a cada elemento en el rango [begin, end),
 *        recorriéndolo con @c operator++ desde @p begin hasta @p end.
 *
 * @tparam Iterator Tipo de iterador; debe soportar @c operator*, @c operator++
 *                  y comparación con @c operator!=.
 * @tparam Func Tipo del callable invocado por cada elemento.
 * @tparam Args Tipos de los argumentos extra reenviados a @p func.
 * @param begin Iterador al primer elemento del rango.
 * @param end Iterador al final del rango (exclusivo).
 * @param func Callable con firma compatible con (elemento, Args...).
 * @param args Argumentos adicionales reenviados a @p func en cada llamada.
 */
template <typename Iterator, typename Func, typename... Args>
void ForEach(Iterator begin, Iterator end, Func func, Args &&... args){
    for (auto it = begin; it != end; ++it)
        func(*it, forward<Args>(args)...);
    // cout<<endl;
}

// Variadic templates: template <typename ...Args>
// Variadic templates allow a function or class to accept an arbitrary
// number of arguments.
// Example: template <typename ...Args> func() { // ... }
/**
 * @brief Busca el primer elemento en el rango [begin, end) para el cual
 *        @p func retorna @c true.
 *
 * @tparam Iterator Tipo de iterador; debe soportar @c operator*, @c operator++
 *                  y comparación con @c operator!=.
 * @tparam Func Tipo del callable/predicado invocado por cada elemento.
 * @tparam Args Tipos de los argumentos extra reenviados a @p func.
 * @param begin Iterador al primer elemento del rango.
 * @param end Iterador al final del rango (exclusivo).
 * @param func Predicado con firma compatible con (elemento, Args...) -> bool.
 * @param args Argumentos adicionales reenviados a @p func en cada llamada.
 * @return Iterador al primer elemento que satisface @p func, o @p end si ninguno lo hace.
 */
template <typename Iterator, typename Func, typename... Args>
Iterator FirstThat(Iterator begin, Iterator end, Func func, Args &&... args){
    for (auto it = begin; it != end; ++it){
        if (func(*it, forward<Args>(args)...))
            return it;
    }
    return end;
}

/**
 * @brief Sobrecarga de conveniencia: aplica @p func a cada elemento de
 *        @p v1 delegando en ForEach(begin, end, func, args...) sobre
 *        @c v1.begin() / @c v1.end().
 *
 * @tparam Container Tipo con métodos @c begin() y @c end() (p. ej. std::vector, Vector).
 * @tparam Func Tipo del callable invocado por cada elemento.
 * @tparam Args Tipos de los argumentos extra reenviados a @p func.
 * @param v1 Contenedor a recorrer.
 * @param func Callable con firma compatible con (elemento, Args...).
 * @param args Argumentos adicionales reenviados a @p func en cada llamada.
 */
template <typename Container, typename Func, typename... Args>
void ForEach(Container& v1, Func func, Args &&... args){
    ForEach(v1.begin(), v1.end(), func, forward<Args>(args)...);
}

#endif // __FOREACH_H__