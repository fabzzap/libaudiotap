all:audiotap.dll

ifdef WITH_VERSION
  RESOURCE_OBJECT=lib%-resource.o
endif

%.dll: lib%.o lib%_external_symbols_delayload.o windows_wait_event.o %.def ext-weaklink/libaudiofile.a ext-weaklink/libportaudio.a ext-weaklink/libtapencoder.a ext-weaklink/libtapdecoder.a $(RESOURCE_OBJECT)
	$(CC) -shared -static-libgcc -Wl,--out-implib=libaudiotap.a -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o *.dll *.lib *~ *.so *.dylib ext-weaklink/*.a

libaudiotap.so: libaudiotap.o libaudiotap_external_symbols.o pthread_wait_event.o
	$(CC) -shared -o $@ $^ -ldl $(LDFLAGS)

libaudiotap.so: AUDIOFILE_HDR=-isystem ext
libaudiotap.so: PORTAUDIO_HDR=-isystem ext
audiotap.dll: AUDIOFILE_HDR=-isystem ext-weaklink
audiotap.dll: PORTAUDIO_HDR=-isystem ext-weaklink
libaudiotap.dylib: AUDIOFILE_HDR=-isystem ext-weaklink
libaudiotap.dylib: PORTAUDIO_HDR=-isystem ext-weaklink

libaudiotap.dylib: libaudiotap.o libaudiotap_external_symbols_weaklink.o pthread_wait_event.o
	$(CC) -shared -o $@ $^ $(LDLIBS) -Lext-weaklink -weak-laudiofile -weak-lportaudio -weak-ltapencoder -weak-ltapdecoder

CFLAGS+=$(AUDIOFILE_HDR) $(PORTAUDIO_HDR)
libaudiotap.dylib: LDLIBS+=$(if $(AUDIOFILE_LIB), -L$(AUDIOFILE_LIB))
libaudiotap.dylib: LDLIBS+=$(if $(PORTAUDIO_LIB), -L$(PORTAUDIO_LIB))
libaudiotap.dylib: LDLIBS+=$(if $(TAPENCODER_LIB), -L$(TAPENCODER_LIB))
libaudiotap.dylib: LDLIBS+=$(if $(TAPDECODER_LIB), -L$(TAPDECODER_LIB))

ifdef DEBUG
 CFLAGS+=-g
endif

ifdef OPTIMISE
 CFLAGS+=-O3
endif

ifdef LINUX64BIT
 CFLAGS+=-fPIC
endif

ifdef USE_RPATH
 LDFLAGS=-Wl,-rpath=$(USE_RPATH)
endif

WINDRES=windres

%-resource.o: %-resource.rc
	$(WINDRES) -o $@ $^

ext-weaklink/libaudiofile.a: DLL_NAME=libaudiofile-1
ext-weaklink/libportaudio.a: DLL_NAME=libportaudio-2
ext-weaklink/lib%.a: ext-weaklink/%.def
	dlltool --output-delaylib $@ --def $^ --dllname $(if $(DLL_NAME),$(DLL_NAME),$*).dll
