#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>

#define VERSION "0.1"
#define DEFAULT_DEVICE_FILE "/dev/ttyS0"
#define DEFAULT_BAUD B9600
#define DEFAULT_BAUD_NAME 9600
#define DEFAULT_INTERFACE "pli"
#define IO_WAIT 10

typedef struct {
	char *name;
	char *description;
	int (*function)(int fd);
} command;

typedef struct {
	char *name;
	command *commands;
} interface;

extern int plain_output;

int help(int fd);
int version(int fd);
void printhelp(FILE *output);
void lockwait(int signal);
int openserialport();

#endif /* MAIN_H_ */
