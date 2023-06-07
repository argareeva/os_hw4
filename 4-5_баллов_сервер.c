#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define MAX_PICTURES 5
#define MAX_VISITORS 10

typedef struct {
    int picture_id;
    int visitors_count;
} Picture;

void handle_client(int client_socket, struct sockaddr_in client_address) {
    // Логика обработки запросов от клиента
    Picture pictures[MAX_PICTURES];
    for (int i = 0; i < MAX_PICTURES; i++) {
        pictures[i].picture_id = i + 1;
        pictures[i].visitors_count = 0;
    }

    while (1) {
        // Пример чтения данных от клиента
        char buffer[1024];
        socklen_t client_address_length = sizeof(client_address);
        int bytes_received = recvfrom(client_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_address, &client_address_length);
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
                    sendto(client_socket, response, sizeof(response), 0, (struct sockaddr*)&client_address, sizeof(client_address));
                    printf("Количество посетителей превышает лимит\n");
                    continue;
                }
                // Увеличение счетчика посетителей
                pictures[random_picture].visitors_count++;
                // Отправка информации о количестве посетителей
                char response[256];
                sprintf(response, "Картинка %d: %d посетителей", pictures[random_picture].picture_id, pictures[random_picture].visitors_count);
                printf("Отправка данных клиенту: Картинка %d: %d посетителей\n", pictures[random_picture].picture_id, pictures[random_picture].visitors_count);
                sendto(client_socket, response, sizeof(response), 0, (struct sockaddr*)&client_address, sizeof(client_address));
            } else if (strcmp(buffer, "Запрос на выход из галереи") == 0) {
                // Запрос на выход из галереи
                char response[] = "Выход из галереи";
                sendto(client_socket, response, sizeof(response), 0, (struct sockaddr*)&client_address, sizeof(client_address));
                printf("Выход из галереи\n");
                break;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    // Создание сокета
    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        perror("Ошибка при создании сокета");
        return 1;
    }

    // Привязка сокета к IP-адресу и порту
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(server_ip);
    server_address.sin_port = htons(server_port);
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Ошибка при привязке сокета");
        return 1;
    }

    printf("Сервер запущен и ожидает подключений...\n");

    while (1) {
        struct sockaddr_in client_address;
        socklen_t client_address_length = sizeof(client_address);

        // Принятие запроса от клиента
        char buffer[1024];
        int bytes_received = recvfrom(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_address, &client_address_length);
        if (bytes_received > 0) {
          printf("Подключение клиента\n");
            // Создание нового процесса для обслуживания клиента
            pid_t pid = fork();
            if (pid < 0) {
                perror("Ошибка при создании процесса");
                return 1;
            } else if (pid == 0) {
                // Дочерний процесс - обработка запроса клиента
                close(server_socket);
                handle_client(server_socket, client_address);
                printf("Отключение клиента\n");
                exit(0);
            } else {
                // Родительский процесс - продолжение ожидания подключений
                close(client_socket);
            }
        }
    }

    // Закрытие сокета
    close(server_socket);

    return 0;
}
