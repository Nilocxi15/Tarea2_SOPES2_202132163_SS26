/*
 * Problema 1 - Comunicación Unidireccional y Bidireccional
 * Javier Eduardo Ixcolín Orozco
 * 202132163
 */

#include <unistd.h>    // Contiene fork(), pipe(), read(), write(), close()
#include <sys/types.h> // Define tipos de datos como pid_t, size_t
#include <sys/wait.h>  // Contiene wait() y waitpid()
#include <stdio.h>     // Contiene printf(), scanf(), fgets
#include <stdlib.h>    // Contiene exit(), strtol()

// Prototipos de funciones
int verification();                                     // Verificacion de la comunicación entre procesos mediante pipes
int payProcess();                                       // Función para procesar el pago de la tarjeta
int readSafeInt(const char *message, int min, int max); // Función para leer un número entero de manera segura

int main()
{
    int option;         // Variable para almacenar la opción seleccionada por el usuario
    int testResult = 1; // Variable para almacenar el resultado de la verificación

    printf("\t\t.: Bienvenido al Sistema de Cancelación de Servicios :.\n");
    printf("¿Desea verificar la comunicación del canal?\n");
    printf("1. Sí (Recomendado)\n");
    printf("2. No\n");
    printf("3. Salir\n");
    option = readSafeInt("Seleccione una opción: ", 1, 3); // Leer la opción del usuario de manera segura

    if (option == 3)
    {
        return 0; // Salir del programa si el usuario selecciona la opción 3
    }

    if (option == 1)
    {
        printf("\n\t\t.: Verificación del Canal :.\n");
        testResult = verification(); // Llamar a la función de verificación y almacenar el resultado en testResult

        if (testResult == 1)
        {
            printf("Error: La comunicación entre procesos mediante pipes falló.\n");
            printf("¿Desea continuar con la ejecución del programa?\n");
            printf("1. Sí\n");
            printf("2. No\n");
            option = readSafeInt("Seleccione una opción: ", 1, 2); // Leer la opción del usuario de manera segura
            if (option == 2)
            {
                return 0; // Salir del programa si el usuario selecciona la opción 2
            }
        }
    }

    printf("\n\t\t.: Proceso de Pago :.\n");
    payProcess(); // Llamar a la función de procesamiento de pago

    return 0;
}

// Función para verificar la comunicación entre procesos mediante pipes
int verification()
{
    int pipeFS[2]; // Pipe para comunicación del padre al hijo
    int pipeSF[2]; // Pipe para comunicación del hijo al padre

    // Crear los pipes
    if (pipe(pipeFS) == -1 || pipe(pipeSF) == -1)
    { // Al regresar -1, significa que hubo un error al crear los pipes
        perror("Error al crear los pipes");
        return 1;
    }

    pid_t pid = fork(); // Crear un proceso hijo

    if (pid == -1)
    {
        perror("Error al crear el proceso hijo");
        return 1;
    }
    else if (pid == 0)
    {
        // Código del proceso hijo
        close(pipeFS[1]); // Pipe padre - cierra el extremo de escritura
        close(pipeSF[0]); // Pipe hijo - cierra el extremo de lectura

        // Leer la cadena enviada por el proceso padre
        char message[21];
        read(pipeFS[0], message, sizeof(message)); // Leer la cadena enviada por el proceso hijo

        // Inversión de la cadena recibida del proceso padre
        int length = 0;
        char reversedMessage[21];

        // Obtención de la longitud de la cadena
        while (message[length] != '\0' && length < 20)
        {
            length++;
        }

        // Inversión de la cadena
        for (int i = 0; i < length; i++)
        {
            reversedMessage[i] = message[length - 1 - i];
        }

        reversedMessage[length] = '\0'; // Agregar el carácter nulo al final de la cadena invertida

        printf("Respuesta del canal: %s\n", reversedMessage); // Imprimir la cadena invertida

        close(pipeFS[0]); // Cerrar el extremo de lectura del pipe padre
        close(pipeSF[1]); // Cerrar el extremo de escritura del pipe hijo

        exit(0); // Salir del proceso hijo
    }
    else
    {
        // Código del proceso padre
        close(pipeFS[0]); // Pipe padre - cierra el extremo de lectura
        close(pipeSF[1]); // Pipe hijo - cierra el extremo de escritura

        // Enviar una cadena al proceso hijo
        char message[21];
        printf("Por favor ingrese cualquier cadena no mayor a 20 caracteres para verificar si el canal está activo.\n");
        fgets(message, sizeof(message), stdin); // Leer la cadena del usuario de manera segura

        // Quitar el salto de línea al final de la cadena si existe
        int len = 0;
        while (message[len] != '\0')
        {
            if (message[len] == '\n')
            {
                message[len] = '\0';
                break;
            }
            len++;
        }

        write(pipeFS[1], message, sizeof(message)); // Enviar la cadena al proceso padre

        wait(NULL);       // Esperar a que el proceso hijo termine
        close(pipeFS[1]); // Cerrar el extremo de escritura del pipe padre
        close(pipeSF[0]); // Cerrar el extremo de lectura del pipe hijo
    }

    return 0;
}

// Función para procesar el pago de la tarjeta
int payProcess()
{
    int pipeFS[2]; // Pipe para comunicación del padre al hijo
    int pipeSF[2]; // Pipe para comunicación del hijo al padre

    // Crear los pipes
    if (pipe(pipeFS) == -1 || pipe(pipeSF) == -1)
    {
        perror("Error al crear los pipes");
        return 1;
    }

    pid_t pid = fork(); // Crear un proceso hijo

    if (pid == -1)
    {
        perror("Error al crear el proceso hijo");
        return 1;
    }
    else if (pid == 0)
    {
        // Código del proceso hijo
        close(pipeFS[1]); // Pipe padre - cierra el extremo de escritura
        close(pipeSF[0]); // Pipe hijo - cierra el extremo de lectura

        int cardNumber;                                   // Variable para almacenar el número de tarjeta recibido del proceso padre
        read(pipeFS[0], &cardNumber, sizeof(cardNumber)); // Leer el número de tarjeta enviado por el proceso padre

        int residue = cardNumber % 2;

        // Verificación de si el número de tarjeta es par o impar
        if (residue == 0)
        {
            write(pipeSF[1], "PAGO_APROBADO", 14); // Enviar mensaje de pago aprobado al proceso padre
        }
        else
        {
            write(pipeSF[1], "PAGO_RECHAZADO", 14); // Enviar mensaje de pago rechazado al proceso padre
        }

        close(pipeFS[0]); // Cerrar el extremo de lectura del pipe padre
        close(pipeSF[1]); // Cerrar el extremo de escritura del pipe hijo
        exit(0);          // Salir del proceso hijo
    }
    else
    {
        // Codigo del proceso padre
        close(pipeFS[0]); // Pipe padre - cierra el extremo de lectura
        close(pipeSF[1]); // Pipe hijo - cierra el extremo de escritura

        int cardNumber = readSafeInt("Ingrese el número de tarjeta (4 dígitos): ", 1000, 9999); // Leer el número de tarjeta de manera segura
        write(pipeFS[1], &cardNumber, sizeof(cardNumber));                                      // Enviar el número de tarjeta al proceso hijo

        // Leer la respuesta del proceso hijo
        char response[14];                            // Variable para almacenar la respuesta del proceso hijo
        read(pipeSF[0], &response, sizeof(response)); // Leer la respuesta del proceso hijo

        printf("Respuesta del proceso de pago: %s\n", response); // Imprimir la respuesta del proceso hijo

        wait(NULL);       // Esperar a que el proceso hijo termine
        close(pipeFS[1]); // Cerrar el extremo de escritura del pipe padre
        close(pipeSF[0]); // Cerrar el extremo de lectura del pipe hijo
    }

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