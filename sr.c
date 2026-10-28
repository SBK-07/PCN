
#include <stdio.h>

#define N 4
#define WINDOW 4

int main(void) {
    int ack[N] = {0};
    int received[N] = {0};
    int lost_once = 0;
    int base = 0;

    printf("Selective Repeat Protocol\n");
    printf("Total packets: %d, Window size: %d\n\n",
           N, WINDOW);

    /* Initially send all packets in the window */
    for (int i = 0; i < N && i < WINDOW; i++) {
        printf("Sender: Sending packet %d\n", i + 1);

        if (i == 1 && !lost_once) {
            printf("Network: Packet 2 lost!\n");
            lost_once = 1;
            continue;
        }

        received[i] = 1;
        ack[i] = 1;

        printf("Receiver: Packet %d received and buffered\n",
               i + 1);
        printf("Receiver: ACK %d\n", i + 1);
    }

    /* Deliver buffered packets in order */
    while (base < N && received[base]) {
        printf("Receiver: Delivering packet %d to application\n",
               base + 1);
        base++;
    }

    /* Timeout only for the missing packet */
    for (int i = 0; i < N; i++) {
        if (!ack[i]) {
            printf("\nTimeout for packet %d\n", i + 1);
            printf("Sender: Retransmitting packet %d only\n",
                   i + 1);

            received[i] = 1;
            ack[i] = 1;

            printf("Receiver: Packet %d received\n", i + 1);
            printf("Receiver: ACK %d\n", i + 1);
        }
    }

    /* Deliver packets that were buffered out of order */
    while (base < N && received[base]) {
        printf("Receiver: Delivering packet %d to application\n",
               base + 1);
        base++;
    }

    printf("\nAll packets acknowledged successfully!\n");
    return 0;
}
