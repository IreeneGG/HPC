#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <sys/stat.h>

 
void guardarResultadoMPI(const char *nombre_archivo, int filas, int procesos, double tiempo);
int leerLineaCSV(const char *linea, float **valores_f, int **valores_i, int es_float);


int main(int argc, char *argv[]) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 3) {
        if (rank == 0) fprintf(stderr, "Uso: %s matriz.csv vector.csv\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    float *val = NULL;
    int *col = NULL, *row_ptr = NULL;
    float *x = NULL;
    float *y = NULL;
    int nnz = 0, filas = 0, tam_x = 0;

    if (rank == 0) {
        FILE *fmat = fopen(argv[1], "r");
        if (!fmat) {
            perror("Error al abrir matriz.csv");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        char linea[32768];
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
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        fgets(linea, sizeof(linea), fvec);
        tam_x = leerLineaCSV(linea, &x, NULL, 1);
        fclose(fvec);

        if (tam_x <= 0 || filas <= 0) {
            fprintf(stderr, "Error: Vector o matriz vacíos\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    // Broadcast de tamaños
    MPI_Bcast(&nnz, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&filas, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&tam_x, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Asignar memoria para todos
    if (rank != 0) {
        val = (float *)malloc(nnz * sizeof(float));
        col = (int *)malloc(nnz * sizeof(int));
        row_ptr = (int *)malloc((filas + 1) * sizeof(int));
        x = (float *)malloc(tam_x * sizeof(float));
    }
    y = (float *)calloc(filas, sizeof(float));

    // Broadcast de datos
    MPI_Bcast(val, nnz, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(col, nnz, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(row_ptr, filas + 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(x, tam_x, MPI_FLOAT, 0, MPI_COMM_WORLD);

    int filas_local = filas / size;
    int inicio = rank * filas_local;
    int fin = (rank == size - 1) ? filas : inicio + filas_local;

    double t0 = MPI_Wtime();

    for (int i = inicio; i < fin; i++) {
        float suma = 0.0;
        for (int j = row_ptr[i]; j < row_ptr[i + 1]; j++) {
            suma += val[j] * x[col[j]];
        }
        y[i] = suma;
    }

    float *resultado = NULL;
    if (rank == 0) resultado = (float *)malloc(filas * sizeof(float));

    // Recopilar resultados
    MPI_Reduce(rank == 0 ? MPI_IN_PLACE : y, y, filas, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

    double t1 = MPI_Wtime();

    if (rank == 0) {
        printf("Resultado y = A * x:\n");
        for (int i = 0; i < filas; i++) {
            printf("y[%d] = %.2f\n", i, y[i]);
        }

        guardarResultadoMPI("resultadosMPI.csv", filas, size, t1 - t0);
    }

    free(val); free(col); free(row_ptr); free(x); free(y);
    MPI_Finalize();
    return 0;
}

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

void guardarResultadoMPI(const char *nombre_archivo, int filas, int procesos, double tiempo) {
    struct stat buffer;
    int archivo_existe = (stat(nombre_archivo, &buffer) == 0);

    FILE *fres = fopen(nombre_archivo, "a");
    if (!fres) {
        perror("Error al abrir el archivo de resultados");
        return;
    }

    if (!archivo_existe) {
        // Escribimos cabecera solo si el archivo no existía
        fprintf(fres, "Metodo,N_filas,Núcleos,Tiempo_segundos\n");
    }

    // Escribimos los resultados
    fprintf(fres, "MPI,%d,%d,%.6f\n", filas, procesos, tiempo);
    fclose(fres);
}