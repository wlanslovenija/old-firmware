#ifndef PLI_H_
#define PLI_H_

#import "main.h"

#define RETRY 10
#define INTLOAD_DIV 10.0 // PL20/PL40 = 10.0, PL60 = 5.0
#define INTCHARGE_DIV 10.0  // PL20 = 10.0, PL40 = 5.0, PL60 = 2.5
#define CONFIGURATION_START 0x0E
#define CONFIGURATION_END 0x2C
#define CONFIGURATION_SIZE (CONFIGURATION_END - CONFIGURATION_START + 1)

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
int pli_plversion(int fd);
int pli_getday(int fd);
int pli_gettime(int fd);
int pli_setdaytime(int fd);
int pli_batcapacity(int fd);
int pli_batvoltage(int fd);
int pli_solvoltage(int fd);
int pli_charge(int fd);
int pli_load(int fd);
int pli_state(int fd);
int pli_save(int fd);
int pli_restore(int fd);
int pli_powercycle(int fd);

#endif /* PLI_H_ */
