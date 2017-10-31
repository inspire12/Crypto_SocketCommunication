#include "chat.h"

#define CLNT_MAX_NUM 5

int clnt_num = 0;
int clnt_socks[CLNT_MAX_NUM];

void * clnt_manage(void * arg)
{
	int		clnt_sock	= (int)arg;
	int		str_len		= 1;
	int		i, j;

	TALK	msg;

	while ( 0 != (str_len = read(clnt_sock, (void *)&msg, sizeof(TALK))) ) {
		pthread_mutex_lock(&mutex);
		for ( i = 0 ; i < clnt_num ; i++ ) {
			if ( clnt_sock != clnt_socks[i] ) {
				write(clnt_socks[i], (void *)&msg, str_len);
			}
		}
		fprintf(stdout, "[%s] %s", msg.name, msg.msg);
		memset(msg.msg, 0x0, sizeof(msg.msg));
		pthread_mutex_unlock(&mutex);
	}

	// client exit
	sprintf(msg.msg, "--- Exit ---\n");
	str_len = strlen(msg.msg);
	fprintf(stdout, "[%s] %s", msg.name, msg.msg);

	pthread_mutex_lock(&mutex);
	for ( i = 0 ; i < clnt_num ; i++ ) {
		if ( clnt_sock == clnt_socks[i] ) {
			for ( j = i ; j < clnt_num - 1 ; j++ ) {
				clnt_socks[j] = clnt_socks[j+1];
			}
		}
		else {
			write(clnt_socks[i], (void *)&msg, NAME_SIZE + str_len);
		}
	}
	pthread_mutex_unlock(&mutex);
}

int main(int argc, char *argv[])
{
	int					serv_sock;
	struct sockaddr_in	serv_addr;

	int					clnt_sock;
	struct sockaddr_in	clnt_addr;
	socklen_t			clnt_addr_size;

	pthread_t			thread;

	// SIG HANDLER
	signal(SIGINT, (void *)handler);

	if ( argc != 2 ) {
		fprintf(stderr, "Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	if ( pthread_mutex_init(&mutex, NULL) ) {
		fprintf(stderr, "[!] ERROR : Mutext Init\n");
	}

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if ( -1 == serv_sock ) {
		fprintf(stderr, "[!] ERROR : Socket()\n");
	}
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= htonl(INADDR_ANY);
	serv_addr.sin_port			= htons(atoi(argv[1]));

	if ( -1 == bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) ) {
		fprintf(stderr, "[!] ERROR : Bind()\n");
	}
	if ( -1 == listen(serv_sock, CLNT_MAX_NUM) ) {
		fprintf(stderr, "[!] ERROR : Listen()\n");
	}

	while(1) {
		clnt_addr_size = sizeof(clnt_addr);
		clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
		if ( -1 == clnt_sock ) {
			fprintf(stderr, "[!] ERROR : Accept()\n");
		}

		pthread_mutex_lock(&mutex);
		clnt_socks[clnt_num++] = clnt_sock;
		pthread_create(&thread, NULL, clnt_manage, (void *)clnt_sock);
		pthread_mutex_unlock(&mutex);
		fprintf(stdout, "[!] New User : %s\n", (char *)inet_ntoa(clnt_addr.sin_addr));
	}

	return 0;
}
