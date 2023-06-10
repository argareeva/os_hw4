#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void display_information(const char *info, const char *answ) {
    // Отображение информации о процессе обмена
    printf("%s%s\n", info, ans);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        return 1;
    }
    // Аргументы командной строки
    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);

    // Создание сокета
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1) {
        perror("Ошибка при создании сокета");
        exit(1);
    }

    // Структура для адреса сервера
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    server_address.sin_addr.s_addr = inet_addr(server_ip);
    printf("Соединение с сервером установлено\n");

    while (1) {
        // Получение запроса от клиента
        char request[256];
        int bytes_received = recv(client_socket, request, sizeof(request), 0);
        if (bytes_received > 0) {
            request[bytes_received] = '\0';
            printf("Получен запрос от клиента\n");
            // Перенаправление запроса к серверу
            sendto(client_socket, request, sizeof(request), 0, (struct sockaddr *)&server_address, sizeof(server_address));
            // Получение ответа от сервера
            char response[256];
            bytes_received = recvfrom(client_socket, response, sizeof(response), 0, (struct sockaddr *)&server_address, sizeof(server_address));
            if (bytes_received > 0) {
                response[bytes_received] = '\0';
                printf("Получен ответ от сервера\n");
                // Перенаправление ответа к клиенту
                sendto(client_socket, response, sizeof(response), 0,  (struct sockaddr *)&server_address, sizeof(server_address));
                // Отображение информации о процессе обмена
                display_information("Запрос от клиента: ", request);
                display_information("Ответ от сервера: ", response);
            }
        }
    }

    // Закрытие клиентского сокета
    close(client_socket);

    return 0;
}
