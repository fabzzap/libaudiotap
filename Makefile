audiotap.dll: libaudiotap.c audiotap.def
	$(CC) $(CFLAGS) -shared -I../libtap -Wl,--out-implib=audiotap.lib -o $@ $^ ../libtap/tap.lib $(LDFLAGS)

clean:
	rm -f *.o *.dll *.lib *~ *.so

libaudiotap.so: libaudiotap.c
	$(CC) $(CFLAGS) -shared -I../libtap -o $@ $^ -ltap -L../libtap $(LDFLAGS)
