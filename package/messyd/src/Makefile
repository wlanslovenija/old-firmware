# $Id$

all: messyd

%.o: %.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -c -Iinclude -o $@ $^

messyd: libnetlink.o ll_map.o ll_addr.o utils.o rtm_map.o rt_names.o dnet_pton.o dnet_ntop.o messyd.o
	$(CC) -o $@ $^

clean:
	rm -f *.o messyd
