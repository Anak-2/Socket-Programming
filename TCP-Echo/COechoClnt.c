// COechoClnt.c
// TCP echo client program
//
// 아주대학교 네트워크소프트웨어설계 과목을 위한 TCP 에코 클라이언트 프로그램입니다.
// 소켓을 이용하여 서버와 연결하고, 데이터를 주고받는 예제입니다.

#include <winsock2.h> // 윈도우 소켓 프로그래밍을 위한 헤더
#include <stdlib.h>	  // 표준 라이브러리 함수들 사용을 위한 헤더
#include <stdio.h>	  // 입출력 함수 사용을 위한 헤더

#define BUFSIZE 1500 // 버퍼 크기 설정 (데이터 송수신을 위한 최대 크기)

// 소켓 함수 오류 발생 시 프로그램 종료 함수
void err_quit(char *msg)
{
	// 오류 메시지 출력 후 프로그램 종료
	printf("Error [%s] ... program terminated \n", msg);
	exit(-1);
}

// 소켓 함수 오류 발생 시 메시지 출력 함수
void err_display(char *msg)
{
	// 오류 메시지 출력 (프로그램 종료는 하지 않음)
	printf("socket function error [%s]\n", msg);
}

int main(int argc, char *argv[])
{
	int retval;				// 함수 반환 값을 저장할 변수
	SOCKET sock;			// 소켓 디스크립터
	SOCKADDR_IN serveraddr; // 서버 주소 구조체
	char buf[BUFSIZE + 1];	// 송수신 데이터 버퍼
	int len;				// 송수신 데이터 길이

	// 윈도우 소켓 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) // 소켓 라이브러리 초기화
		return -1;							   // 초기화 실패 시 -1 반환

	// 소켓 생성
	sock = socket(AF_INET, SOCK_STREAM, 0); // TCP 소켓 생성
	if (sock == INVALID_SOCKET)
		err_quit("socket()"); // 소켓 생성 오류 처리

	// 서버 주소 설정 (localhost 사용)
	ZeroMemory(&serveraddr, sizeof(serveraddr));		 // 주소 구조체 초기화
	serveraddr.sin_family = AF_INET;					 // 주소 체계 설정 (IPv4)
	serveraddr.sin_port = htons(9000);					 // 서버 포트 설정 (9000)
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 서버 IP 설정 (127.0.0.1)

	// 서버에 연결 시도
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr)); // 서버 연결
	if (retval == SOCKET_ERROR)
		err_quit("connect()"); // 연결 실패 시 오류 처리

	// 서버와의 데이터 통신 반복
	while (1)
	{
		// 송신할 데이터 입력받기
		ZeroMemory(buf, sizeof(buf)); // 버퍼 초기화
		printf("\n[보낼 데이터] ");
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL) // 표준 입력으로부터 데이터 받기
			break;

		// 입력받은 데이터 끝에 있는 '\n' 문자를 제거
		len = strlen(buf); // 문자열 길이 계산
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0'; // 개행 문자 제거
		if (strlen(buf) == 0)	 // 빈 문자열일 경우 루프 종료
			break;

		// 서버로 데이터 보내기
		retval = send(sock, buf, strlen(buf), 0); // 데이터를 서버로 전송
		if (retval == SOCKET_ERROR)
		{ // 전송 실패 시 처리
			err_display("send()");
			break;
		}
		printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval); // 전송된 바이트 수 출력

		// 서버로부터 데이터 받기
		retval = recv(sock, buf, BUFSIZE, 0); // 서버로부터 데이터 수신
		if (retval == SOCKET_ERROR)
		{ // 수신 실패 시 처리
			err_display("recv()");
			break;
		}
		else if (retval == 0) // 서버와의 연결이 끊긴 경우 루프 종료
			break;

		// 수신된 데이터 출력
		buf[retval] = '\0';											 // 받은 데이터를 문자열로 처리
		printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", retval); // 수신된 바이트 수 출력
		printf("[받은 데이터] %s\n", buf);							 // 수신된 데이터 출력
	}

	// 소켓 닫기
	closesocket(sock);

	// 윈도우 소켓 자원 해제
	WSACleanup();
	return 0;
}
