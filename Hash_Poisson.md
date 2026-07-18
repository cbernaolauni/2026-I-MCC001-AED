# Hash y Distribución de Poisson

## Introducción

Las **tablas hash** son estructuras de datos diseñadas para almacenar y recuperar información de forma muy eficiente. Su funcionamiento se basa en una **función hash**, la cual transforma una clave en un índice dentro de una tabla de tamaño fijo.

Cuando la función hash distribuye correctamente las claves, las operaciones de inserción, búsqueda y eliminación tienen una complejidad promedio de **O(1)**. Sin embargo, cuando dos o más claves se asignan a la misma posición de la tabla, se produce una **colisión**.

Para estudiar matemáticamente el comportamiento de estas colisiones, se utiliza la **Distribución de Poisson**, la cual permite estimar la probabilidad de que un bucket reciba cierta cantidad de elementos.

---

# Factor de carga

El comportamiento de una tabla hash depende principalmente del **factor de carga**:

$$
\alpha = \frac{N}{M}
$$

Donde:

- **N** = número de elementos almacenados.
- **M** = número de buckets (casillas) de la tabla.
- **α** = factor de carga.

Interpretación:

- Si **α < 1**, la tabla está poco ocupada.
- Si **α ≈ 1**, existe aproximadamente un elemento por bucket.
- Si **α > 1**, comienzan a incrementarse las colisiones.

---

# Distribución de Poisson

Suponiendo que la función hash distribuye las claves de forma uniforme e independiente, el número de elementos que recibe un bucket puede modelarse mediante la **Distribución de Poisson**.

La probabilidad de que un bucket contenga exactamente **k** elementos está dada por:

$$
P(X=k)=\frac{\alpha^k e^{-\alpha}}{k!}
$$

Donde:

- **k** = número de elementos en un bucket.
- **α** = factor de carga.
- **e** = número de Euler (≈ 2.71828).

Este modelo es una excelente aproximación cuando el número de elementos y de buckets es suficientemente grande.

---

# Probabilidad de una casilla vacía

Si se desea conocer la probabilidad de que un bucket permanezca vacío, basta con evaluar la distribución para **k = 0**.

$$
P(X=0)=e^{-\alpha}
$$

Esto significa que un bucket permanecerá vacío con una probabilidad igual a \(e^{-\alpha}\).

Por lo tanto, el número esperado de buckets ocupados es:

$$
M\left(1-e^{-\alpha}\right)
$$

---

# Número esperado de colisiones

Cada bucket ocupado contiene al menos un elemento que no genera colisión. Todos los elementos adicionales almacenados en ese mismo bucket representan colisiones.

Por ello, el número esperado de colisiones puede estimarse mediante:

$$
E[C]=N-M\left(1-e^{-\alpha}\right)
$$

donde:

- **E[C]** = número esperado de colisiones.
- **N** = número total de elementos.
- **M** = número de buckets.

Esta expresión permite estimar la cantidad de colisiones que tendrá una tabla hash antes de implementarla.

---

# Análisis del comportamiento

## Caso 1: Aumenta el tamaño de la tabla

Si el número de buckets aumenta mientras el número de elementos permanece constante,

$$
\alpha=\frac{N}{M}\rightarrow0
$$

entonces

$$
e^{-\alpha}\rightarrow1
$$

y el número esperado de colisiones tiende a cero.

**Conclusión:** cuanto mayor sea el tamaño de la tabla hash, menor será la probabilidad de colisiones.

---

## Caso 2: Aumenta el número de elementos

Cuando el número de elementos es mucho mayor que el número de buckets,

$$
\alpha\gg1
$$

entonces

$$
e^{-\alpha}\approx0
$$

y la expresión anterior se aproxima a:

$$
E[C]\approx N-M
$$

En este escenario la tabla hash se encuentra saturada y prácticamente cada nuevo elemento insertado genera una colisión.

---

## Caso 3: Factor de carga cercano a uno

Cuando

$$
\alpha\approx1
$$

cada bucket contiene, en promedio, un elemento.

Aunque continúan existiendo colisiones, la tabla mantiene un buen rendimiento y las operaciones siguen ejecutándose en tiempo promedio **O(1)**.

---

# Conclusiones

- Una tabla hash permite acceder a los datos de forma muy eficiente mediante una función hash.
- El rendimiento de la estructura depende directamente del **factor de carga**.
- La **Distribución de Poisson** proporciona un modelo probabilístico para estimar cuántos elementos terminarán almacenados en cada bucket.
- Gracias a este modelo es posible calcular el número esperado de colisiones y comprender cómo influyen el tamaño de la tabla y la cantidad de elementos en el rendimiento.
- Mantener un factor de carga bajo reduce significativamente las colisiones y permite conservar un tiempo de búsqueda e inserción cercano a **O(1)**.