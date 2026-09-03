#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

/*
 * Problema 3: Comunicación mediante FIFO
 *
 * Este programa representa una sucursal
 *
 * La sucursal puede:
 * 1. Enviar un reporte de pagos al centro
 * 2. Enviar el mensaje "cerrar" para indicar que terminó el día
 *
 * No se utiliza fork() porque centro.c y sucursal.c son
 * programas independientes
 */

#define FIFO "pagos_fifo"

int main()
{
    int opcion;

    printf("=== SUCURSAL ===\n");
    printf("1. Enviar reporte\n");
    printf("2. Cerrar el dia\n");
    printf("Seleccione una opcion: ");
    scanf("%d", &opcion);

    /*
     * Primero comprobamos que la opción sea válida
     */
    if (opcion != 1 && opcion != 2) {
        printf("\nOpcion no valida.\n");
        return 1;
    }

    /*
     * Abrimos la FIFO para enviar información al centro
     */
    int fd = open(FIFO, O_WRONLY);

    if (fd == -1) {
        perror("Error al abrir la FIFO");
        return 1;
    }

    /*
     * Opción 1: enviar un reporte de pagos
     */
    if (opcion == 1) {

        char sucursal[100];
        int pagos;

        printf("\nIngrese el nombre de la sucursal: ");
        scanf("%99s", sucursal);

        do {
            printf("Ingrese el total de pagos procesados: ");
            scanf("%d", &pagos);

            if (pagos < 0) {
                printf("La cantidad no puede ser negativa.\n");
            }

        } while (pagos < 0);

        char mensaje[200];

        /*
         * Construimos el reporte que será enviado al centro
         */
        snprintf(mensaje, sizeof(mensaje),
                 "Sucursal: %s\nPagos: %d",
                 sucursal, pagos);

        write(fd, mensaje, sizeof(mensaje));

        printf("\nReporte enviado correctamente.\n");
    }

    /*
     * Opción 2: enviar la orden de cierre
     */
    else {
        char mensaje[] = "cerrar";

        write(fd, mensaje, sizeof(mensaje));

        printf("\nSe envio la orden de cierre al centro.\n");
    }

    close(fd);

    return 0;
}