#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#pragma comment(lib, "ws2_32.lib")

int main()
{
    SetConsoleOutputCP(CP_UTF8); // 콘솔 UTF-8 인코딩 설정
    WSADATA wsaData;
    SOCKET sockfd;
    struct sockaddr_in serverAddr;

    // Winsock 초기화
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // 소켓 생성
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // 서버에 연결
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9000);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    // 데이터 송신 및 수신
    send(sockfd, "클라이언트에서 보냄: Hello!", 26, 0);
    char buffer[512];
    recv(sockfd, buffer, sizeof(buffer), 0);
    printf("수신한 메시지: %s\n", buffer);

    // 소켓 닫기
    closesocket(sockfd);
    WSACleanup();
    return 0;
}
