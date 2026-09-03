#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/*
 * Ejercicio 2: Productor-Consumidor utilizando pipes.
 *
 * El proceso padre funciona como productor y registra los 20 pedidos
 *
 * Los dos procesos hijos representan las estaciones de empaque
 *
 * Ambos hijos utilizan el mismo pipe para recibir los pedidos
 *
 * Cada estación toma el siguiente pedido disponible y mantiene su
 * propio contador de pedidos y total de unidades procesadas
 */

int main()
{
    int pipe_pedidos[2];

    // Creamos la tubería que transportará los pedidos
    if (pipe(pipe_pedidos) == -1) {
        perror("Error al crear el pipe");
        return 1;
    }

    // Creamos el primer hijo
    pid_t hijo1 = fork();

    if (hijo1 == -1) {
        perror("Error al crear el primer hijo");
        return 1;
    }

    // Creamos el segundo hijo solamente desde el padre
    pid_t hijo2 = -1;

    if (hijo1 > 0) {
        hijo2 = fork();

        if (hijo2 == -1) {
            perror("Error al crear el segundo hijo");
            return 1;
        }
    }

    // =====================================================
    // PROCESO PADRE - PRODUCTOR
    // =====================================================
    if (hijo1 > 0 && hijo2 > 0) {

        // El padre escribe en el pipe, por lo que no necesita leer
        close(pipe_pedidos[0]);

        printf("=== REGISTRO DE PEDIDOS ===\n");

        /*
         * El encargado ingresa los 20 pedidos antes de comenzar
         * a enviarlos por la banda transportadora
         */
        for (int i = 0; i < 20; i++) {

            int unidades;

            do {
                printf("Pedido %d - unidades (1-100): ", i + 1);
                scanf("%d", &unidades);

                if (unidades < 1 || unidades > 100) {
                    printf("Valor invalido. Debe estar entre 1 y 100.\n");
                }

            } while (unidades < 1 || unidades > 100);

            /*
             * Guardamos temporalmente cada pedido en el pipe
             * El dato se escribe inmediatamente después de ingresarlo
             */
            write(pipe_pedidos[1], &unidades, sizeof(unidades));
        }

        /*
         * Al terminar los 20 pedidos, cerramos el extremo de escritura
         * Esto permite que los hijos sepan que ya no quedan pedidos
         */
        close(pipe_pedidos[1]);

        // Esperamos a que ambas estaciones terminen
        waitpid(hijo1, NULL, 0);
        waitpid(hijo2, NULL, 0);
    }

    // =====================================================
    // PROCESO HIJO - ESTACIÓN 1
    // =====================================================
    else if (hijo1 == 0) {

        // La estación solamente necesita leer del pipe
        close(pipe_pedidos[1]);

        int pedido;
        int cantidad_pedidos = 0;
        int total_unidades = 0;

        /*
         * La estación toma pedidos mientras haya datos disponibles
         * Cuando read() devuelve 0 significa que el padre cerró
         * el extremo de escritura y ya no quedan pedidos
         */
        while (read(pipe_pedidos[0], &pedido, sizeof(pedido)) > 0) {

            cantidad_pedidos++;
            total_unidades += pedido;
        }

        printf("\n=== ESTACION DE EMPAQUE 1 ===\n");
        printf("Pedidos procesados: %d\n", cantidad_pedidos);
        printf("Total de unidades despachadas: %d\n", total_unidades);

        close(pipe_pedidos[0]);
        exit(0);
    }

    // =====================================================
    // PROCESO HIJO - ESTACIÓN 2
    // =====================================================
    else if (hijo2 == 0) {

        // La estación solamente necesita leer del pipe
        close(pipe_pedidos[1]);

        int pedido;
        int cantidad_pedidos = 0;
        int total_unidades = 0;

        /*
         * Esta estación compite con la estación 1 por los pedidos
         * Cada read() obtiene el siguiente pedido disponible
         */
        while (read(pipe_pedidos[0], &pedido, sizeof(pedido)) > 0) {

            cantidad_pedidos++;
            total_unidades += pedido;
        }

        printf("\n=== ESTACION DE EMPAQUE 2 ===\n");
        printf("Pedidos procesados: %d\n", cantidad_pedidos);
        printf("Total de unidades despachadas: %d\n", total_unidades);

        close(pipe_pedidos[0]);
        exit(0);
    }

    return 0;
}
