
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9090
#define MAX_FRAME 2048
#define GENERATOR "10011"
#define CRC_LEN 4

// Perform modulo-2 division in place
void divide_crc(char bits[], int len)
{
    int gen_len = strlen(GENERATOR);

    for (int i = 0; i <= len - gen_len; i++)
    {
        if (bits[i] == '1')
        {
            for (int j = 0; j < gen_len; j++)
            {
                bits[i + j] =
                    (bits[i + j] == GENERATOR[j]) ? '0' : '1';
            }
        }
    }
}

int main(void)
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr;
    char frame[MAX_FRAME];
    char check[MAX_FRAME];
    int len = 0;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("CRC TCP Receiver listening on port %d...\n", PORT);

    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0)
    {
        perror("accept");
        close(server_fd);
        return 1;
    }

    // Read one newline-terminated frame
    while (len < MAX_FRAME - 1)
    {
        char ch;
        ssize_t n = recv(client_fd, &ch, 1, 0);

        if (n <= 0)
            break;

        if (ch == '\n')
            break;

        frame[len++] = ch;
    }
    frame[len] = '\0';

    printf("\nReceived frame: %s\n", frame);

    if (len <= CRC_LEN || len >= MAX_FRAME - 1)
    {
        printf("Invalid or incomplete frame.\n");
    }
    else
    {
        // Make a copy because division modifies the bits
        strcpy(check, frame);
        divide_crc(check, len);

        printf("Remainder: %.*s\n", CRC_LEN,
               check + len - CRC_LEN);

        int error = 0;
        for (int i = len - CRC_LEN; i < len; i++)
        {
            if (check[i] != '0')
            {
                error = 1;
                break;
            }
        }

        if (error)
            printf("Result: ERROR DETECTED in received frame.\n");
        else
            printf("Result: Frame is error-free (no error detected).\n");
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
