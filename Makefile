PREFIX ?= /usr/local

CFLAGS += -std=c99 -Wall -Wextra -fPIC -pedantic
CPPFLAGS += -I./include

ifeq ($(CC),gcc)
    CFLAGS += -ggdb3
endif
ifeq ($(CC),clang)
    CFLAGS += -Qunused-arguments -ggdb -Weverything
endif

VPATH = src
OBJS := rdslab.o rdheap.o

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
	install -m 0755 include/rdslab.h $(PREFIX)/include

.PHONY: clean
clean:
	rm -f *.a *.o *.so
