#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#pragma comment(lib, "ws2_32.lib")

// 상수 정의
#define BUFSIZE 512
#define NICKNAME_SIZE 20
#define SERVER_PORT 9000
#define SERVER_IP "127.0.0.1" // 필요에 따라 변경

// 전역 변수
SOCKET sock;
char nickname[NICKNAME_SIZE];  // 닉네임 저장
struct sockaddr_in serveraddr; // 서버 주소 저장

// 함수 프로토타입 선언
void err_quit(const char *msg);
void err_display(const char *msg);
unsigned __stdcall recv_message(void *arg);
void send_message();
void initialize_winsock();
void create_socket();
void setup_server_address();
void input_nickname();
void register_nickname();
void start_recv_thread();

int main()
{
    SetConsoleOutputCP(CP_UTF8); // 콘솔 UTF-8 인코딩 설정

    initialize_winsock();   // 윈속 초기화
    create_socket();        // 소켓 생성
    setup_server_address(); // 서버 주소 설정
    input_nickname();       // 닉네임 입력
    register_nickname();    // 닉네임 등록 메시지 전송
    start_recv_thread();    // 수신 스레드 시작
    send_message();         // 메시지 전송 시작

    closesocket(sock);
    WSACleanup();
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

void setup_server_address()
{
    ZeroMemory(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERVER_PORT);          // 서버 포트
    serveraddr.sin_addr.s_addr = inet_addr(SERVER_IP); // 서버 IP
}

void input_nickname()
{
    printf("닉네임을 입력하세요: ");
    scanf_s("%s", nickname, (unsigned int)sizeof(nickname));
    getchar(); // 입력 버퍼 비우기
}

void register_nickname()
{
    char initMsg[BUFSIZE + NICKNAME_SIZE];
    sprintf_s(initMsg, sizeof(initMsg), "[%s] 님이 입장하였습니다.", nickname);
    int retval = sendto(sock, initMsg, (int)strlen(initMsg), 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR)
    {
        err_quit("sendto()");
    }
}

void start_recv_thread()
{
    HANDLE hThread;
    hThread = (HANDLE)_beginthreadex(NULL, 0, recv_message, NULL, 0, NULL);
    if (hThread == 0)
    {
        err_quit("_beginthreadex()");
    }
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

unsigned __stdcall recv_message(void *arg)
{
    char buf[BUFSIZE + 1];
    int retval;

    while (1)
    {
        retval = recvfrom(sock, buf, BUFSIZE, 0, NULL, NULL);
        if (retval == SOCKET_ERROR)
        {
            err_display("recvfrom()");
            break;
        }

        buf[retval] = '\0';    // 문자열 처리
        printf("\n%s\n", buf); // 수신된 메시지 출력
        fflush(stdout);
    }

    return 0;
}

void send_message()
{
    char buf[BUFSIZE + 1];
    char formattedMsg[BUFSIZE + NICKNAME_SIZE];
    int retval;

    while (1)
    {
        printf("[메시지 입력]: ");
        if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
        {
            break;
        }

        int len = (int)strlen(buf);
        if (buf[len - 1] == '\n')
        {
            buf[len - 1] = '\0'; // 개행 문자 제거
        }

        // [닉네임] 메시지 형식으로 포맷팅
        sprintf_s(formattedMsg, sizeof(formattedMsg), "[%s] %s", nickname, buf);

        retval = sendto(sock, formattedMsg, (int)strlen(formattedMsg), 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
        if (retval == SOCKET_ERROR)
        {
            err_display("sendto()");
            continue;
        }
    }
}
