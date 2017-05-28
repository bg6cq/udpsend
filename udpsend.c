/* UDPsend: send udp 
	  by james@ustc.edu.cn 2009.04.02
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <net/if.h>
#include <stdarg.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXLEN 			2048
#define MAX_PACKET_SIZE		65536

void usage(void)
{
	printf("Usage:\n");
	printf("./sendudp -l packet_len -c packet_cout remoteip remoteport\n");
	exit(0);
}

int packet_len = 1472;
unsigned long int pkt_cnt, packet_count = 10000;
int ignore_error;

int main(int argc, char *argv[])
{
	int i = 1;
	char buf[MAX_PACKET_SIZE];
	int got_one = 0;
	do {
		got_one = 1;
		if (argc - i <= 0)
			usage();

		if (strcmp(argv[i], "-i") == 0) {
			ignore_error = 1;
		} else if (strcmp(argv[i], "-l") == 0) {
			i++;
			if (argc - i <= 0)
				usage();
			packet_len = atoi(argv[i]);
		} else if (strcmp(argv[i], "-c") == 0) {
			i++;
			if (argc - i <= 0)
				usage();
			packet_count = atoi(argv[i]);
		} else
			got_one = 0;
		if (got_one)
			i++;
	}
	while (got_one);

	if (argc != i + 2)
		usage();

	fprintf(stderr, "packet_len = %d, packet_count = %lu\n", packet_len, packet_count);

	int n;
	for (n = i; n < argc; n++)
		fprintf(stderr, "%s ", argv[n]);
	printf("\n");

	int sockfd;
	struct addrinfo hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((n = getaddrinfo(argv[i], argv[i + 1], &hints, &res)) != 0) {
		fprintf(stderr, "host name lookup error for %s, %s", argv[i], argv[i + 1]);
	}
	ressave = res;
	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

		if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
			break;	/* success */
	}
	while ((res = res->ai_next) != NULL);

	if (packet_len > MAX_PACKET_SIZE)
		packet_len = MAX_PACKET_SIZE;
	memset(buf, 'a', packet_len);
	time_t start_tm, end_tm;
	time(&start_tm);
	pkt_cnt = packet_count;
	while (1) {
		int r;
		r = send(sockfd, buf, packet_len, 0);
		if ((ignore_error == 0) && (r < 0)) {
			fprintf(stderr, "send error, send %lu, remains %lu packets\n", packet_count-pkt_cnt, packet_count);
			exit(0);
		}

		pkt_cnt--;
		if (pkt_cnt == 0)
			break;
	}
	time(&end_tm);
	fprintf(stderr,"%lu seconds\n",end_tm - start_tm);
	fprintf(stderr,"%.0f PPS, %.0f BPS\n", (float)packet_count/((float)(end_tm-start_tm)),
		8.0*(packet_len+28)* (float)packet_count/((float)(end_tm-start_tm)));
	fprintf(stderr,"done\n");
	exit(0);
}
