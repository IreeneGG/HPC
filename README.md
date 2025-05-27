#MULTIPLICACIÓN DE MATRIZ DISPERSA POR VECTOR 💻
Este proyecto se centra en la implementación y análisis de la multiplicación de una matriz dispersa por un vector, una operación fundamental en áreas como la ingeniería, simulaciones científicas y sistemas de recomendación.

El objetivo es evaluar el rendimiento de tres enfoques: una versión secuencial, una paralela con OpenMP y otra paralela distribuida usando MPI. Se mide el tiempo de ejecución, el speedup y la eficiencia en distintos tamaños de entrada y configuraciones de hardware.

##Estructura del proyecto 🗂️
###Versiones de implementación ⚙️
- secuencial.c: Implementación básica secuencial del producto matriz-vector en formato CSR.
- versionOpenMP.c: Implementación paralela utilizando OpenMP para acelerar la operación.
- versionMPI.c: Implementación paralela distribuida con MPI para ejecutar sobre múltiples procesos.

###Datos utilizados 📊
- Matrices y vectores utilizados representados en formato CSV (CSR para la matriz):
- matrizSencilla.csv, vectorSencillo.csv: Conjunto de datos pequeño, ideal para pruebas básicas.
- matriz500.csv, vector500.csv: Tamaño medio para evaluación de escalabilidad.
- matrizComplejaGrande.csv, vectorComplejoGrande.csv: Datos grandes para análisis de eficiencia y comportamiento con múltiples núcleos.

###Resultados 📈
Archivos CSV que registran el resultados son los ficheros:

- resultadosSecuencial.csv
- resultadosOpenMP.csv
- resultadosMPI.csv

Cada resultado incluye el número de filas procesadas, cantidad de hilos o procesos usados, y el tiempo total de ejecución.
