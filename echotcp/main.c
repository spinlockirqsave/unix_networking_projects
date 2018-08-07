
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MSG_LEN 12

int main(void)
{
	int fd = -1;
	struct sockaddr_in server = {0};
	const char *message = "test message";
	char msg[MSG_LEN + 1] = {0};			/* length of message + terminating '\0' */

	server.sin_family = AF_INET;
	server.sin_port = htons(5060);
	inet_pton(AF_INET, "34.215.174.112", &server.sin_addr);

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		perror("Socket creation failed");

	if (connect(fd, (struct sockaddr*) &server, sizeof(server)) < 0) {
		perror("Connect failed");
		exit(EXIT_FAILURE);
	}

	memcpy(msg, message, MSG_LEN);
	msg[MSG_LEN + 1] = '\0';			/* just in case */

	int bytes_n = 0;
	int written_now = 0;
	while (((written_now = write(fd, msg, MSG_LEN)) != -1)
	       && ((bytes_n += written_now) < MSG_LEN))
	{
		fprintf(stderr, "written %d, total %d\n", written_now, bytes_n);
	}

	if (written_now == -1) {
		perror("Error on write");
		exit(EXIT_FAILURE);
	} else {
		fprintf(stdout, "Written: %s\n", msg);
	}

	memset(msg, 0, MSG_LEN);

	bytes_n = 0;
	int read_now = 0;
	while (((read_now = read(fd, msg, MSG_LEN)) != -1)
	       && ((bytes_n += read_now) < MSG_LEN));

	if (read_now == -1) {
		perror("Error on read");
		exit(EXIT_FAILURE);
	} else {
		fprintf(stdout, "Read: %s\n", msg);
	}

	return EXIT_SUCCESS;
}
