/*
 * Problema 2 - Productor - Consumidor
 * Javier Eduardo Ixcolín Orozco
 * 202132163
 */

#include <unistd.h>    // Contiene fork(), pipe(), read(), write(), close()
#include <sys/types.h> // Define tipos de datos como pid_t, size_t
#include <sys/wait.h>  // Contiene wait() y waitpid()
#include <stdio.h>     // Contiene printf(), scanf(), fgets
#include <stdlib.h>    // Contiene exit(), strtol()

// Prototipos de funciones
int readSafeInt(const char *message, int min, int max); // Función para leer un número entero de manera segura

int main()
{
    int stock[20]; // Arreglo para almacenar las unidades de camisas de un pedido

    printf("\t\t.: Sistema de Control de Pedidos de Camisas :.\n");
    printf("Ingrese las unidades de camisas para cada pedido (20 pedidos en total):\n");

    for (int i = 0; i < 20; i++)
    {
        char message[100];
        snprintf(message, sizeof(message), "Ingrese las unidades de camisas para el pedido #%d: ", i + 1);
        stock[i] = readSafeInt(message, 1, 100); // Leer las unidades de camisas de manera segura
    }

    // Crear el pipe para la comunicación entre procesos
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        perror("Error al crear el pipe");
        return 1;
    }

    // Código relacionado al hijo 1
    pid_t pid1 = fork(); // Crear el primer proceso hijo
    if (pid1 == -1)
    {
        perror("Error al crear el proceso hijo");
        return 1;
    }
    if (pid1 == 0)
    {
        // Código del proceso hijo 1
        close(pipefd[1]); // Cerrar el extremo de escritura del pipe en el hijo

        int receivedStock = 0; // Variable para almacenar las unidades de camisas recibidas del padre
        int totalStock = 0;    // Variable para acumular las unidades de camisas recibidas
        int orderCount = 0;    // Variable para contar la cantidad de pedidos procesados

        // Bucle para leer los pedidos enviados por el proceso padre a través del pipe
        // read() bloquea esperando datos.
        // Devuelve sizeof(int) si leyó un pedido.
        // Devuelve 0 automáticamente cuando el padre hace close(pipefd[1]) y la banda se vacía.
        while (read(pipefd[0], &receivedStock, sizeof(int)) > 0)
        {
            orderCount++;
            totalStock += receivedStock;
        }

        // Al salir del while, significa que la tubería se cerró.
        printf("\n[Estación 1] Reporte final:\n");
        printf(" -> Pedidos procesados: %d\n", orderCount);
        printf(" -> Total unidades despachadas: %d camisas\n", totalStock);

        close(pipefd[0]); // Cerrar el extremo de lectura del pipe en el hijo
        exit(0);          // Salir del proceso hijo
    }

    // Código relacionado al hijo 2
    pid_t pid2 = fork(); // Crear el segundo proceso hijo
    if (pid2 == -1)
    {
        perror("Error al crear el proceso hijo");
        return 1;
    }
    if (pid2 == 0)
    {
        // Código del proceso hijo 2
        close(pipefd[1]); // Cerrar el extremo de escritura del pipe en el hijo

        int receivedStock = 0; // Variable para almacenar las unidades de camisas recibidas del padre
        int totalStock = 0;    // Variable para acumular las unidades de camisas recibidas
        int orderCount = 0;    // Variable para contar la cantidad de pedidos procesados

        // Bucle para leer los pedidos enviados por el proceso padre a través del pipe
        // read() bloquea esperando datos.
        // Devuelve sizeof(int) si leyó un pedido.
        // Devuelve 0 automáticamente cuando el padre hace close(pipefd[1]) y la banda se vacía.
        while (read(pipefd[0], &receivedStock, sizeof(int)) > 0)
        {
            orderCount++;
            totalStock += receivedStock;
        }

        // Al salir del while, significa que la tubería se cerró.
        printf("\n[Estación 2] Reporte final:\n");
        printf(" -> Pedidos procesados: %d\n", orderCount);
        printf(" -> Total unidades despachadas: %d camisas\n", totalStock);

        close(pipefd[0]); // Cerrar el extremo de lectura del pipe en el hijo
        exit(0);          // Salir del proceso hijo
    }

    // Envío de los pedidos a los procesos hijos mediante el pipe
    close(pipefd[0]); // Cerrar el extremo de lectura del pipe en el padre

    for (int i = 0; i < 20; i++)
    {
        write(pipefd[1], &stock[i], sizeof(int));
    }

    close(pipefd[1]); // Cerrar el extremo de escritura del pipe en el padre

    wait(NULL); // Esperar a que el primer proceso hijo termine
    wait(NULL); // Esperar a que el segundo proceso hijo termine

    return 0;
}

// Función para leer un número entero de manera segura
int readSafeInt(const char *message, int min, int max)
{
    char buffer[64];
    char *endptr;
    long value;

    while (1)
    {
        printf("%s", message);

        // 1. Leer una línea completa del teclado de forma segura
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        {
            continue; // Si hay un error al leer, continuar el bucle
        }

        // 2. Intentar convertir la cadena a entero (base 10)
        // endptr apuntará al primer carácter que no sea un dígito
        value = strtol(buffer, &endptr, 10);

        // 3. Validar errores de conversión:
        // Si endptr == buffer: El usuario no escribió ningún número
        // Si *endptr != '\n' && *endptr != '\0': El usuario escribió letras después del número
        if (endptr == buffer || (*endptr != '\n' && *endptr != '\0'))
        {
            printf("Error: Entrada inválida. Por favor, ingrese un número entero.\n");
            continue; // Volver a pedir la entrada
        }

        // 4. Validar el rango del número ingresado
        if (value < min || value > max)
        {
            printf("Error: El número debe estar entre %d y %d.\n", min, max);
            continue; // Volver a pedir la entrada
        }

        return (int)value;
    }

    // Línea inalcanzable; evita la advertencia de "control reaches end of non-void function"
    return 0;
}