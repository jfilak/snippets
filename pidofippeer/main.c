#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

int main(int argc, char *argv[])
{
	if (argc != 2)
		errx(EXIT_FAILURE, "Usage:\n  %s PORT\n", argv[0]);

	int sfd = socket(AF_INET6, SOCK_STREAM, 0);
	if (sfd < 0)
		err(EXIT_FAILURE, "socket");

	struct sockaddr_in6 host_addr = {
		.sin6_family = AF_INET6,
	};

	if (inet_pton(AF_INET6, "::1", host_addr.sin6_addr.s6_addr) < 0)
		err(EXIT_FAILURE, "inet_pton");

	host_addr.sin6_port = htons(atoi(argv[1]));
	host_addr.sin6_scope_id = 0;

	if (bind(sfd, (struct sockaddr *)&host_addr, sizeof(host_addr)) < 0)
		err(EXIT_FAILURE, "bind");

	if (listen(sfd, 6) < 0)
		err(EXIT_FAILURE, "listen");

	for (;;) {
		struct sockaddr_in6 peer_addr;
		socklen_t peer_addr_size = sizeof(peer_addr);
		int psfd = accept(sfd, (struct sockaddr *)&peer_addr, &peer_addr_size);
		if (psfd < 0)
			err(EXIT_FAILURE, "accept");
		write(psfd, "Hello, world!\n", sizeof("Hello, world!")); 
		close(psfd);
	}

	return EXIT_SUCCESS;
}
