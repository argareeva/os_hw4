#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#define MAX_CLIENTS 10

void display_information(const char *info, const char *answ) {
    // Отображение информации о процессе обмена
    printf("%s%s\n", info, answ);
}

void *handle_client(void *client_socket_ptr) {
    int client_socket = *(int *)client_socket_ptr;
    struct sockaddr_in client_address;
    socklen_t client_address_length = sizeof(client_address);
    char request[256];
    int bytes_received;

    while ((bytes_received = recvfrom(client_socket, request, sizeof(request), 0, (struct sockaddr *)&client_address, &client_address_length)) > 0) {
        request[bytes_received] = '\0';
        printf("Получен запрос от клиента: %s\n", request);
        // Перенаправление запроса к серверу
        sendto(client_socket, request, bytes_received, 0, (struct sockaddr *)&client_address, client_address_length);
        // Получение ответа от сервера
        char response[256];
        bytes_received = recvfrom(client_socket, response, sizeof(response), 0, (struct sockaddr *)&client_address, &client_address_length);
        if (bytes_received > 0) {
            response[bytes_received] = '\0';
            printf("Получен ответ от сервера: %s\n", response);
            // Перенаправление ответа к клиенту
            sendto(client_socket, response, bytes_received, 0, (struct sockaddr *)&client_address, client_address_length);
            // Отображение информации о процессе обмена
            display_information("Запрос от клиента: ", request);
            display_information("Ответ от сервера: ", response);
        }
    }

    // Закрытие клиентского сокета
    close(client_socket);
    free(client_socket_ptr);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        return 1;
    }
    // Аргументы командной строки
    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    // Создание сокета
    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == -1) {
        perror("Ошибка при создании сокета");
        exit(1);
    }

    // Структура для адреса сервера
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    server_address.sin_addr.s_addr = inet_addr(server_ip);

    // Привязка сокета к адресу сервера
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Ошибка при привязке сокета к адресу сервера");
        exit(EXIT_FAILURE);
    }

    printf("Сервер запущен. Ожидание подключений...\n");

    while (1) {
        // Принятие входящего соединения
        struct sockaddr_in client_address;
        socklen_t client_address_length = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_length);
        if (client_socket == -1) {
            perror("Ошибка при принятии подключения");
            continue;
        }
        printf("Новое подключение принято\n");

        // Создание отдельного потока для обработки клиента
        pthread_t client_thread;
        int *client_socket_ptr = malloc(sizeof(int));
        *client_socket_ptr = client_socket;

        if (pthread_create(&client_thread, NULL, handle_client, (void *)client_socket_ptr) != 0) {
            perror("Ошибка при создании потока для обработки клиента");
            close(client_socket);
            free(client_socket_ptr);
        }
    }

    // Закрытие серверного сокета
    close(server_socket);

    return 0;
}
