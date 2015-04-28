PREFIX ?= /usr/local

CFLAGS += -std=c99 -ggdb3 -Wall -Wextra -ggdb3 -fPIC -pedantic
CPPFLAGS += -I./include

ifeq ($(CC),gcc)
    CFLAGS += -ggdb3
endif
ifeq ($(CC),clang)
    CFLAGS += -Qunused-arguments -ggdb3 -Weverything
endif

VPATH = src
OBJS := rdcache.o rdslab.o rdheap.o

ARFLAGS=rvs

.PHONY: all
all: librdslab.so

.PHONY: static
static: librdslab.a

$(OBJS): %.o: src/%.c

librdslab.so: $(OBJS)
	$(CC) -shared -o $@ $^ $(LDLIBS)

librdslab.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $^


.PHONY: install
install:
	install -m 0755 librdslab.so $(PREFIX)/lib
	install -m 0755 include/rdcache.h $(PREFIX)/include

.PHONY: clean
clean:
	rm -f *.a *.o *.so
