// COechoClnt.c
// TCP echo client program
//
// ���ִ��б� ��Ʈ��ũ����Ʈ����� ������ ���� TCP ���� Ŭ���̾�Ʈ ���α׷��Դϴ�.
// ������ �̿��Ͽ� ������ �����ϰ�, �����͸� �ְ�޴� �����Դϴ�.

#include <winsock2.h> // ������ ���� ���α׷����� ���� ���
#include <stdlib.h>	  // ǥ�� ���̺귯�� �Լ��� ����� ���� ���
#include <stdio.h>	  // ����� �Լ� ����� ���� ���

#define BUFSIZE 1500 // ���� ũ�� ���� (������ �ۼ����� ���� �ִ� ũ��)

// ���� �Լ� ���� �߻� �� ���α׷� ���� �Լ�
void err_quit(char *msg)
{
	// ���� �޽��� ��� �� ���α׷� ����
	printf("Error [%s] ... program terminated \n", msg);
	exit(-1);
}

// ���� �Լ� ���� �߻� �� �޽��� ��� �Լ�
void err_display(char *msg)
{
	// ���� �޽��� ��� (���α׷� ����� ���� ����)
	printf("socket function error [%s]\n", msg);
}

int main(int argc, char *argv[])
{
	int retval;				// �Լ� ��ȯ ���� ������ ����
	SOCKET sock;			// ���� ��ũ����
	SOCKADDR_IN serveraddr; // ���� �ּ� ����ü
	char buf[BUFSIZE + 1];	// �ۼ��� ������ ����
	int len;				// �ۼ��� ������ ����

	// ������ ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) // ���� ���̺귯�� �ʱ�ȭ
		return -1;							   // �ʱ�ȭ ���� �� -1 ��ȯ

	// ���� ����
	sock = socket(AF_INET, SOCK_STREAM, 0); // TCP ���� ����
	if (sock == INVALID_SOCKET)
		err_quit("socket()"); // ���� ���� ���� ó��

	// ���� �ּ� ���� (localhost ���)
	ZeroMemory(&serveraddr, sizeof(serveraddr));		 // �ּ� ����ü �ʱ�ȭ
	serveraddr.sin_family = AF_INET;					 // �ּ� ü�� ���� (IPv4)
	serveraddr.sin_port = htons(9000);					 // ���� ��Ʈ ���� (9000)
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // ���� IP ���� (127.0.0.1)

	// ������ ���� �õ�
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr)); // ���� ����
	if (retval == SOCKET_ERROR)
		err_quit("connect()"); // ���� ���� �� ���� ó��

	// �������� ������ ��� �ݺ�
	while (1)
	{
		// �۽��� ������ �Է¹ޱ�
		ZeroMemory(buf, sizeof(buf)); // ���� �ʱ�ȭ
		printf("\n[���� ������] ");
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL) // ǥ�� �Է����κ��� ������ �ޱ�
			break;

		// �Է¹��� ������ ���� �ִ� '\n' ���ڸ� ����
		len = strlen(buf); // ���ڿ� ���� ���
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0'; // ���� ���� ����
		if (strlen(buf) == 0)	 // �� ���ڿ��� ��� ���� ����
			break;

		// ������ ������ ������
		retval = send(sock, buf, strlen(buf), 0); // �����͸� ������ ����
		if (retval == SOCKET_ERROR)
		{ // ���� ���� �� ó��
			err_display("send()");
			break;
		}
		printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� ���½��ϴ�.\n", retval); // ���۵� ����Ʈ �� ���

		// �����κ��� ������ �ޱ�
		retval = recv(sock, buf, BUFSIZE, 0); // �����κ��� ������ ����
		if (retval == SOCKET_ERROR)
		{ // ���� ���� �� ó��
			err_display("recv()");
			break;
		}
		else if (retval == 0) // �������� ������ ���� ��� ���� ����
			break;

		// ���ŵ� ������ ���
		buf[retval] = '\0';											 // ���� �����͸� ���ڿ��� ó��
		printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� �޾ҽ��ϴ�.\n", retval); // ���ŵ� ����Ʈ �� ���
		printf("[���� ������] %s\n", buf);							 // ���ŵ� ������ ���
	}

	// ���� �ݱ�
	closesocket(sock);

	// ������ ���� �ڿ� ����
	WSACleanup();
	return 0;
}
