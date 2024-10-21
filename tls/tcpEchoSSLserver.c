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

void InitOpenSSL() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void CleanupOpenSSL() {
    EVP_cleanup();
}

SSL_CTX* CreateServerContext() {
    const SSL_METHOD* method;
    SSL_CTX* ctx;

    method = SSLv23_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void ConfigureServerContext(SSL_CTX* ctx) {
    // 서버 인증서 및 개인 키 설정
    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

int main() {
    WSADATA wsa;
    SOCKET server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    int client_len = sizeof(client_addr);
    SSL_CTX* ctx;
    SSL* ssl;

    // 초기화
    InitOpenSSL();
    ctx = CreateServerContext();
    ConfigureServerContext(ctx);

    // Winsock 초기화
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // 소켓 생성
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        printf("Could not create socket. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // 바인드
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // 대기
    listen(server_fd, 1);

    printf("SSL Echo 서버가 실행 중입니다...\n");

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == INVALID_SOCKET) {
            printf("accept failed. Error Code: %d\n", WSAGetLastError());
            return 1;
        }

        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client_fd);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
        }
        else {
            while (1) {
                char buffer[1024] = { 0 };
                int bytes = SSL_read(ssl, buffer, sizeof(buffer));
                if (bytes > 0) {
                    printf("받은 메시지: %s\n", buffer);
                    SSL_write(ssl, buffer, bytes);
                }
                else
                    break;
            }
        }

        SSL_shutdown(ssl);
        SSL_free(ssl);
        closesocket(client_fd);
    }

    closesocket(server_fd);
    SSL_CTX_free(ctx);
    CleanupOpenSSL();
    WSACleanup();

    return 0;
}
