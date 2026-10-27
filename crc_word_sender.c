
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9090
#define MAX_TEXT 128
#define MAX_BITS (MAX_TEXT * 8 + 5)
#define GENERATOR "10011"
#define CRC_LEN 4

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
    int sockfd;
    struct sockaddr_in server_addr;
    char data[MAX_TEXT];
    char data_bits[MAX_BITS];
    char work[MAX_BITS];
    char frame[MAX_BITS];
    int bit_len = 0;

    printf("Enter data: ");
    if (fgets(data, sizeof(data), stdin) == NULL)
        return 1;

    data[strcspn(data, "\n")] = '\0';

    if (data[0] == '\0')
    {
        printf("Data cannot be empty.\n");
        return 1;
    }

    // Convert each character into 8-bit binary
    for (int i = 0; data[i] != '\0'; i++)
    {
        unsigned char ch = (unsigned char)data[i];

        for (int j = 7; j >= 0; j--)
            data_bits[bit_len++] = ((ch >> j) & 1) ? '1' : '0';
    }
    data_bits[bit_len] = '\0';

    printf("Binary data: %s\n", data_bits);

    // Append four zeros before division
    strcpy(work, data_bits);
    for (int i = 0; i < CRC_LEN; i++)
        work[bit_len + i] = '0';

    work[bit_len + CRC_LEN] = '\0';

    // Calculate CRC remainder
    divide_crc(work, bit_len + CRC_LEN);

    char remainder[CRC_LEN + 1];
    memcpy(remainder, work + bit_len, CRC_LEN);
    remainder[CRC_LEN] = '\0';

    printf("Generator: %s\n", GENERATOR);
    printf("CRC checksum: %s\n", remainder);

    // Append CRC to original data bits
    strcpy(frame, data_bits);
    strcat(frame, remainder);

    printf("Transmitted frame: %s\n", frame);

    // Create TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1",
                  &server_addr.sin_addr) != 1)
    {
        fprintf(stderr, "Invalid server IP address.\n");
        close(sockfd);
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sockfd);
        return 1;
    }

    // Send frame with newline as the message delimiter
    strcat(frame, "\n");
    size_t total = strlen(frame);
    size_t sent = 0;

    while (sent < total)
    {
        ssize_t n = send(sockfd, frame + sent, total - sent, 0);

        if (n <= 0)
        {
            perror("send");
            close(sockfd);
            return 1;
        }
        sent += (size_t)n;
    }

    printf("Frame sent successfully over TCP.\n");

    close(sockfd);
    return 0;
}
