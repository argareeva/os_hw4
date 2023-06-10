#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#define MAX_THREADS 10

typedef struct {
    int client_socket;
    struct sockaddr_in server_address;
    socklen_t server_address_length;
} Client;

void *handle_client(void *client_ptr) {
    Client *client = (Client *)client_ptr;
    int client_socket = client->client_socket;
    struct sockaddr_in server_address = client->server_address;
    socklen_t server_address_length = client->server_address_length;

    while (1) {
        char request[] = "Запрос на просмотр картин";
        sendto(client_socket, request, sizeof(request), 0, (struct sockaddr *)&server_address, server_address_length);

        // Получение ответа от сервера
        char response[256];
        int bytes_received = recvfrom(client_socket, response, sizeof(response), 0, NULL, NULL);
        if (bytes_received > 0) {
            response[bytes_received] = '\0';
            printf("Получен ответ от сервера: %s\n", response);

            // Обработка ответа от сервера
            if (strncmp(response, "Количество посетителей превышает лимит", strlen("Количество посетителей превышает лимит")) == 0) {
                // Количество посетителей превышает лимит, ожидание
                printf("Получен ответ от сервера: %s\n", response);
                sleep(1);
                continue;
            } else if (strncmp(response, "Выход из галереи", strlen("Выход из галереи")) == 0) {
                // Выход из галереи
                printf("Получен ответ от сервера: %s\n", response);
                break;
            }
        }
    }
    
    char exit_request[] = "Запрос на выход из галереи";
    sendto(client_socket, exit_request, sizeof(exit_request), 0, (struct sockaddr *)&server_address, server_address_length);

    // Закрытие соединения с сервером
    close(client_socket);
    free(client);

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    // Создание клиентского сокета
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1) {
        perror("Ошибка при создании клиентского сокета");
        return 1;
    }

    // Структура для адреса сервера
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    server_address.sin_addr.s_addr = inet_addr(server_ip);

    // Создание массива для хранения идентификаторов потоков
    pthread_t threads[MAX_THREADS];
    int thread_count = 0;

    while (1) {
        if (thread_count >= MAX_THREADS) {
            printf("Превышен лимит клиентов. Закрытие подключения.\n");
            break;
        }

        // Создание структуры для передачи информации клиентскому обработчику
        Client *client = (Client *)malloc(sizeof(Client));
        client->client_socket = client_socket;
        client->server_address = server_address;
        client->server_address_length = sizeof(server_address);

        // Создание нового потока для обработки клиента
        pthread_create(&threads[thread_count], NULL, handle_client, (void *)client);
        thread_count++;
    }

    // Ожидание завершения всех потоков
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    // Закрытие клиентского сокета
    close(client_socket);

    return 0;
}
