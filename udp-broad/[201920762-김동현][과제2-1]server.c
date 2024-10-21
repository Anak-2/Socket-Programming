// 헤더 파일 포함
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h> // UTF-8 인코딩을 위한 헤더
#include <process.h> // 스레드 사용을 위한 헤더
#pragma comment(lib, "ws2_32.lib")

// 상수 정의
#define BUFSIZE 512
#define MAX_CLIENTS 100
#define NICKNAME_SIZE 20
#define SERVER_PORT 9000

// 데이터 타입 정의
typedef struct
{
    SOCKADDR_IN addr;
    char nickname[NICKNAME_SIZE];
} ClientInfo;

// 전역 변수 선언
SOCKET sock;
ClientInfo clients[MAX_CLIENTS]; // 클라이언트 정보 저장
int client_count = 0;            // 접속된 클라이언트 수
int total_messages = 0;          // 총 수신한 메시지 수
time_t server_start_time;        // 서버 시작 시간

CRITICAL_SECTION cs; // 스레드 동기화를 위한 크리티컬 섹션

// 함수 프로토타입 선언
void err_quit(const char *msg);
void err_display(const char *msg);
void initialize_winsock();
void create_socket();
void bind_socket();
void initialize_critical_section();
void start_threads();
void cleanup();
unsigned __stdcall relay_messages(void *arg);
unsigned __stdcall handle_menu(void *arg);
void add_client(SOCKADDR_IN client, const char *nickname);
void broadcast_message(const char *message, int message_len, SOCKADDR_IN *sender);
void print_client_info();
void print_chat_stats();

// 메인 함수
int main(int argc, char *argv[])
{
    SetConsoleOutputCP(CP_UTF8); // 콘솔 UTF-8 인코딩 설정

    initialize_winsock();          // 윈속 초기화
    create_socket();               // UDP 소켓 생성
    bind_socket();                 // 소켓에 IP와 포트 번호 바인딩
    initialize_critical_section(); // 동시성 이슈용 크리티컬 섹션 초기화
    server_start_time = time(NULL);
    start_threads(); // 스레드 시작
    cleanup();       // 종료 처리

    return 0;
}

void initialize_winsock()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        err_quit("WSAStartup()");
    }
}

void create_socket()
{
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET)
    {
        err_quit("socket()");
    }
}

void bind_socket()
{
    SOCKADDR_IN localaddr;
    ZeroMemory(&localaddr, sizeof(localaddr));
    localaddr.sin_family = AF_INET;
    localaddr.sin_port = htons(SERVER_PORT);
    localaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int retval = bind(sock, (SOCKADDR *)&localaddr, sizeof(localaddr));
    if (retval == SOCKET_ERROR)
    {
        err_quit("bind()");
    }
}

// 동시성 이슈를 위한 Critical Section 생성
void initialize_critical_section()
{
    InitializeCriticalSection(&cs);
}

void start_threads()
{
    HANDLE hThreadRelay, hThreadMenu;
    // 클라이언트 메세지 전송용 쓰레드 생성
    hThreadRelay = (HANDLE)_beginthreadex(NULL, 0, relay_messages, NULL, 0, NULL);
    if (hThreadRelay == 0)
    {
        err_quit("_beginthreadex()");
    }

    // 채팅 정보 확인용 쓰레드 생성
    hThreadMenu = (HANDLE)_beginthreadex(NULL, 0, handle_menu, NULL, 0, NULL);
    if (hThreadMenu == 0)
    {
        err_quit("_beginthreadex()");
    }

    WaitForSingleObject(hThreadRelay, INFINITE);
    WaitForSingleObject(hThreadMenu, INFINITE);
}

void cleanup()
{
    DeleteCriticalSection(&cs);
    closesocket(sock);
    WSACleanup();
}

void err_quit(const char *msg)
{
    LPVOID lpMsgBuf;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        0,
        (LPSTR)&lpMsgBuf, 0, NULL);
    MessageBoxA(NULL, (LPCSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(-1);
}

void err_display(const char *msg)
{
    LPVOID lpMsgBuf;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, WSAGetLastError(),
        0,
        (LPSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s\n", msg, (char *)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

void add_client(SOCKADDR_IN client, const char *nickname)
{
    EnterCriticalSection(&cs);

    for (int i = 0; i < client_count; i++)
    {
        if (clients[i].addr.sin_addr.s_addr == client.sin_addr.s_addr &&
            clients[i].addr.sin_port == client.sin_port)
        {
            LeaveCriticalSection(&cs);
            return;
        }
    }

    if (client_count < MAX_CLIENTS)
    {
        clients[client_count].addr = client;
        strncpy_s(clients[client_count].nickname, sizeof(clients[client_count].nickname), nickname, _TRUNCATE);
        client_count++;
        printf("새로운 클라이언트 추가: %s (%s:%d)\n", nickname, inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    }
    else
    {
        printf("최대 클라이언트 수에 도달했습니다.\n");
    }

    LeaveCriticalSection(&cs);
}

void broadcast_message(const char *message, int message_len, SOCKADDR_IN *sender)
{
    EnterCriticalSection(&cs);

    for (int i = 0; i < client_count; i++)
    {
        // 클라이언트 자기 자신에게는 보내지 않음
        if (clients[i].addr.sin_addr.s_addr == sender->sin_addr.s_addr &&
            clients[i].addr.sin_port == sender->sin_port)
        {
            continue;
        }

        int retval = sendto(sock, message, message_len, 0, (SOCKADDR *)&clients[i].addr, sizeof(clients[i].addr));
        if (retval == SOCKET_ERROR)
        {
            err_display("sendto()");
        }
    }

    LeaveCriticalSection(&cs);
}

// 메시지 브로드캐스팅용 함수
unsigned __stdcall relay_messages(void *arg)
{
    char buf[BUFSIZE + 1];
    SOCKADDR_IN peeraddr;
    int addrlen;
    int retval;

    while (1)
    {
        addrlen = sizeof(peeraddr);
        retval = recvfrom(sock, buf, BUFSIZE, 0, (SOCKADDR *)&peeraddr, &addrlen);
        if (retval == SOCKET_ERROR)
        {
            err_display("recvfrom()");
            continue;
        }

        buf[retval] = '\0';

        char nickname[NICKNAME_SIZE] = "Unknown";
        sscanf_s(buf, "[%19[^]]]", nickname, (unsigned int)sizeof(nickname));

        add_client(peeraddr, nickname);

        // 메세지 디버깅용 출력
        // printf("[UDP/%s:%d] %s\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port), buf);

        broadcast_message(buf, retval, &peeraddr);

        total_messages++;
    }

    return 0;
}

// 메뉴 선택용 함수
unsigned __stdcall handle_menu(void *arg)
{
    while (1)
    {
        printf("\n메뉴를 선택하세요:\n");
        printf("1. 클라이언트 정보\n");
        printf("2. 채팅 통계\n");
        printf("3. 채팅 종료 (서버 종료)\n");
        printf("메뉴 선택 (1, 2, 3): ");

        int choice;
        scanf_s("%d", &choice);
        getchar(); // 입력 버퍼 비우기

        switch (choice)
        {
        case 1:
            print_client_info();
            break;
        case 2:
            print_chat_stats();
            break;
        case 3:
            printf("\n채팅 서버를 종료합니다.\n");
            cleanup();
            exit(0);
        default:
            printf("\n1, 2, 3 중 하나를 선택해주세요..\n");
            break;
        }
    }

    return 0;
}

void print_client_info()
{
    EnterCriticalSection(&cs);
    printf("\n접속 클라이언트 수: %d\n", client_count);
    for (int i = 0; i < client_count; i++)
    {
        // 각 클라이언트의 정보 [닉네임] IP 주소: 포트번호
        printf("[%s] %s:%d\n", clients[i].nickname, inet_ntoa(clients[i].addr.sin_addr), ntohs(clients[i].addr.sin_port));
    }
    LeaveCriticalSection(&cs);
}

void print_chat_stats()
{
    time_t current_time = time(NULL);
    double minutes_elapsed = difftime(current_time, server_start_time) / 60.0;
    double messages_per_minute = total_messages / (minutes_elapsed > 0 ? minutes_elapsed : 1);

    printf("분당 평균 메시지 수: %.2f\n", messages_per_minute);
}
