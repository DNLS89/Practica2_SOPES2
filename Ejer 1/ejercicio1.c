#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/*
 * Ejercicio 1: Comunicación mediante pipes.
 *
 * El programa utiliza dos procesos:
 * - Padre: representa al cliente
 * - Hijo: representa al sistema que procesa la información
 *
 * Se utilizan dos pipes para permitir comunicación en ambos sentidos:
 * - pipe_padre_hijo: padre -> hijo
 * - pipe_hijo_padre: hijo -> padre
 *
 * También se utiliza un indicador para que el hijo pueda identificar
 * qué tipo de información está recibiendo
 */

/*
 * Invierte una cadena carácter por carácter
 */
void invertir_cadena(char *cadena, char *invertida)
{
    int longitud = 0;

    // Calculamos la longitud de la cadena
    while (cadena[longitud] != '\0') {
        longitud++;
    }

    // Copiamos los caracteres comenzando desde el último
    for (int i = 0; i < longitud; i++) {
        invertida[i] = cadena[longitud - 1 - i];
    }

    invertida[longitud] = '\0';
}

int main()
{
    int pipe_padre_hijo[2];
    int pipe_hijo_padre[2];

    // Creamos las dos tuberías
    if (pipe(pipe_padre_hijo) == -1 || pipe(pipe_hijo_padre) == -1) {
        perror("Error al crear las tuberias");
        return 1;
    }

    // Creamos el proceso hijo
    pid_t pid = fork();

    if (pid == -1) {
        perror("Error al crear el proceso hijo");
        return 1;
    }

    // =====================================================
    // PROCESO PADRE
    // =====================================================
    if (pid > 0) {

        // El padre solamente escribe en el primer pipe y solo lee del segundo
        close(pipe_padre_hijo[0]);
        close(pipe_hijo_padre[1]);

        int opcion;

        printf("=== SISTEMA DE PAGO ===\n");
        printf("1. Verificar estado del canal\n");
        printf("2. Omitir verificacion\n");
        printf("Seleccione una opcion: ");
        scanf("%d", &opcion);

        /*
         * Si el usuario quiere verificar el canal, enviamos el indicador 1
         */
        if (opcion == 1) {

            int indicador = 1;

            write(pipe_padre_hijo[1], &indicador, sizeof(indicador));

            char mensaje[100];
            char respuesta[100];

            printf("\nEscriba una palabra para verificar el canal: ");
            scanf("%s", mensaje);

            // Enviamos el mensaje al hijo
            write(pipe_padre_hijo[1], mensaje, sizeof(mensaje));

            // Esperamos la respuesta del hijo
            read(pipe_hijo_padre[0], respuesta, sizeof(respuesta));

            printf("Confirmacion del canal: %s\n", respuesta);
        }
        else if (opcion != 2) {

            printf("Opcion no valida.\n");

            close(pipe_padre_hijo[1]);
            close(pipe_hijo_padre[0]);

            wait(NULL);
            return 0;
        }

        /*
         * Ahora comienza la fase de pago
         * El indicador 2 informa al hijo que el siguiente dato
         * corresponde al número de tarjeta
         */
        int indicador = 2;

        write(pipe_padre_hijo[1], &indicador, sizeof(indicador));

        int tarjeta;

        do {
            printf("\nIngrese su numero de tarjeta (1000 - 9999): ");
            scanf("%d", &tarjeta);

            if (tarjeta < 1000 || tarjeta > 9999) {
                printf("Numero fuera de rango.\n");
            }

        } while (tarjeta < 1000 || tarjeta > 9999);

        // Enviamos el número de tarjeta al hijo
        write(pipe_padre_hijo[1], &tarjeta, sizeof(tarjeta));

        // Recibimos el resultado del hijo
        char resultado[50];

        read(pipe_hijo_padre[0], resultado, sizeof(resultado));

        printf("\nResultado final: %s\n", resultado);

        // Cerramos los pipes
        close(pipe_padre_hijo[1]);
        close(pipe_hijo_padre[0]);

        // Esperamos que termine el hijo
        wait(NULL);
    }

    // =====================================================
    // PROCESO HIJO
    // =====================================================
    else {

        // El hijo solamente lee del primer pipe y solo escribe en el segundo
        close(pipe_padre_hijo[1]);
        close(pipe_hijo_padre[0]);

        int indicador;

        /*
         * El hijo permanece leyendo indicadores hasta recibir
         * la instrucción correspondiente al pago
         */
        while (read(pipe_padre_hijo[0], &indicador, sizeof(indicador)) > 0) {

            // ---------------------------------------------
            // Indicador 1: verificar el canal
            // ---------------------------------------------
            if (indicador == 1) {

                char mensaje[100];
                char invertida[100];

                // Recibimos el mensaje enviado por el padre
                read(pipe_padre_hijo[0], mensaje, sizeof(mensaje));

                // Invertimos el mensaje
                invertir_cadena(mensaje, invertida);

                // Enviamos la respuesta al padre
                write(pipe_hijo_padre[1], invertida, sizeof(invertida));
            }

            // ---------------------------------------------
            // Indicador 2: procesar el pago
            // ---------------------------------------------
            else if (indicador == 2) {

                int tarjeta;

                // Recibimos el número de tarjeta.
                read(pipe_padre_hijo[0], &tarjeta, sizeof(tarjeta));

                char resultado[50];

                // Verificamos si el número es par o impar
                if (tarjeta % 2 == 0) {
                    sprintf(resultado, "PAGO_APROBADO");
                } else {
                    sprintf(resultado, "PAGO_RECHAZADO");
                }

                // Enviamos el resultado al padre
                write(pipe_hijo_padre[1], resultado, sizeof(resultado));

                break;
            }
        }

        // Cerramos los pipes
        close(pipe_padre_hijo[0]);
        close(pipe_hijo_padre[1]);
    }

    return 0;
}
