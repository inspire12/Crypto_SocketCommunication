#include "chat.h"

TALK	msg;

void * send_msg(void * arg)
{
	int sock = (int)arg;

	while(1) {
		fgets(msg.msg, MSG_SIZE, stdin);
		printf("\033[F\033[J");
		write(sock, (void *)&msg, sizeof(TALK));
		fprintf(stdout, "[%s] %s", msg.name, msg.msg);
		memset(msg.msg, 0x0, MSG_SIZE);
	}
}

void * recv_msg(void * arg)
{
	int sock = (int)arg;
	int str_len;

	while(1) {
		TALK tmp;
		str_len = read(sock, (void *)&tmp, sizeof(TALK));
		if ( -1 == str_len ) {
			return (void *)1;
		}
		fprintf(stdout, "[%s] %s", tmp.name, tmp.msg);
	}
}

int main(int argc, char *argv[])
{
	int					serv_sock;
	struct sockaddr_in	serv_addr;

	pthread_t			send_thread, recv_thread;
	void *				thread_result;

	// SIG HANDLER
	signal(SIGINT, (void *)handler);

	if ( argc != 4 ) {
		fprintf(stderr, "Usage : %s <ip> <port> <name>\n", argv[0]);
		exit(1);
	}
	sprintf(msg.name, "%s", argv[3]);

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if ( -1 == serv_sock ) {
		fprintf(stderr, "[!] ERROR : Socket()\n");
	}
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= inet_addr(argv[1]);
	serv_addr.sin_port			= htons(atoi(argv[2]));

	if ( -1 == connect(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) ) {
		fprintf(stderr, "[!] ERROR : Connect()\n");
	}

	pthread_create(&send_thread, NULL, send_msg, (void *)serv_sock);
	pthread_create(&recv_thread, NULL, recv_msg, (void *)serv_sock);

	pthread_join(send_thread, &thread_result);
	pthread_join(recv_thread, &thread_result);

	close(serv_sock);

	return 0;
}