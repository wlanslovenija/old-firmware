#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>

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
