#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")
#pragma warning(disable:4996)

#define PORT 9999
// #define SERVER_ADDR "127.0.0.1"
#define SERVER_ADDR "192.168.0.10"
#define BUFSIZE 512

void InitOpenSSL() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void CleanupOpenSSL() {
    EVP_cleanup();
}

SSL_CTX* CreateClientContext() {
    const SSL_METHOD* method;
    SSL_CTX* ctx;

    method = SSLv23_client_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

int main() {
    WSADATA wsa;
    SOCKET client_fd;
    struct sockaddr_in server_addr;
    SSL_CTX* ctx;
    SSL* ssl;
    char buf[BUFSIZE + 1];

    // OpenSSL �ʱ�ȭ
    InitOpenSSL();
    ctx = CreateClientContext();

    // Winsock �ʱ�ȭ
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // ���� ����
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == INVALID_SOCKET) {
        printf("Could not create socket. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    // ������ ����
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connect failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client_fd);

    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
    }
    else {
        // ������ ������ ���
        while (1) {
            // ����ڷκ��� �Է� �ޱ�
            printf("\n������ ���� �޽����� �Է��ϼ��� ('exit' �Է� �� ����): ");
            fgets(buf, sizeof(buf), stdin);
            buf[strcspn(buf, "\n")] = 0;  // �Է¿��� ���๮�ڸ� ����
            if (strcmp(buf, "exit") == 0) {
                break;
            }
            // ������ �޽��� ����
            SSL_write(ssl, buf, strlen(buf));

            // �����κ��� ���� �޽��� ����
            int bytes = SSL_read(ssl, buf, sizeof(buf));
            if (bytes > 0) {
                printf("�����κ��� ���� ���� �޽���: %s\n", buf);
            }
        }
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    closesocket(client_fd);

    SSL_CTX_free(ctx);
    CleanupOpenSSL();
    WSACleanup();

    return 0;
}
