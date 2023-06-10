#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#define MAX_PICTURES 5
#define MAX_VISITORS 10

typedef struct {
    int picture_id;
    int visitors_count;
} Picture;

typedef struct {
    int client_socket;
    struct sockaddr_in client_address;
    socklen_t client_address_length;
    Picture *pictures;
} Client;

void *handle_client(void *client_ptr) {
    Client *client = (Client *)client_ptr;
    int client_socket = client->client_socket;
    struct sockaddr_in client_address = client->client_address;
    socklen_t client_address_length = client->client_address_length;
    Picture *pictures = client->pictures;

    while (1) {
        // Чтение данных от клиента
        char buffer[1024];
        int bytes_received = recvfrom(client_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_address, &client_address_length);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("Получены данные от клиента: %s\n", buffer);
            // Обработка запроса от клиента
            if (strcmp(buffer, "Запрос на просмотр картин") == 0) {
                // Случайный выбор картинки
                int random_picture = rand() % MAX_PICTURES;
                // Проверка количества посетителей у картинки
                if (pictures[random_picture].visitors_count >= MAX_VISITORS) {
                    // Количество посетителей превышает лимит
                    char response[] = "Количество посетителей превышает лимит";
                    sendto(client_socket, response, sizeof(response), 0, (struct sockaddr *)&client_address, client_address_length);
                    printf("Количество посетителей превышает лимит");
                    continue;
                }
                // Увеличение счетчика посетителей
                pictures[random_picture].visitors_count++;
                // Отправка информации о количестве посетителей
                char response[256];
                sprintf(response, "Картинка %d: %d посетителей", pictures[random_picture].picture_id, pictures[random_picture].visitors_count);
                printf("Отправка данных клиенту: Картинка %d: %d посетителей", pictures[random_picture].picture_id, pictures[random_picture].visitors_count);
                sendto(client_socket, response, sizeof(response), 0, (struct sockaddr *)&client_address, client_address_length);
            } else if (strcmp(buffer, "Запрос на выход из галереи") == 0) {
                // Запрос на выход из галереи
                char response[] = "Выход из галереи";
                sendto(client_socket, response, sizeof(response), 0, (struct sockaddr *)&client_address, client_address_length);
                printf("Выход из галереи");
                break;
            }
        }
    }

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

    // Создание серверного сокета
    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == -1) {
        perror("Ошибка при создании серверного сокета");
        return 1;
    }

    // Настройка адреса сервера
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    server_address.sin_addr.s_addr = inet_addr(server_ip);

    // Привязка серверного сокета к адресу
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Ошибка при привязке серверного сокета");
        return 1;
    }

    printf("Сервер запущен. Ожидание подключений...\n");

    // Создание массива с информацией о картинках
    Picture pictures[MAX_PICTURES];
    for (int i = 0; i < MAX_PICTURES; i++) {
        pictures[i].picture_id = i + 1;
        pictures[i].visitors_count = 0;
    }
  while (1) {
        // Принятие клиентского запроса
        struct sockaddr_in client_address;
        socklen_t client_address_length = sizeof(client_address);
        char buffer[1024];
        int bytes_received = recvfrom(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_address, &client_address_length);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("Получен запрос от клиента: %s\n", buffer);

            if (strcmp(buffer, "Подключение клиента") == 0) {
                // Создание структуры для передачи информации клиентскому обработчику
                Client *client = (Client *)malloc(sizeof(Client));
                client->client_socket = server_socket;
                client->client_address = client_address;
                client->client_address_length = client_address_length;
                client->pictures = pictures;

                // Создание нового потока для обработки клиента
                pthread_t thread;
                pthread_create(&thread, NULL, handle_client, (void *)client);
                pthread_detach(thread);
            }
        }
    }

    // Закрытие серверного сокета
    close(server_socket);

    return 0;
}
