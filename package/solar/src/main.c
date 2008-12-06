#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>

#include "main.h"
#include "pli.h"

char *device = DEFAULT_DEVICE_FILE;
speed_t baud = DEFAULT_BAUD;
int plain_output = 0;
interface *iface;

interface interfaces[] = {
	{"pli", pli_commands},
	{NULL, NULL}
};

int help(int fd) {
	if ((plain_output != 0) && (iface->name != NULL)) {
		fprintf(stdout, "help\n");
		fprintf(stdout, "version\n");
		command *j;
		for (j = iface->commands; j->name != NULL; j++) {
			fprintf(stdout, "%s\n", j->name);
		}
	}
	else {
		printhelp(stdout);
	}
	return 0;
}

int version(int fd) {
	fprintf(stdout, "%s%s\n", ((plain_output != 0) ? "" : "Version: "), VERSION);
	return 0;
}

void printhelp(FILE *output) {
	fprintf(output, "solar[.<iface>] [-s] [-d <device>] [-b <baud>] <command>\n");
	fprintf(output, "  -p           plain (just values) output\n");
	fprintf(output, "  -d <device>  use <device> as a serial port device file (default: %s)\n", DEFAULT_DEVICE_FILE);
	fprintf(output, "  -b <baud>    communicate with <baud> baud over a serial port (default: %d)\n", DEFAULT_BAUD_NAME);
	fprintf(output, "\n");
	fprintf(output, "  <iface>    which interface to use (default: %s, possible:", DEFAULT_INTERFACE);
	interface *i;
	for (i = interfaces; i->name != NULL; i++) {
		fprintf(output, " %s", i->name);
	}
	fprintf(output, ")\n");
	fprintf(output, "  <baud>     baud of communication over a serial port (integer)\n");
	fprintf(output, "  <device>   path to a serial port device file\n");
	if (iface->name == NULL) {
		fprintf(output, "  <command>  command of interface to execute (possible commands bellow)\n");
	}
	else {
		fprintf(output, "  <command>  command of '%s' interface to execute (possible commands bellow)\n", iface->name);
	}
	int maxname = 7;
	if (iface->name != NULL) {
		command *j;
		for (j = iface->commands; j->name != NULL; j++) {
			if (strlen(j->name) > maxname) maxname = strlen(j->name);
		}
	}
	fprintf(output, "\n");
	fprintf(output, "  %-*s  %s\n", maxname, "help", "display this help");
	fprintf(output, "  %-*s  %s %s\n", maxname, "version", "display version of this program, that is", VERSION);
	if (iface->name != NULL) {
		command *j;
		for (j = iface->commands; j->name != NULL; j++) {
			fprintf(output, "  %-*s  %s\n", maxname, j->name, j->description);
		}
	}
}

void lockwait(int signal) {
	fprintf(stderr, "Timeout while waiting for serial port device file '%s'.\n", device);
	exit(2);
}

int openserialport() {
	int fd;
	struct termios params;

	// We have to use O_NONBLOCK because otherwise open blocks if serial port is not yet connected
	if ((fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK)) == -1) return -1;
	if (fcntl(fd, F_SETFL, 0) == -1) return -1; // We clear O_NONBLOCK
	if (tcgetattr(fd, &params) == -1) return -1;

	cfmakeraw(&params);
	cfsetspeed(&params, baud);

	// 8 bit data, one stop bit, RTS/CTS flow control
	params.c_cflag = CLOCAL | CREAD | CS8 | HUPCL | CRTSCTS;

	if (tcsetattr(fd, TCSANOW, &params) == -1) return -1;

	// Will wait IO_WAIT seconds for the lock
	alarm(IO_WAIT);

	if (flock(fd, LOCK_EX) == -1) {
		int e = errno;

		// Disables alarm
		alarm(0);

		return e;
	}

	// Disables alarm
	alarm(0);

	return fd;
}

int main(int argc, char *argv[]) {
	// Gets the extension of the program filename
	char *iname = strrchr(argv[0], '/');
	if (iname == NULL) iname = argv[0];
	iname = strrchr(iname, '.');
	iname = (iname != NULL) ? (iname + 1) : DEFAULT_INTERFACE;

	for (iface = interfaces; iface->name != NULL; iface++) {
		if (strcmp(iface->name, iname) == 0) break;
	}

	if (iface->name == NULL) {
		fprintf(stderr, "Unsupported interface '%s'.\n\n", iname);
		printhelp(stderr);
		return 1;
	}

	signal(SIGALRM, lockwait); // If we get SIGALRM this probably means that we timeout on lock

	int (*c)(int fd) = NULL;

	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-d") == 0) {
			i++;
			if ((i < argc) && (argv[i][0] != '\0')) {
				device = argv[i];
			}
			else {
				fprintf(stderr, "Missing parameter for -d argument.\n\n");
				printhelp(stderr);
				return 1;
			}
		}
		else if (strcmp(argv[i], "-p") == 0) {
			plain_output = 1;
		}
		else if (strcmp(argv[i], "-b") == 0) {
			i++;
			if ((i < argc) && (argv[i][0] != '\0')) {
				char *end;
				int b = strtol(argv[i], &end, 10);
				if (*end != '\0') {
					fprintf(stderr, "Invalid parameter '%s' for -b argument.\n\n", argv[i]);
					printhelp(stderr);
					return 1;
				}
				switch (b) {
					case 50:
						baud = B50;
						break;
					case 75:
						baud = B75;
						break;
					case 110:
						baud = B110;
						break;
					case 134:
						baud = B134;
						break;
					case 150:
						baud = B150;
						break;
					case 200:
						baud = B200;
						break;
					case 300:
						baud = B300;
						break;
					case 600:
						baud = B600;
						break;
					case 1200:
						baud = B1200;
						break;
					case 1800:
						baud = B1800;
						break;
					case 2400:
						baud = B2400;
						break;
					case 4800:
						baud = B4800;
						break;
					case 9600:
						baud = B9600;
						break;
					case 19200:
						baud = B19200;
						break;
					case 38400:
						baud = B38400;
						break;
					case 57600:
						baud = B57600;
						break;
					case 115200:
						baud = B115200;
						break;
					case 230400:
						baud = B230400;
						break;
					default:
						fprintf(stderr, "Invalid parameter '%s' for -b argument.\n\n", argv[i]);
						printhelp(stderr);
						return 1;
				}
			}
			else {
				fprintf(stderr, "Missing parameter for -b argument.\n\n");
				printhelp(stderr);
				return 1;
			}
		}
		else {
			if (c != NULL) {
				fprintf(stderr, "Unsupported or unexpected command '%s' of interface '%s'.\n\n", argv[i], iface->name);
				printhelp(stderr);
				return 1;
			}

			command *j;
			for (j = iface->commands; j->name != NULL; j++) {
				if (strcmp(j->name, argv[i]) == 0) {
					c = j->function;
					break;
				}
			}

			if (c == NULL) {
				if (strcmp(argv[i], "help") == 0) {
					c = help;
				}
				else if (strcmp(argv[i], "version") == 0) {
					c = version;
				}
				else {
					fprintf(stderr, "Unsupported command '%s' of interface '%s'.\n\n", argv[i], iface->name);
					printhelp(stderr);
					return 1;
				}
			}
		}
	}

	if (c == NULL) {
		fprintf(stderr, "Missing command argument of interface '%s'.\n\n", iface->name);
		printhelp(stderr);
		return 1;
	}

	int fd;
	if ((c != help) && (c != version) && ((fd = openserialport()) == -1)) {
		fprintf(stderr, "Unable to open serial port device file '%s': %s.\n", device, strerror(errno));
		return 2;
	}

	int ret = c(fd);

	if ((c != help) && (c != version) && (close(fd) == -1)) {
		fprintf(stderr, "Unable to close serial port device file '%s': %s.\n", device, strerror(errno));
		return 2;
	}

	return ret;
}
