all:audiotap.dll

%.dll: lib%.o lib%_external_symbols.o %.def
	$(CC) -shared -Wl,--out-implib=libaudiotap.a -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o *.dll *.lib *~ *.so

libaudiotap.so: libaudiotap.o libaudiotap_external_symbols.o
	$(CC) -shared -o $@ $^ -ldl $(LDFLAGS)

LIBTAP_DIR=../libtap

ifdef DEBUG
 CFLAGS+=-g
endif

ifdef LINUX64BIT
 CFLAGS+=-fPIC
endif

ifdef USE_RPATH
 LDFLAGS=-Wl,-rpath=$(LIBTAP_DIR)
endif

