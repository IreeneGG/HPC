#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <sys/stat.h>

// Función para leer una línea CSV como float o int
int leerLineaCSV(const char *linea, float **valores_f, int **valores_i, int es_float) {
    int capacidad = 16, cantidad = 0;
    if (es_float)
        *valores_f = (float *)malloc(capacidad * sizeof(float));
    else
        *valores_i = (int *)malloc(capacidad * sizeof(int));

    char *token = strtok((char *)linea, ",");
    while (token != NULL) {
        if (cantidad >= capacidad) {
            capacidad *= 2;
            if (es_float)
                *valores_f = (float *)realloc(*valores_f, capacidad * sizeof(float));
            else
                *valores_i = (int *)realloc(*valores_i, capacidad * sizeof(int));
        }
        if (es_float)
            (*valores_f)[cantidad] = atof(token);
        else
            (*valores_i)[cantidad] = atoi(token);
        cantidad++;
        token = strtok(NULL, ",");
    }
    return cantidad;
}

void multiplicarCSRParalela(float val[], int col[], int row_ptr[], float x[], float y[], int filas) {
    #pragma omp parallel for
    for (int i = 0; i < filas; i++) {
        float suma = 0.0;
        for (int j = row_ptr[i]; j < row_ptr[i + 1]; j++) {
            suma += val[j] * x[col[j]];
        }
        y[i] = suma;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s matriz.csv vector.csv\n", argv[0]);
        return 1;
    }

    FILE *fmat = fopen(argv[1], "r");
    if (!fmat) {
        perror("Error al abrir matriz.csv");
        return 1;
    }

    char linea[32768];
    float *val = NULL;
    int *col = NULL, *row_ptr = NULL;
    int nnz, filas;

    // val[]
    fgets(linea, sizeof(linea), fmat);
    nnz = leerLineaCSV(linea, &val, NULL, 1);

    // col[]
    fgets(linea, sizeof(linea), fmat);
    leerLineaCSV(linea, NULL, &col, 0);

    // row_ptr[]
    fgets(linea, sizeof(linea), fmat);
    int row_ptr_len = leerLineaCSV(linea, NULL, &row_ptr, 0);
    filas = row_ptr_len - 1;
    fclose(fmat);

    FILE *fvec = fopen(argv[2], "r");
    if (!fvec) {
        perror("Error al abrir vector.csv");
        return 1;
    }

    fgets(linea, sizeof(linea), fvec);
    float *x = NULL;
    int tam_x = leerLineaCSV(linea, &x, NULL, 1);
    fclose(fvec);

    if (tam_x <= 0 || filas <= 0) {
        fprintf(stderr, "Error: Vector o matriz vacíos\n");
        return 1;
    }

    float *y = (float *)calloc(filas, sizeof(float));

    double inicio = omp_get_wtime();
    multiplicarCSRParalela(val, col, row_ptr, x, y, filas);
    double fin = omp_get_wtime();
    double tiempo = fin - inicio;

    // Imprimir resultado
    printf("Resultado y = A * x:\n");
    for (int i = 0; i < filas; i++) {
        printf("y[%d] = %.2f\n", i, y[i]);
    }

    // Guardar CSV con tiempo
    FILE *fres;
    struct stat st;
    int existe = stat("resultadosOpenMP.csv", &st) == 0;

    fres = fopen("resultadosOpenMP.csv", "a");
    if (fres) {
        if (!existe) {
            // Escribir cabecera si el archivo no existía
            fprintf(fres, "Metodo,N_filas,Hilos,Tiempo_segundos\n");
        }
        fprintf(fres, "OpenMP,%d,%d,%.6f\n", filas, omp_get_max_threads(), tiempo);

        fclose(fres);
    } else {
        perror("Error al guardar resultadosOpenMP.csv");
    }
}
