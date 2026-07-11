-- =====================================================================
-- indices_demo.sql
-- Demo de indices en PostgreSQL: B-Tree, Hash y GiST
-- + Analizador de tiempos de consulta (con y sin indice)
--
-- Uso recomendado (psql):
--   psql -U tu_usuario -d tu_bd -f indices_demo.sql
--
-- El script es idempotente: puede correrse varias veces sin errores.
-- =====================================================================
-- (solo psql) muestra el tiempo de cada sentencia ejecutada
\timing on
 
-- =====================================================================
-- 0) LIMPIEZA (por si se corre el script varias veces)
-- =====================================================================
DROP TABLE IF EXISTS query_benchmark_log CASCADE;
DROP TABLE IF EXISTS reservas            CASCADE;
DROP TABLE IF EXISTS empleados           CASCADE;
 
-- =====================================================================
-- 1) B-TREE: tabla + datos + consultas de comparacion, rango y orden
-- =====================================================================
CREATE TABLE empleados (
    id            SERIAL PRIMARY KEY,      -- ya crea un B-Tree implicito
    nombre        TEXT,
    salario       NUMERIC,
    fecha_ingreso DATE
);
 
-- Datos sinteticos (200k filas) para que las diferencias de tiempo sean medibles
INSERT INTO empleados (nombre, salario, fecha_ingreso)
SELECT
    'empleado_' || i,
    (random() * 10000 + 1000)::numeric(10,2),
    date '2015-01-01' + (random() * 4000)::int
FROM generate_series(1, 200000) AS i;
 
-- --- Consultas SIN indice sobre "salario" (aun no lo creamos) --------
EXPLAIN ANALYZE
SELECT * FROM empleados WHERE salario > 5000;
 
-- --- Ahora creamos el indice B-Tree -----------------------------------
CREATE INDEX idx_empleados_salario ON empleados (salario);
 
-- --- Misma consulta, ahora CON indice ---------------------------------
EXPLAIN ANALYZE
SELECT * FROM empleados WHERE salario > 5000;
 
-- El B-Tree tambien acelera rangos y ORDER BY sobre la misma columna
EXPLAIN ANALYZE
SELECT * FROM empleados WHERE salario BETWEEN 3000 AND 6000;
 
EXPLAIN ANALYZE
SELECT * FROM empleados ORDER BY salario LIMIT 20;
 
-- =====================================================================
-- 2) HASH: solo sirve para igualdad exacta (=)
-- =====================================================================
 
-- --- Busqueda por nombre exacto SIN indice ----------------------------
EXPLAIN ANALYZE
SELECT * FROM empleados WHERE nombre = 'empleado_150000';
 
-- --- Creamos el indice HASH --------------------------------------------
CREATE INDEX idx_empleados_nombre_hash ON empleados USING HASH (nombre);
 
-- --- Misma busqueda, ahora CON indice hash -----------------------------
EXPLAIN ANALYZE
SELECT * FROM empleados WHERE nombre = 'empleado_150000';
 
-- El indice hash NO ayuda en comparaciones de rango: el planner lo ignora
-- y vuelve a un Seq Scan (o usa el B-Tree si existiera uno sobre nombre).
EXPLAIN ANALYZE
SELECT * FROM empleados WHERE nombre > 'empleado_1';
 
-- =====================================================================
-- 3) GiST: solapamiento de rangos (tsrange), no resoluble con B-Tree
-- =====================================================================
CREATE TABLE reservas (
    id      SERIAL PRIMARY KEY,
    sala    TEXT,
    periodo TSRANGE
);
 
INSERT INTO reservas (sala, periodo)
SELECT
    'sala_' || (i % 20),
    tsrange(
        ts,
        ts + (interval '30 minutes' * (1 + (random() * 5)::int))
    )
FROM (
    SELECT
        i,
        timestamp '2026-01-01 08:00' + (random() * interval '180 days')
            + (floor(random() * 20) * interval '30 minutes') AS ts
    FROM generate_series(1, 100000) AS i
) sub;
 
-- --- Consulta de solapamiento SIN indice --------------------------------
EXPLAIN ANALYZE
SELECT * FROM reservas
WHERE periodo && tsrange('2026-03-10 09:00', '2026-03-10 11:00');
 
-- --- Creamos el indice GiST sobre el rango ------------------------------
CREATE INDEX idx_reservas_periodo ON reservas USING GIST (periodo);
 
-- --- Misma consulta, ahora CON indice GiST -------------------------------
EXPLAIN ANALYZE
SELECT * FROM reservas
WHERE periodo && tsrange('2026-03-10 09:00', '2026-03-10 11:00');
 
-- =====================================================================
-- 4) ANALIZADOR DE TIEMPOS: funcion + tabla de bitacora reutilizable
--
-- Permite medir el tiempo real de ejecucion de cualquier sentencia SQL
-- (via EXECUTE dinamico) y dejar el resultado en una tabla, para poder
-- comparar "antes" vs "despues" de crear un indice sin depender de leer
-- manualmente cada EXPLAIN ANALYZE.
-- =====================================================================
CREATE TABLE query_benchmark_log (
    id           SERIAL PRIMARY KEY,
    etiqueta     TEXT,
    consulta     TEXT,
    tiempo_ms    NUMERIC,
    ejecutado_en TIMESTAMP DEFAULT now()
);
 
CREATE OR REPLACE FUNCTION medir_consulta(p_etiqueta TEXT, p_sql TEXT)
RETURNS NUMERIC AS $$
DECLARE
    v_inicio TIMESTAMPTZ;
    v_fin    TIMESTAMPTZ;
    v_ms     NUMERIC;
BEGIN
    v_inicio := clock_timestamp();
    EXECUTE p_sql;                 -- corre la consulta; si es SELECT, descarta las filas
    v_fin := clock_timestamp();
 
    v_ms := EXTRACT(EPOCH FROM (v_fin - v_inicio)) * 1000;
 
    INSERT INTO query_benchmark_log (etiqueta, consulta, tiempo_ms)
    VALUES (p_etiqueta, p_sql, v_ms);
 
    RETURN v_ms;
END;
$$ LANGUAGE plpgsql;
 
-- Helper para repetir una medicion N veces y promediar (reduce ruido de cache/OS)
CREATE OR REPLACE FUNCTION medir_consulta_repetida(p_etiqueta TEXT, p_sql TEXT, p_repeticiones INT DEFAULT 5)
RETURNS NUMERIC AS $$
DECLARE
    v_promedio NUMERIC;
    i INT;
BEGIN
    FOR i IN 1..p_repeticiones LOOP
        PERFORM medir_consulta(p_etiqueta, p_sql);
    END LOOP;
 
    SELECT AVG(tiempo_ms) INTO v_promedio
    FROM query_benchmark_log
    WHERE etiqueta = p_etiqueta
      AND ejecutado_en >= now() - interval '1 minute';
 
    RETURN v_promedio;
END;
$$ LANGUAGE plpgsql;
 
-- =====================================================================
-- 5) USO DEL ANALIZADOR: comparar consultas con distintos indices
-- =====================================================================
 
-- B-Tree: igualdad y rango sobre salario (ya indexado en el paso 1)
SELECT medir_consulta_repetida(
    'btree_igualdad',
    'SELECT * FROM empleados WHERE salario = 5000'
);
 
SELECT medir_consulta_repetida(
    'btree_rango',
    'SELECT * FROM empleados WHERE salario BETWEEN 3000 AND 6000'
);
 
-- Hash: igualdad sobre nombre (ya indexado en el paso 2)
SELECT medir_consulta_repetida(
    'hash_igualdad',
    'SELECT * FROM empleados WHERE nombre = ''empleado_150000'''
);
 
-- GiST: solapamiento de rangos (ya indexado en el paso 3)
SELECT medir_consulta_repetida(
    'gist_solapamiento',
    'SELECT * FROM reservas WHERE periodo && tsrange(''2026-03-10 09:00'', ''2026-03-10 11:00'')'
);
 
-- Sin indice, para contraste: forzamos un Seq Scan deshabilitando el uso de indices
-- (solo dentro de esta sesion; no afecta a otras conexiones)
SET enable_indexscan = OFF;
SET enable_bitmapscan = OFF;
 
SELECT medir_consulta_repetida(
    'btree_igualdad_sin_indice',
    'SELECT * FROM empleados WHERE salario = 5000'
);
 
RESET enable_indexscan;
RESET enable_bitmapscan;
 
-- =====================================================================
-- 6) REPORTE FINAL: promedio de tiempo por etiqueta, ordenado de mas
--    lento a mas rapido
-- =====================================================================
SELECT
    etiqueta,
    COUNT(*)                AS mediciones,
    ROUND(AVG(tiempo_ms), 3) AS tiempo_promedio_ms,
    ROUND(MIN(tiempo_ms), 3) AS tiempo_min_ms,
    ROUND(MAX(tiempo_ms), 3) AS tiempo_max_ms
FROM query_benchmark_log
GROUP BY etiqueta
ORDER BY tiempo_promedio_ms DESC;
 
\timing off