#define _GNU_SOURCE // for accept4

#include <arpa/inet.h>
#include <grp.h>
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
	const unsigned int uid = p->pw_uid;

	const struct group *g = getgrnam("nobody");
	if (g == NULL) g = getgrnam("nogroup");
	if (g == NULL) return 11;
	const unsigned int gid = g->gr_gid;

	if (p->pw_gid != gid) return 12;

	if (
	   strcmp(p->pw_shell, "/bin/nologin")      != 0
	&& strcmp(p->pw_shell, "/usr/bin/nologin")  != 0
	&& strcmp(p->pw_shell, "/usr/sbin/nologin") != 0
	&& strcmp(p->pw_shell, "/sbin/nologin")     != 0
	) return 13;

	if (setgid(gid) != 0) return 14;
	if (setuid(uid) != 0) return 15;

	if (getuid() != uid || getgid() != gid) return 16;

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

	while(1) {
		const int sockNew = accept4(sock, NULL, NULL, SOCK_NONBLOCK);

		shutdown(sockNew, SHUT_RD);

		char r[113 + lenDomain];
		memcpy(r,
			"HTTP/1.1 301 r\r\n"
			"Tk: N\r\n"
			"Content-Length: 0\r\n"
			"Connection: close\r\n"
			"Referrer-Policy: no-referrer\r\n"
			"Location: https://"
		, 109);

		memcpy(r + 109, domain, lenDomain);
		memcpy(r + 109 + lenDomain, "\r\n\r\n", 4);

		write(sockNew, r, 113 + lenDomain);
		close(sockNew);
	}

	return 0;
}
