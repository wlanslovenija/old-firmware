#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

#include "pli.h"
#include "main.h"

#define RETRY 10
#define LOFF 80 // 8.0 V
#define LON 100 // 10.0 V
#define LDEL 10 // 1 min
#define INTLOAD_DIV 10.0 // PL20/PL40 = 10.0, PL60 = 5.0
#define INTCHARGE_DIV 10.0  // PL20 = 10.0, PL40 = 5.0, PL60 = 2.5
#define CONFIGURATION_START 0x0E
#define CONFIGURATION_END 0x2C
#define CONFIGURATION_SIZE (CONFIGURATION_END - CONFIGURATION_START + 1)

command pli_commands[] = {
	{"test", "loopback test connection to PLI", pli_test},
	{"plversion", "get PL software version", pli_plversion},
	{"getday", "get current day in a month", pli_getday},
	{"gettime", "get current time", pli_gettime},
	{"setdaytime", "set current day and time from local time on this system", pli_setdaytime},
	{"batcapacity", "get battery capacity configuration", pli_batcapacity},
	{"batvoltage", "get current battery voltage", pli_batvoltage},
	{"solvoltage", "get current solar voltage", pli_solvoltage},
	{"charge", "get current charging current", pli_charge},
	{"load", "get current load current", pli_load},
	{"state", "get current regulator state", pli_state},
	{"save", "save current configuration to 'solar.conf'", pli_save},
	{"restore", "restore configuration from 'solar.conf'", pli_restore},
	//{"powercycle", "powercycle load (switch power off and the back on)", pli_powercycle}, not really working (it does not turn the load back on)
	{NULL, NULL}
};

int write_buffer(int fd, unsigned char *buffer) {
	int count;
	int i = 0;
	int w = 0;

	while ((count = write(fd, buffer + w, sizeof(buffer) - w)) != sizeof(buffer) - w) {
		if (count == -1) {
			fprintf(stderr, "Could not write command: %s.\n", strerror(errno));
			return 2;
		}
		else if (i < RETRY) {
			w =+ count;
			i++;
		}
		else {
			fprintf(stderr, "Could not write complete command buffer.\n");
			return 2;
		}
	}

	// Communication with PLI can immediately follow (read syscall is successful)
	// but if PLI does not have data from the regulator yet it returns sent buffer
	// We wait approximately 200 ms as specified in the documentation, probably
	// could wait less
	usleep(200000);

	return 0;
}

int read_buffer(int fd, unsigned char *buffer, int size) {
	int count;
	int i = 0;
	int r = 0;

	while ((count = read(fd, buffer + r, size - r)) != size - r) {
		if (count == -1) {
			fprintf(stderr, "Could not read response: %s.\n", strerror(errno));
			return 2;
		}
		else if (i < RETRY) {
			r += count;
			i++;
		}
		else {
			fprintf(stderr, "Could not read complete response buffer.\n");
			return 2;
		}
	}

	return 0;
}

void printerror(unsigned char code) {
	if (plain_output != 0) return;

	fprintf(stdout, "Command failed");
	switch (code) {
		case 0x81:
			fprintf(stdout, ": timeout error.\n");
			break;
		case 0x82:
			fprintf(stdout, ": checksum error in PLI receive data.\n");
			break;
		case 0x83:
			fprintf(stdout, ": command received by PLI is not recognised.\n");
			break;
		case 0x85:
			fprintf(stdout, ": processor did not receive a reply to request.\n");
			break;
		case 0x86:
			fprintf(stdout, ": error in reply from PL.\n");
			break;
		default:
			fprintf(stdout, ".\n");
			break;
	}
}

int read_processor(int fd, int location) {
	unsigned char buffer[] = {0x14, location, 0x00, 0x14 ^ 0xFF};

	if (write_buffer(fd, buffer)) return -1;
	if (read_buffer(fd, buffer, 2)) return -1;

	if (buffer[0] == 0xC8) {
		return buffer[1];
	}
	else {
		printerror(buffer[0]);
		return -1;
	}
}

int read_eprom(int fd, int location) {
	unsigned char buffer[] = {0x48, location, 0x00, 0x48 ^ 0xFF};

	if (write_buffer(fd, buffer)) return -1;
	if (read_buffer(fd, buffer, 2)) return -1;

	if (buffer[0] == 0xC8) {
		return buffer[1];
	}
	else {
		printerror(buffer[0]);
		return -1;
	}
}

int write_processor(int fd, int location, unsigned char data) {
	unsigned char buffer[] = {0x98, location, data, 0x98 ^ 0xFF};

	if (write_buffer(fd, buffer)) return -1;

	return 0;
}

int write_eprom(int fd, int location, unsigned char data) {
	unsigned char buffer[] = {0xCA, location, data, 0xCA ^ 0xFF};

	if (write_buffer(fd, buffer)) return -1;

	return 0;
}

int long_push(int fd) {
	unsigned char buffer[] = {0x57, 0x02, 0x00, 0x57 ^ 0xFF};

	if (write_buffer(fd, buffer)) return -1;

	return 0;
}

int short_push(int fd) {
	unsigned char buffer[] = {0x57, 0x01, 0x00, 0x57 ^ 0xFF};

	if (write_buffer(fd, buffer)) return -1;

	return 0;
}

int pli_test(int fd) {
	unsigned char buffer[] = {0xBB, 0x00, 0x00, 0xBB ^ 0xFF};

	if (write_buffer(fd, buffer)) return 3;
	if (read_buffer(fd, buffer, 1)) return 3;

	if (buffer[0] == 0x80) {
		if (plain_output == 0) fprintf(stdout, "Test successful.\n");
		return 0;
	}
	else {
		if (plain_output == 0) fprintf(stdout, "Test failed.\n");
		return 3;
	}
}

int pli_plversion(int fd) {
	int version;
	if ((version = read_processor(fd, 0x00)) == -1) {
		return 3;
	}

	fprintf(stdout, "%s%d\n", ((plain_output != 0) ? "" : "Version: "), version);
	return 0;
}

int pli_getday(int fd) {
	int day;
	if ((day = read_processor(fd, 0x31)) == -1) return 3;

	fprintf(stdout, "%s%d\n", ((plain_output != 0) ? "" : "Day: "), day);
	return 0;
}

int pli_gettime(int fd) {
	int hour;
	if ((hour = read_processor(fd, 0x30)) == -1) return 3;

	int min;
	if ((min = read_processor(fd, 0x2F)) == -1) return 3;

	int sec;
	if ((sec = read_processor(fd, 0x2E)) == -1) return 3;

	fprintf(stdout, "%s%02d:%02d:%02d\n", ((plain_output != 0) ? "" : "Time: "), hour / 10, ((hour % 10) * 6) + min, sec);
	return 0;
}

int pli_setdaytime(int fd) {
	time_t rawtime;
	if (time(&rawtime) == -1) {
		fprintf(stderr, "Could not get local time: %s.\n", strerror(errno));
		return 2;
	}
	struct tm *timeinfo = localtime(&rawtime);

	if (write_processor(fd, 0x31, timeinfo->tm_mday - 1) == -1) return 3;
	if (write_processor(fd, 0x30, (timeinfo->tm_hour * 10) + (timeinfo->tm_min / 6)) == -1) return 3;
	if (write_processor(fd, 0x2F, timeinfo->tm_min % 6) == -1) return 3;
	if (write_processor(fd, 0x2E, timeinfo->tm_sec) == -1) return 3;

	return 0;
}

int pli_batcapacity(int fd) {
	int bcap;
	if ((bcap = read_processor(fd, 0x5E)) == -1) return 3;

	fprintf(stdout, "%s%d\n", ((plain_output != 0) ? "" : "Battery capacity (Ah): "), ((bcap <= 50) ? (bcap * 20) : ((bcap - 50) * 100)));
	return 0;
}

int pli_batvoltage(int fd) {
	int vdiv;
	if ((vdiv = read_processor(fd, 0x20)) == -1) return 3;

	int batv;
	if ((batv = read_processor(fd, 0x32)) == -1) return 3;

	batv = batv * (vdiv + 1);
	fprintf(stdout, "%s%.1f\n", ((plain_output != 0) ? "" : "Battery voltage (V): "), (double)batv / 10.0);
	return 0;
}

int pli_solvoltage(int fd) {
	// Repeats three times to be sure
	if (write_processor(fd, 0x29, 0x00) == -1) return 3; // Wakes up the display
	if (write_processor(fd, 0x29, 0x00) == -1) return 3; // Wakes up the display
	if (write_processor(fd, 0x29, 0x00) == -1) return 3; // Wakes up the display

	if (write_processor(fd, 0x66, 0x27) == -1) return 3; // Selects solv display

	// Sleeps three seconds for measurement to stabilize
	sleep(3);

	int solv;
	if ((solv = read_processor(fd, 0x35)) == -1) return 3;

	if (write_processor(fd, 0x66, 0x00) == -1) return 3; // Selects initial display

	// Repeats three times to be sure
	if (write_processor(fd, 0x29, 0x10) == -1) return 3; // Puts the display to sleep
	if (write_processor(fd, 0x29, 0x10) == -1) return 3; // Puts the display to sleep
	if (write_processor(fd, 0x29, 0x10) == -1) return 3; // Puts the display to sleep

	fprintf(stdout, "%s%.1f\n", ((plain_output != 0) ? "" : "Solar voltage (V): "), (double)solv / 2.0);
	return 0;
}

int pli_charge(int fd) {
	int cint;
	if ((cint = read_processor(fd, 0xD5)) == -1) return 3;

	int cext;
	if ((cext = read_processor(fd, 0xCD)) == -1) return 3;

	int extf;
	if ((extf = read_processor(fd, 0xCF)) == -1) return 3;

	fprintf(stdout, "%s%.1f\n", ((plain_output != 0) ? "" : "Charging current (A): "), (double)cint / INTCHARGE_DIV + (double)cext / (((extf & 0x01) == 0) ? 10.0 : 1.0));
	return 0;
}

int pli_load(int fd) {
	int lint;
	if ((lint = read_processor(fd, 0xD9)) == -1) return 3;

	int lext;
	if ((lext = read_processor(fd, 0xCE)) == -1) return 3;

	int extf;
	if ((extf = read_processor(fd, 0xCF)) == -1) return 3;

	fprintf(stdout, "%s%.1f\n", ((plain_output != 0) ? "" : "Load current (A): "), (double)lint / INTLOAD_DIV + (double)lext / (((extf & 0x02) == 0) ? 10.0 : 1.0));
	return 0;
}

int pli_state(int fd) {
	int rstate;
	if ((rstate = read_processor(fd, 0x65)) == -1) return 3;

	char *state;
	switch (rstate & 0x03) {
		case 0:
			state = "boost";
			break;
		case 1:
			state = "equalize";
			break;
		case 2:
			state = "absorption";
			break;
		case 3:
			state = "float";
			break;
	}
	fprintf(stdout, "%s%s\n", ((plain_output != 0) ? "" : "Regulator state: "), state);
	return 0;
}

int pli_save(int fd) {
	unsigned char buffer[CONFIGURATION_SIZE];
	int i;
	int val;
	for (i = CONFIGURATION_START; i <= CONFIGURATION_END; i++) {
		if ((val = read_eprom(fd, i)) == -1) return 3;
		buffer[i - CONFIGURATION_START] = val;
	}

	int file;
	if ((file = open("solar.conf", O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) {
		fprintf(stderr, "Could not open configuration file 'solar.conf': %s.\n", strerror(errno));
		return 2;
	}

	int count;
	int w = 0;

	while ((count = write(file, buffer + w, sizeof(buffer) - w)) != sizeof(buffer) - w) {
		if (count == -1) {
			fprintf(stderr, "Could not write configuration file 'solar.conf': %s.\n", strerror(errno));
			return 2;
		}
		else if (i < RETRY) {
			w =+ count;
			i++;
		}
		else {
			fprintf(stderr, "Could not write complete configuration file 'solar.conf'.\n");
			return 2;
		}
	}

	if (close(file) == -1) {
		fprintf(stderr, "Unable to close configuration file 'solar.conf': %s.\n", strerror(errno));
		return 2;
	}

	return 0;
}

int pli_restore(int fd) {
	int file;
	if ((file = open("solar.conf", O_RDONLY)) == -1) {
		fprintf(stderr, "Could not open configuration file 'solar.conf': %s.\n", strerror(errno));
		return 2;
	}

	unsigned char buffer[CONFIGURATION_SIZE];
	int count;
	int i = 0;
	int r = 0;

	while ((count = read(file, buffer + r, sizeof(buffer) - r)) != sizeof(buffer) - r) {
		if (count == -1) {
			fprintf(stderr, "Could not read configuration file 'solar.conf': %s.\n", strerror(errno));
			return 2;
		}
		else if (i < RETRY) {
			r += count;
			i++;
		}
		else {
			fprintf(stderr, "Could not read complete configuration file 'solar.conf'.\n");
			return 2;
		}
	}

	if (close(file) == -1) {
		fprintf(stderr, "Unable to close configuration file 'solar.conf': %s.\n", strerror(errno));
		return 2;
	}

	for (i = CONFIGURATION_START; i <= CONFIGURATION_END; i++) {
		if (write_eprom(fd, i, buffer[i - CONFIGURATION_START]) == -1) return 3;
	}

	return 0;
}

// It powercycles load - temporary switches power off then waits LDEL minutes
// before it switches power back on
// It works only if battery voltage is over LON, otherwise the power will stay
// off until battery voltage reaches LON
int pli_powercycle(int fd) {
	// Repeats three times to be sure
	if (write_processor(fd, 0x29, 0x00) == -1) return 3; // Wakes up the display
	if (write_processor(fd, 0x29, 0x00) == -1) return 3; // Wakes up the display
	if (write_processor(fd, 0x29, 0x00) == -1) return 3; // Wakes up the display

	if ((write_eprom(fd, 0x15, LDEL)) == -1) return 3; // Sets delay before switching on/off
	if ((write_eprom(fd, 0x13, LOFF)) == -1) return 3; // Sets voltage at which load disconnects
	if ((write_eprom(fd, 0x14, LON)) == -1) return 3; // Sets voltage at which load reconnects

	if ((write_processor(fd, 0x47, LDEL)) == -1) return 3; // Sets delay before switching on/off
	if ((write_processor(fd, 0x45, LOFF)) == -1) return 3; // Sets voltage at which load disconnects
	if ((write_processor(fd, 0x46, LON)) == -1) return 3; // Sets voltage at which load reconnects

	if (write_processor(fd, 0x66, 0x17) == -1) return 3; // Selects lset display

	if (long_push(fd) == -1) return 3; // Sends a long push command

	// Will probably never reach the regulator if this program is running on a system
	// powered by the regulator as system's power will be turned off

	// Repeats three times to be sure
	if (write_processor(fd, 0x29, 0x10) == -1) return 3; // Puts the display to sleep
	if (write_processor(fd, 0x29, 0x10) == -1) return 3; // Puts the display to sleep
	if (write_processor(fd, 0x29, 0x10) == -1) return 3; // Puts the display to sleep

	return 0;
}
