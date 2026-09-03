/*
 * Problema 3 - Tuberías con FIFO
 * Javier Eduardo Ixcolín Orozco
 * 202132163
 */

#include <stdio.h>     // Contiene printf(), scanf(), fgets
#include <stdlib.h>    // Contiene malloc(), free(), exit()
#include <unistd.h>    // Contiene fork(), pipe(), read(), write(), close()
#include <fcntl.h>     // Contiene O_RDONLY, O_WRONLY, O_CREAT, O_TRUNC
#include <sys/stat.h>  // Contiene S_IRUSR, S_IWUSR, S_IRGRP, S_IROTH
#include <sys/types.h> // Define tipos de datos como pid_t, size_t
#include <time.h>      // Contiene time(), localtime(), strftime()
#include <string.h>    // Contiene strlen(), strcpy(), strcat()

#define FIFO_PATH "/tmp/fifo_empresa"

// Estructura para almacenar la información del reporte
typedef struct
{
    char branch[32];  // Sucursal
    double amount;    // Total de pagos
    int closeCommand; // Comando de cierre de comunicación
} Report;

// Prototipos de funciones
void getCurrentTime(char *buffer, size_t size); // Función para obtener la hora actual formateada

int main()
{
    // Crear el pipe con nombre (si ya existe, mkfifo falla pero se ignora con EEXIST)
    mkfifo(FIFO_PATH, 0666);

    printf("\t.: Centro de Operaciones Activo :.\n");
    printf("Esperando reportes de las sucursales en %s...\n\n", FIFO_PATH);

    double gran_total = 0.0;  // Variable para almacenar el total acumulado de pagos
    int received_reports = 0; // Contador de reportes recibidos
    Report r;                 // Variable para almacenar el reporte recibido

    int fd_fifo = open(FIFO_PATH, O_RDWR); // Abrir el FIFO para lectura y escritura
    if (fd_fifo == -1)
    {
        perror("Error al abrir el FIFO en el centro de operaciones");
        return 1;
    }

    char hour[16]; // Buffer para almacenar la hora

    while (1)
    {
        ssize_t bytes = read(fd_fifo, &r, sizeof(Report)); // Leer el reporte desde el FIFO
        if (bytes > 0)
        {
            getCurrentTime(hour, sizeof(hour)); // Obtener la hora actual

            // Si se recibe el comando especial de cierre, salir del bucle
            if (r.closeCommand == 1 || strcmp(r.branch, "CLOSE") == 0)
            {
                printf("[%s] Comando de cierre recibido. Finalizando operaciones...\n", hour);
                break;
            }

            received_reports++;     // Incrementar el contador de reportes recibidos
            gran_total += r.amount; // Acumular el total de pagos

            // Imprimir el reporte recibido
            printf("[%s] Reporte recibido de: %-15s | Monto: Q%.2f\n", hour, r.branch, r.amount);
        }
    }

    // Resumen al cerrar el día
    printf("\n\t.: Resumen del Día :.\n");
    printf("-> Sucursales registradas: %d\n", received_reports);
    printf("-> Total acumulado del día: Q%.2f\n", gran_total);

    close(fd_fifo);    // Cerrar el descriptor de lectura
    unlink(FIFO_PATH); // Eliminar el FIFO del sistema de archivos
}

// Función para obtener la hora actual formateada
void getCurrentTime(char *buffer, size_t size)
{
    time_t rawtime;                               // Variable para almacenar el tiempo en formato de tiempo crudo
    struct tm *timeinfo;                          // Estructura para almacenar la información de tiempo desglosada
    time(&rawtime);                               // Obtiene el tiempo actual
    timeinfo = localtime(&rawtime);               // Convierte el tiempo crudo a tiempo local
    strftime(buffer, size, "%H:%M:%S", timeinfo); // Formatea la hora actual y la almacena en el buffer
}