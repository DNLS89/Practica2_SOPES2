#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

/*
 * Problema 3: Comunicación entre programas mediante una FIFO
 *
 * Este programa representa el centro de operaciones
 * Permanece activo durante el día esperando reportes de las sucursales
 *
 * La FIFO permite que centro.c y sucursal.c se comuniquen aunque
 * sean programas independientes y se ejecuten desde terminales
 * diferentes
 */

#define FIFO "pagos_fifo"

int main()
{
    /*
     * Creamos la FIFO si todavía no existe
     */
    if (mkfifo(FIFO, 0666) == -1) {
        printf("La FIFO ya existe, se utilizara la existente.\n");
    }

    printf("=== CENTRO DE OPERACIONES ===\n");
    printf("Esperando reportes de las sucursales...\n");

    int total_pagos = 0;

    /*
     * El centro permanece activo y vuelve a abrir la FIFO
     * cada vez que una sucursal termina de enviar su reporte
     */
    while (1) {

        /*
         * Abrimos la FIFO para lectura
         * Si no hay una sucursal conectada, el programa espera
         */
        int fd = open(FIFO, O_RDONLY);

        if (fd == -1) {
            perror("Error al abrir la FIFO");
            return 1;
        }

        char mensaje[200];
        ssize_t bytes_leidos;

        /*
         * Leemos los datos enviados por la sucursal
         */
        while ((bytes_leidos = read(fd, mensaje, sizeof(mensaje) - 1)) > 0) {

            /*
             * Agregamos el carácter final para tratar los datos
             * recibidos como una cadena de texto
             */
            mensaje[bytes_leidos] = '\0';

            /*
             * Si recibimos "cerrar", terminamos el día
             */
            if (mensaje[0] == 'c' &&
                mensaje[1] == 'e' &&
                mensaje[2] == 'r' &&
                mensaje[3] == 'r' &&
                mensaje[4] == 'a' &&
                mensaje[5] == 'r' &&
                mensaje[6] == '\0') {

                close(fd);

                printf("\n=== RESUMEN DEL DIA ===\n");
                printf("Total de pagos procesados: %d\n", total_pagos);

                unlink(FIFO);
                return 0;
            }

            /*
             * El formato esperado es:
             *
             * Sucursal: nombre
             * Pagos: cantidad
             */
            int pagos;
            char sucursal[100];

            if (sscanf(mensaje, "Sucursal: %99[^\n]\nPagos: %d",
                       sucursal, &pagos) == 2) {

                time_t ahora = time(NULL);
                struct tm *hora = localtime(&ahora);

                printf("\nReporte recibido\n");
                printf("Sucursal: %s\n", sucursal);
                printf("Pagos procesados: %d\n", pagos);
                printf("Hora de llegada: %02d:%02d:%02d\n",
                       hora->tm_hour,
                       hora->tm_min,
                       hora->tm_sec);

                total_pagos += pagos;
            }
        }

        /*
         * La sucursal cerró su extremo de la FIFO
         * Cerramos también el nuestro y volvemos a esperar
         * por el siguiente reporte
         */
        close(fd);
    }

    return 0;
}