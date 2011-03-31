all:audiotap.dll

%.dll: lib%.c %.def
	$(CC) $(CFLAGS) -shared -I../libtap -Wl,--out-implib=audiotap.lib -o $@ $^ ../libtap/tap.lib $(LDFLAGS)

clean:
	rm -f *.o *.dll *.lib *~ *.so

%.so: %.c
	$(CC) $(CFLAGS) -shared -I../libtap -o $@ $^ -ltapencoder -ltapdecoder -ldl -L../libtap $(LDFLAGS)

