#define _GNU_SOURCE // for accept4

#include <arpa/inet.h>
#include <pwd.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT_HTTP 80

static int initSocket(const int * const sock) {
	struct sockaddr_in servAddr;
	bzero((char*)&servAddr, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(PORT_HTTP);

	const int ret = bind(*sock, (struct sockaddr*)&servAddr, sizeof(servAddr));
	if (ret < 0) return ret;

	listen(*sock, 3); // socket, backlog (# of connections to keep in queue)
	return 0;
}

static int dropRoot(void) {
	const struct passwd * const p = getpwnam("nobody");
	if (p == NULL) return 10;

	if (setgid(p->pw_gid) != 0) return 11;
	if (setuid(p->pw_uid) != 0) return 12;

	return 0;
}

int main(int argc, char *argv[]) {
	if (argc != 2) return 1;

	const char * const domain = argv[1];
	const size_t lenDomain = strlen(domain);

	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) return 2; // Prevent writing to closed/invalid sockets from ending the process

	const int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (initSocket(&sock) != 0) return 3;

	const int ret = dropRoot();
	if (ret != 0) return ret;

	const size_t lenResponse = 113 + lenDomain;
	char response[lenResponse];

	memcpy(response,
		"HTTP/1.1 301 r\r\n"
		"Tk: N\r\n"
		"Content-Length: 0\r\n"
		"Connection: close\r\n"
		"Referrer-Policy: no-referrer\r\n"
		"Location: https://"
	, 109);

	memcpy(response + 109, domain, lenDomain);
	memcpy(response + 109 + lenDomain, "\r\n\r\n", 4);

	while(1) {
		const int sockNew = accept4(sock, NULL, NULL, SOCK_NONBLOCK);

		shutdown(sockNew, SHUT_RD);
		write(sockNew, response, lenResponse);
		close(sockNew);
	}

	return 0;
}
