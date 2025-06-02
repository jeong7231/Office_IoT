#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <mysql/mysql.h>

#define ID_SIZE 20
#define BUF_SIZE 100
#define NAME_SIZE 20
#define ARR_CNT 5

void *send_msg(void *arg);
void *recv_msg(void *arg);
void error_handling(char *msg);

char name[NAME_SIZE] = "[Default]";
char msg[BUF_SIZE];

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void *thread_return;

	if (argc != 4)
	{
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	}

	sprintf(name, "%s", argv[3]);

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == -1)
		error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");

	sprintf(msg, "[%s:PASSWD]", name);
	write(sock, msg, strlen(msg));

	pthread_create(&rcv_thread, NULL, recv_msg, (void *)&sock);
	pthread_create(&snd_thread, NULL, send_msg, (void *)&sock);

	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);

	close(sock);
	return 0;
}

void *send_msg(void *arg)
{
	int *sock = (int *)arg;
	int ret;
	fd_set initset, newset;
	struct timeval tv;
	char name_msg[NAME_SIZE + BUF_SIZE + 2];

	FD_ZERO(&initset);
	FD_SET(STDIN_FILENO, &initset);

	fputs("Input a message! [ID]msg (Default ID:ALLMSG)\n", stdout);
	while (1)
	{
		memset(msg, 0, sizeof(msg));
		name_msg[0] = '\0';
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		newset = initset;
		ret = select(STDIN_FILENO + 1, &newset, NULL, NULL, &tv);
		if (FD_ISSET(STDIN_FILENO, &newset))
		{
			fgets(msg, BUF_SIZE, stdin);
			if (!strncmp(msg, "quit\n", 5))
			{
				*sock = -1;
				return NULL;
			}
			else if (msg[0] != '[')
			{
				strcat(name_msg, "[ALLMSG]");
				strcat(name_msg, msg);
			}
			else
				strcpy(name_msg, msg);
			if (write(*sock, name_msg, strlen(name_msg)) <= 0)
			{
				*sock = -1;
				return NULL;
			}
		}
		if (ret == 0)
		{
			if (*sock == -1)
				return NULL;
		}
	}
}

void *recv_msg(void *arg)
{
	MYSQL *conn;
	MYSQL_ROW sqlrow;
	int res;
	char sql_cmd[200] = {0};
	char *host = "localhost";
	char *user = "iot";
	char *pass = "pwiot";
	char *dbname = "iotdb";

	int *sock = (int *)arg;
	int i;
	char *pToken;
	char *pArray[ARR_CNT] = {0};

	char name_msg[NAME_SIZE + BUF_SIZE + 1];
	int str_len;

	int illu;
	double temp, humi;
	static char recentSender[ID_SIZE] = "PRJ_AND";

	puts("MYSQL startup");
	conn = mysql_init(NULL);
	if (!mysql_real_connect(conn, host, user, pass, dbname, 0, NULL, 0))
	{
		fprintf(stderr, "ERROR : %s[%d]\n", mysql_error(conn), mysql_errno(conn));
		exit(1);
	}
	else
	{
		printf("Connection Successful!\n\n");
	}

	while (1)
	{
		memset(name_msg, 0x0, sizeof(name_msg));
		str_len = read(*sock, name_msg, NAME_SIZE + BUF_SIZE);
		if (str_len <= 0)
		{
			*sock = -1;
			return NULL;
		}
		name_msg[str_len] = 0;
		fputs(name_msg, stdout);

		pToken = strtok(name_msg, "[:@]");
		i = 0;
		while (pToken != NULL && i < ARR_CNT)
		{
			pArray[i++] = pToken;
			pToken = strtok(NULL, "[:@]");
		}

		if (!strcmp(pArray[1], "CERT") && i == 4) // [PRJ_SQL]CERT@101@8315D829
		{
			printf("DEBUG CERT: i=%d, p0=%s, p1=%s, p2=%s, p3=%s\n",
				   i, pArray[0], pArray[1], pArray[2], pArray[3]);

			int room_number = atoi(pArray[2]);
			char *recv_card_hex = pArray[3];
			recv_card_hex[strcspn(recv_card_hex, "\r\n")] = '\0'; // recv 값 개행 제거

			char db_card_hex[BUF_SIZE] = {0};
			int match = 0;

			snprintf(sql_cmd, sizeof(sql_cmd),
					 "SELECT card_hex FROM user_info WHERE room_number = %d",
					 room_number);

			printf("SQL CMD: %s\n", sql_cmd);

			if (mysql_query(conn, sql_cmd))
			{
				fprintf(stderr, "ERROR: %s[%d]\n", mysql_error(conn), mysql_errno(conn));
			}
			else
			{
				MYSQL_RES *result = mysql_store_result(conn);
				MYSQL_ROW row = mysql_fetch_row(result);
				if (row && row[0])
				{
					strncpy(db_card_hex, row[0], sizeof(db_card_hex) - 1);
					db_card_hex[sizeof(db_card_hex) - 1] = '\0';
					db_card_hex[strcspn(db_card_hex, "\r\n")] = '\0'; // db 값 개행 제거

					if (!strcmp(db_card_hex, recv_card_hex))
						match = 1;
				}
				if (result)
					mysql_free_result(result);

				snprintf(msg, sizeof(msg), match ? "[PRJ_STM]CERT@OK\n" : "[PRJ_STM]CERT@NO\n");
				printf("DEBUG SEND MSG: %s\n", msg); // 전송 직전 확인용 로그
				write(*sock, msg, strlen(msg));

				continue;
			}
		}

		else if (!strcmp(pArray[1], "SETROOM") && (i == 5 || i == 4)) // [PRJ_SQL]SETROOM@101@IN
		{
			int room_number = atoi(pArray[2]);
			char *status = pArray[3];
			status[strcspn(status, "\n")] = '\0';

			char *door_state = (!strcmp(status, "IN")) ? "UNLOCK" : "LOCK";
			char *light_state = (!strcmp(status, "IN")) ? "ON" : "OFF";

			snprintf(sql_cmd, sizeof(sql_cmd),
					 "UPDATE room_status SET status='%s', door='%s', light='%s' WHERE room=%d",
					 status, door_state, light_state, room_number);

			if (mysql_query(conn, sql_cmd))
			{
				fprintf(stderr, "ERROR: %s[%d]\n", mysql_error(conn), mysql_errno(conn));
			}
			if (i == 5)
			{

				// 개행 문자 제거
				pArray[4][strcspn(pArray[4], "\r\n")] = '\0';
				snprintf(msg, sizeof(msg), "[%s]%s@%s\r\n", pArray[4], pArray[2], pArray[3]);

				write(*sock, msg, strlen(msg));
			}
			continue;
		}

		else if (!strcmp(pArray[1], "GETROOM") && i == 3) // [PRJ_SQL]GETROOM@101 or @ALL
		{
			char *target = pArray[2];
			target[strcspn(target, "\n")] = '\0';

			MYSQL_RES *result;
			MYSQL_ROW row;

			if (!strcmp(target, "ALL"))
			{
				snprintf(sql_cmd, sizeof(sql_cmd),
						 "SELECT room, status, door, light FROM room_status");
			}
			else
			{
				int room_number = atoi(target);
				snprintf(sql_cmd, sizeof(sql_cmd),
						 "SELECT room, status, door, light FROM room_status WHERE room = %d",
						 room_number);
			}

			if (mysql_query(conn, sql_cmd))
			{
				fprintf(stderr, "ERROR: %s[%d]\n", mysql_error(conn), mysql_errno(conn));
			}
			else
			{
				result = mysql_store_result(conn);
				if (result)
				{
					while ((row = mysql_fetch_row(result)) != NULL)
					{
						snprintf(msg, sizeof(msg), "[%s]GETROOM@%s@%s@%s@%s\n",
								 pArray[0], row[0], row[1], row[2], row[3]);
						write(*sock, msg, strlen(msg));
					}
					mysql_free_result(result);
				}
			}
			continue;
		}

		else if (i == 2)
		{
			// 개행 제거
			pArray[1][strcspn(pArray[1], "\r\n")] = '\0';

			if (!strcmp(pArray[1], "UNLOCK"))
			{
				snprintf(sql_cmd, sizeof(sql_cmd),
						 "UPDATE room_status SET door='UNLOCK'");

				if (mysql_query(conn, sql_cmd))
				{
					fprintf(stderr, "ERROR: %s[%d]\n", mysql_error(conn), mysql_errno(conn));
				}

				continue;
			}
		}
	}
}
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

