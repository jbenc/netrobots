#include "robots.h"
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>

#include "net_utils.h"
#include "net_command_list.h"

#undef main

static int serverfd;

void start(void);
void set_name(char *name);

static int eval_response(int resp)
{
	switch (resp) {
	case DEAD:
		printf_die(stdout, "You are dead!\n", 1);
		break;
	case OK:
		break;
	case ERROR:
	default:
		printf_die(stderr, "Error detected.. exiting!\n", 2);
		break;
	}
	return resp;
}

static int get_resp_value(int ret)
{
	char resp[STD_RESP_LEN + 1], **argv;
	int count, result = -1, argc;

	count = read(serverfd, resp, STD_RESP_LEN);

	if (count > 0) {
		resp[count] = '\0';
		argc = str_to_argv(resp, &argv);
		if (argc == -1)
			printf_die(stderr, "[ERROR] Cannot allocate buffer... exiting!\n",
				   EXIT_FAILURE);
		if (argc >= 1 && str_isnumber(argv[0])) {
			eval_response(atoi(argv[0]));
			if (argc >= 2 && str_isnumber(argv[1]))
				result = atoi(argv[1]);
			else
				result = 0;
		}
		free(argv);
	}
	if (ret == -1)
		printf_die(stdout, "Server probably dead or you have been killed!\n");
	return result;
}

static int client_init(char *remotehost, char *port)
{
	int ret, sock = -1;
	struct addrinfo *ai, *runp;
	struct addrinfo hints;

	memset(&hints, '\0', sizeof(hints));
	hints.ai_flags = AI_ADDRCONFIG;
	hints.ai_socktype = SOCK_STREAM;

	if (ret = getaddrinfo(remotehost, port, &hints, &ai))
		printf_die(stderr, "[ERROR] getaddrinfo('%s', '%s'): %s\n",
			   EXIT_FAILURE, remotehost, port, gai_strerror(ret));
	if (!ai)
		printf_die(stderr, "[ERROR] getaddrinf(): couldn't fill the struct!\n",
			   EXIT_FAILURE);

	runp = ai;
	while (runp) {
		sock = socket(runp->ai_family, runp->ai_socktype, runp->ai_protocol);
		if (sock != -1) {
			ndprintf(stdout, "[NETWORK] Connecting to server...\n");
			if (connect(sock, runp->ai_addr, runp->ai_addrlen) == 0) {
				ndprintf(stdout, "[NETWORK] connected to server\n");
				serverfd = sock;
				break;
			}
			close(sock);
			sock = -1;
		}
		runp = runp->ai_next;
	}
	freeaddrinfo(ai);
	return (sock < 0);
}

static void set_default_name(char *argv0)
{
	char *start;

	start = strrchr(argv0, '/');
	if (!start)
		start = argv0;
	else
		start++;
	set_name(start);
}

void usage(char *prog, int retval)
{
	printf("Usage %s [-n <clients> -H <hostname> -P <port> -d]\n"
	       "\t-H <remothost>\tSpecifies hostname (Default: 127.0.0.1)\n"
	       "\t-P <port>\tSpecifies port (Default: 4300)\n"
	       "\t-d\tEnables debug mode\n", prog);
	exit(retval);
}

int main(int argc, char **argv)
{
	int retval;
	char *remotehost = DEFAULT_REMOTEHOST;
	char *port = DEFAULT_PORT;

	while ((retval = getopt(argc, argv, "dn:hH:P:")) != -1) {
		switch (retval) {
		case 'H':
			remotehost = optarg;
			break;
		case 'P':
			port = optarg;
			break;
		case 'd':
			debug = 1;
			break;
		case 'h':
			usage(argv[0], EXIT_SUCCESS);
			break;
		default:
			break;
		}
	}

	if (argc > optind)
		usage(argv[0], EXIT_FAILURE);

	signal(SIGPIPE, SIG_IGN);
	if (client_init(remotehost, port))
		printf_die(stderr, "Could not connect to %s:%s\n",
			   EXIT_FAILURE, remotehost, port);

	srandom(time(NULL) + getpid());
	srand(time(NULL) + getpid());

#ifndef MANUAL_INIT
	set_default_name(argv[0]);
	start();
#endif
	rmain ();
}

void start(void)
{
	int ret;

	ret = sockwrite(serverfd, START, NULL);
	get_resp_value(ret);
}

int scan(int degree, int resolution)
{
	int ret;

	ret = sockwrite(serverfd, SCAN, "%d %d", degree, resolution);
	return get_resp_value(ret);
}

int cannon(int degree, int range)
{
	int ret;

	ret = sockwrite(serverfd, CANNON, "%d %d", degree, range);
	return get_resp_value(ret);
}

void drive (int degree, int speed)
{
	int ret;

	ret = sockwrite(serverfd, DRIVE,  "%d %d", degree, speed);
	get_resp_value(ret);
}

int damage()
{
	int ret;

	ret = sockwrite(serverfd, DAMAGE, NULL);
	return get_resp_value(ret);
}

int cycle2()
{
	int ret;

	ret = sockwrite(serverfd, CYCLE, NULL);
	return get_resp_value(ret);
}

void cycle ()
{
	(void)cycle2();
}

int speed()
{
	int ret;

	ret = sockwrite(serverfd, SPEED, NULL);
	return get_resp_value(ret);
}

int loc_x()
{
	int ret;

	ret = sockwrite(serverfd, LOC_X, NULL);
	return get_resp_value(ret);
}

int loc_y()
{
	int ret;

	ret = sockwrite(serverfd, LOC_Y, NULL);
	return get_resp_value(ret);
}

void set_name(char *name)
{
	int ret, i;
	char sanitized[15];

	for (i = 0; i < 14 && name[i]; i++) {
		if (isspace(name[i]))
			sanitized[i] = '_';
		else
			sanitized[i] = name[i];
	}
	sanitized[i] = 0;
	ret = sockwrite(serverfd, NAME, "%s", sanitized);
	get_resp_value(ret);
}

int image(char *path)
{
	FILE *f;
	void *data;
	long fsize;
	int ret = 0;

	f = fopen(path, "rb");
	if (!f) {
		ndprintf(stderr, "[WARNING] Cannot find file %s\n", path);
		return 0;
	}
	fseek(f, 0, SEEK_END);
	fsize = ftell(f);
	if (fsize < 0 || fsize > MAX_IMAGE_BYTES) {
		ndprintf(stderr, "[WARNING] File %s longer than %d bytes\n",
			 path, MAX_IMAGE_BYTES);
		goto out_close;
	}
	rewind(f);
	data = malloc(fsize);
	if (!data) {
		ndprintf(stderr, "[WARNING] Cannot alloc memory\n");
		goto out_close;
	}
	if (fread(data, 1, fsize, f) != fsize) {
		ndprintf(stderr, "[WARNING] Error reading from %s\n", path);
		goto out_free;
	}
	ret = sockwrite(serverfd, IMAGE, "%ld", fsize);
	get_resp_value(ret);
	ret = write(serverfd, data, fsize);
	get_resp_value(ret);
out_free:
	free(data);
out_close:
	fclose(f);
	return ret;
}
