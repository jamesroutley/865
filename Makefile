TARGET=865
BUILDDIR=bin/
PREFIX=/usr/local/bin/
MAIN=main.c
override CFLAGS+=-Werror -Wall -g -fPIC -O2 -DNDEBUG -ftrapv -Wfloat-equal -Wundef -Wwrite-strings -Wuninitialized -pedantic -std=gnu11

all: main.c
	mkdir -p $(BUILDDIR)
	$(CC) $(MAIN) -o $(BUILDDIR)$(TARGET) $(CFLAGS)

install: all
	install $(BUILDDIR)$(TARGET) $(PREFIX)$(TARGET)

uninstall:
	rm -rf $(PREFIX)$(TARGET)
