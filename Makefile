all:audiotap.dll

%.dll: lib%.c %.def
	$(CC) $(CFLAGS) -shared -I../libtap -Wl,--out-implib=audiotap.lib -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o *.dll *.lib *~ *.so

%.so: %.c
	$(CC) $(CFLAGS) -shared -I../libtap -o $@ $^ -ldl $(LDFLAGS)

