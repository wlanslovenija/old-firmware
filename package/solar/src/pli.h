#ifndef PLI_H_
#define PLI_H_

#import "main.h"

extern command pli_commands[];

int write_buffer(int fd, unsigned char buffer[]);
int read_buffer(int fd, unsigned char buffer[], int size);
void printerror(unsigned char code);
int read_processor(int fd, int location);
int read_eprom(int fd, int location);
int write_processor(int fd, int location, unsigned char data);
int write_eprom(int fd, int location, unsigned char data);
int long_push(int fd);
int short_push(int fd);

int pli_test(int fd);
int pli_version(int fd);
int pli_getday(int fd);
int pli_gettime(int fd);
int pli_setdaytime(int fd);
int pli_batcapacity(int fd);
int pli_batvoltage(int fd);
int pli_solvoltage(int fd);
int pli_charge(int fd);
int pli_load(int fd);
int pli_state(int fd);
int pli_powercycle(int fd);

#endif /* PLI_H_ */
