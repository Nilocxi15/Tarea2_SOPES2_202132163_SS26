/*
 * Problema 3 - Tuberías con FIFO
 * Javier Eduardo Ixcolín Orozco
 * 202132163
 */

#include <stdio.h>    // Contiene printf(), scanf(), fgets
#include <stdlib.h>   // Contiene malloc(), free(), exit()
#include <unistd.h>   // Contiene fork(), pipe(), read(), write(), close()
#include <fcntl.h>    // Contiene O_RDONLY, O_WRONLY, O_CREAT,
#include <sys/stat.h> // Contiene S_IRUSR, S_IWUSR, S_IRGRP, S_IROTH
#include <string.h>   // Contiene strlen(), strcpy(), strcat()

#define FIFO_PATH "/tmp/fifo_empresa"

// Estructura para almacenar la información del reporte
typedef struct
{
    char branch[32];  // Sucursal
    double amount;    // Total de pagos
    int closeCommand; // Comando de cierre de comunicación
} Report;

int main()
{
    // Asegurar que el FIFO exista antes de abrirlo
    mkfifo(FIFO_PATH, 0666);

    printf("\t.: Sucursal Activa :.\n");
    printf("Enviando reportes al centro de operaciones en %s...\n\n", FIFO_PATH);

    int fd = open(FIFO_PATH, O_WRONLY); // Abrir el FIFO para escritura
    if (fd == -1)
    {
        perror("Error al abrir el FIFO hacia el centro de operaciones");
        return 1;
    }

    Report r;                      // Variable para almacenar el reporte a enviar
    memset(&r, 0, sizeof(Report)); // Inicializar la estructura del reporte

    printf("Ingrese nombre de sucursal (o 'CLOSE' para finalizar el día): ");
    char buffer[64];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        return 0;                      // Leer el nombre de la sucursal desde la entrada estándar
    buffer[strcspn(buffer, "\n")] = 0; // Eliminar el salto de línea al final de la cadena

    if (strcmp(buffer, "CLOSE") == 0)
    {
        r.closeCommand = 1;
        snprintf(r.branch, sizeof(r.branch), "CLOSE");
        write(fd, &r, sizeof(Report)); // Enviar el comando de cierre al centro de operaciones
        printf("Comando de cierre enviado al centro de operaciones.\n");
    }
    else
    {
        snprintf(r.branch, sizeof(r.branch), "%s", buffer); // Copiar el nombre de la sucursal al reporte

        printf("Ingrese el monto total de pagos procesados: ");
        if (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            r.amount = strtod(buffer, NULL); // Convertir la entrada a double y almacenarla en el reporte
        }
        r.closeCommand = 0; // Indicar que no es un comando de cierre

        write(fd, &r, sizeof(Report)); // Enviar el reporte al centro de operaciones
        printf("Reporte enviado al centro de operaciones.\n");
    }

    close(fd); // Cerrar el descriptor de archivo del FIFO
    return 0;
}