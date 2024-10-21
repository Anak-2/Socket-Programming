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
    SOCKET listenSocket, clientSocket;
    struct sockaddr_in serverAddr;

    // Winsock 초기화
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // 소켓 생성
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // 소켓을 IP와 포트에 바인딩
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9000);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    bind(listenSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    // 클라이언트 연결 대기
    listen(listenSocket, SOMAXCONN);

    // 클라이언트 연결 수락
    clientSocket = accept(listenSocket, NULL, NULL);

    // 데이터 수신 및 송신
    char buffer[512];
    recv(clientSocket, buffer, sizeof(buffer), 0);
    printf("수신한 메시지: %s\n", buffer);
    send(clientSocket, "서버에서 보냄: Hello!", 30, 0);

    // 소켓 닫기
    closesocket(clientSocket);
    closesocket(listenSocket);
    WSACleanup();
    return 0;
}
