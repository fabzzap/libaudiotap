/* Audiotap shared library: a higher-level interface to TAP shared library
 *
 * libaudiotap_external_symbols.c: load optional libraries, and relative symbols.
 * This allows Audiotap tp work even when the needed libraries are not there,
 * at least as a TAP reader/writer.
 *
 * Copyright (c) Fabrizio Gennari, 2012
 *
 * The program is distributed under the GNU Lesser General Public License.
 * See file LESSER-LICENSE.TXT for details.
 */

#define AUDIOFILE_DECLARE_HERE
#include "audiofile.h"
#define PORTAUDIO_DECLARE_HERE
#include "portaudio.h"
#define TAPENCODER_DECLARE_HERE
#include "tapencoder.h"
#define TAPDECODER_DECLARE_HERE
#include "tapdecoder.h"
#include "audiotap.h"

#ifndef _WIN32
#include <dlfcn.h>
#else
#include <windows.h>
#endif

#if __GNUC__ >= 4
 __attribute__ ((visibility ("hidden")))
#endif
struct audiotap_init_status status = {
  LIBRARY_UNINIT,
  LIBRARY_UNINIT,
  LIBRARY_UNINIT,
  LIBRARY_UNINIT
};

static enum library_status audiofile_init(){
#if defined(WIN32)
  HINSTANCE handle;
#else
  void *handle;
#endif

static const char* audiofile_library_name =
#if (defined _MSC_VER)
#ifdef _DEBUG
"audiofiled.dll"
#else //_MSC_VER and not DEBUG
"audiofile.dll"
#endif //_MSC_VER,DEBUG
#elif (defined _WIN32 || defined __CYGWIN__)
#ifdef AUDIOFILE_COMPILED_WITH_CYGWIN_SHELL
"cygaudiofile-1.dll"
#else //not AUDIOFILE_COMPILED_WITH_CYGWIN_SHELL
"libaudiofile-1.dll"
#endif //_WIN32 or __CYGWIN__,AUDIOFILE_COMPILED_WITH_CYGWIN_SHELL
#elif defined __APPLE__
"libaudiofile.0.0.2.dylib"
#else //not _MSC_VER, not _WIN32, not __CYGWIN__
"libaudiofile.so.0"
#endif//_MSC_VER,_WIN32 or __CYGWIN__
;

#if defined(WIN32)
  handle=LoadLibraryA(audiofile_library_name);
#else
  handle=dlopen(audiofile_library_name, RTLD_LAZY);
#endif
  if (!handle) {
    return LIBRARY_MISSING;
  }

#if defined(WIN32)
  afOpenFile = (void*)GetProcAddress(handle, "afOpenFile");
#else
  afOpenFile = dlsym(handle, "afOpenFile");
#endif
  if (!afOpenFile) {
    return LIBRARY_SYMBOLS_MISSING;
  }

#if defined(WIN32)
  afCloseFile = (void*)GetProcAddress(handle, "afCloseFile");
#else
  afCloseFile = dlsym(handle, "afCloseFile");
#endif
  if (!afCloseFile) {
    return LIBRARY_SYMBOLS_MISSING;
  }

#if defined(WIN32)
  afReadFrames = (void*)GetProcAddress(handle, "afReadFrames");
#else
  afReadFrames = dlsym(handle, "afReadFrames");
#endif
  if (!afReadFrames) {
    return LIBRARY_SYMBOLS_MISSING;
  }

#if defined(WIN32)
  afWriteFrames = (void*)GetProcAddress(handle, "afWriteFrames");
#else
  afWriteFrames = dlsym(handle, "afWriteFrames");
#endif
  if (!afWriteFrames) {
    return LIBRARY_SYMBOLS_MISSING;
  }

#if defined(WIN32)
  afSeekFrame = (void*)GetProcAddress(handle, "afSeekFrame");
#else
  afSeekFrame = dlsym(handle, "afSeekFrame");
#endif
  if (!afSeekFrame) {
    return LIBRARY_SYMBOLS_MISSING;
  }

#if defined(WIN32)
  afSetVirtualChannels = (void*)GetProcAddress(handle, "afSetVirtualChannels");
#else
  afSetVirtualChannels = dlsym(handle, "afSetVirtualChannels");
#endif
  if (!afSetVirtualChannels) {
    return LIBRARY_SYMBOLS_MISSING;
  }

#if defined(WIN32)
  afGetSampleFormat = (void*)GetProcAddress(handle, "afGetSampleFormat");
#else
  afGetSampleFormat = dlsym(handle, "afGetSampleFormat");
#endif
  if (!afGetSampleFormat) {
    return LIBRARY_SYMBOLS_MISSING;
  }

#if defined(WIN32)
  afSetVirtualSampleFormat = (void*)GetProcAddress(handle, "afSetVirtualSampleFormat");
#else
  afSetVirtualSampleFormat = dlsym(handle, "afSetVirtualSampleFormat");
#endif
  if (!afSetVirtualSampleFormat) {
    return LIBRARY_SYMBOLS_MISSING;
  }

#if defined(WIN32)
  afGetVirtualFrameSize = (void*)GetProcAddress(handle, "afGetVirtualFrameSize");
#else
  afGetVirtualFrameSize = dlsym(handle, "afGetVirtualFrameSize");
#endif
  if (!afGetVirtualFrameSize) {
    return LIBRARY_SYMBOLS_MISSING;
  }

#if defined(WIN32)
  afGetFrameCount = (void*)GetProcAddress(handle, "afGetFrameCount");
#else
  afGetFrameCount = dlsym(handle, "afGetFrameCount");
#endif
  if (!afGetFrameCount) {
    return LIBRARY_SYMBOLS_MISSING;
  }

#if defined(WIN32)
  afTellFrame = (void*)GetProcAddress(handle, "afTellFrame");
#else
  afTellFrame = dlsym(handle, "afTellFrame");
#endif
  if (!afTellFrame) {
    return LIBRARY_SYMBOLS_MISSING;
  }

#if defined(WIN32)
  afInitFileFormat = (void*)GetProcAddress(handle, "afInitFileFormat");
#else
  afInitFileFormat = dlsym(handle, "afInitFileFormat");
#endif
  if (!afInitFileFormat) {
    return LIBRARY_SYMBOLS_MISSING;
  }

#if defined(WIN32)
  afInitSampleFormat = (void*)GetProcAddress(handle, "afInitSampleFormat");
#else
  afInitSampleFormat = dlsym(handle, "afInitSampleFormat");
#endif
  if (!afInitSampleFormat) {
    return LIBRARY_SYMBOLS_MISSING;
  }

#if defined(WIN32)
  afInitChannels = (void*)GetProcAddress(handle, "afInitChannels");
#else
  afInitChannels = dlsym(handle, "afInitChannels");
#endif
  if (!afInitChannels) {
    return LIBRARY_SYMBOLS_MISSING;
  }

#if defined(WIN32)
  afInitRate = (void*)GetProcAddress(handle, "afInitRate");
#else
  afInitRate = dlsym(handle, "afInitRate");
#endif
  if (!afInitRate) {
    return LIBRARY_SYMBOLS_MISSING;
  }

#if defined(WIN32)
  afGetRate = (void*)GetProcAddress(handle, "afGetRate");
#else
  afGetRate = dlsym(handle, "afGetRate");
#endif
  if (!afGetRate) {
    return LIBRARY_SYMBOLS_MISSING;
  }

#if defined(WIN32)
  afNewFileSetup = (void*)GetProcAddress(handle, "afNewFileSetup");
#else
  afNewFileSetup = dlsym(handle, "afNewFileSetup");
#endif
  if (!afNewFileSetup) {
    return LIBRARY_SYMBOLS_MISSING;
  }

#if defined(WIN32)
  afFreeFileSetup = (void*)GetProcAddress(handle, "afFreeFileSetup");
#else
  afFreeFileSetup = dlsym(handle, "afFreeFileSetup");
#endif
  if (!afFreeFileSetup) {
    return LIBRARY_SYMBOLS_MISSING;
  }

  return LIBRARY_OK;
}

static enum library_status portaudio_init(){
  void *handle;

  static const char* portaudio_library_name =
#if (defined _MSC_VER)
    "portaudio_x86.dll"
#elif (defined _WIN32 || defined __CYGWIN__) //not _MSC_VER
#ifdef PORTAUDIO_COMPILED_WITH_CYGWIN_SHELL
    "cygportaudio-2.dll"
#else //not PORTAUDIO_COMPILED_WITH_CYGWIN_SHELL
    "libportaudio-2.dll"
#endif //_WIN32 or __CYGWIN__,AUDIOFILE_COMPILED_WITH_CYGWIN_SHELL
#elif defined __APPLE__
    "libportaudio.2.dylib"
#else //not _MSC_VER, not _WIN32, not __CYGWIN__
    "libportaudio.so.2"
#endif//_MSC_VER,_WIN32 or __CYGWIN__
    ;


#if defined(WIN32)
  handle=LoadLibraryA(portaudio_library_name);
#else
  handle=dlopen(portaudio_library_name, RTLD_LAZY);
#endif
  if (!handle)
    return LIBRARY_MISSING;

#if defined(WIN32)
  Pa_Initialize = (void*)GetProcAddress(handle, "Pa_Initialize");
#else
  Pa_Initialize = dlsym(handle, "Pa_Initialize");
#endif
   if (!Pa_Initialize) {
    return LIBRARY_SYMBOLS_MISSING;
  }

#if defined(WIN32)
  Pa_Terminate = (void*)GetProcAddress(handle, "Pa_Terminate");
#else
  Pa_Terminate = dlsym(handle, "Pa_Terminate");
#endif
   if (!Pa_Terminate) {
    return LIBRARY_SYMBOLS_MISSING;
  }

#if defined(WIN32)
  Pa_OpenDefaultStream = (void*)GetProcAddress(handle, "Pa_OpenDefaultStream");
#else
  Pa_OpenDefaultStream = dlsym(handle, "Pa_OpenDefaultStream");
#endif
   if (!Pa_OpenDefaultStream) {
    return LIBRARY_SYMBOLS_MISSING;
  }

#if defined(WIN32)
   Pa_CloseStream = (void*)GetProcAddress(handle, "Pa_CloseStream");
#else
   Pa_CloseStream = dlsym(handle, "Pa_CloseStream");
#endif
   if (!Pa_CloseStream) {
     return LIBRARY_SYMBOLS_MISSING;
   }

#if defined(WIN32)
   Pa_StartStream = (void*)GetProcAddress(handle, "Pa_StartStream");
#else
   Pa_StartStream = dlsym(handle, "Pa_StartStream");
#endif
   if (!Pa_StartStream) {
     return LIBRARY_SYMBOLS_MISSING;
   }

#if defined(WIN32)
   Pa_StopStream = (void*)GetProcAddress(handle, "Pa_StopStream");
#else
   Pa_StopStream = dlsym(handle, "Pa_StopStream");
#endif
   if (!Pa_StopStream) {
     return LIBRARY_SYMBOLS_MISSING;
   }

#if defined(WIN32)
   Pa_ReadStream = (void*)GetProcAddress(handle, "Pa_ReadStream");
#else
   Pa_ReadStream = dlsym(handle, "Pa_ReadStream");
#endif
   if (!Pa_ReadStream) {
     return LIBRARY_SYMBOLS_MISSING;
   }

#if defined(WIN32)
   Pa_WriteStream = (void*)GetProcAddress(handle, "Pa_WriteStream");
#else
   Pa_WriteStream = dlsym(handle, "Pa_WriteStream");
#endif
   if (!Pa_WriteStream) {
     return LIBRARY_SYMBOLS_MISSING;
   }

  if (Pa_Initialize() != paNoError)
  {
    return LIBRARY_INIT_FAILED;
  }

  return LIBRARY_OK;
}

static enum library_status libtapencoder_init(){
#if defined(WIN32)
  HMODULE
#else
  void *
#endif
  handle;

  static const char* tapencoder_library_name =
#if (defined _WIN32 || defined __CYGWIN__) 
    "tapencoder.dll"
#else
    "libtapencoder.so"
#endif//__WIN32 or __CYGWIN__
    ;


#if defined(WIN32)
  handle=LoadLibraryA(tapencoder_library_name);
#else
  handle=dlopen(tapencoder_library_name, RTLD_LAZY);
#endif
  if (!handle)
    return LIBRARY_MISSING;

  tapenc_init2 = 
#if defined(WIN32)
  (void*)GetProcAddress(handle, "tapenc_init2");
#else
  dlsym(handle, "tapenc_init2");
#endif
  if (!tapenc_init2)
    return LIBRARY_SYMBOLS_MISSING;

  tapenc_exit = 
#if defined(WIN32)
  (void*)GetProcAddress(handle, "tapenc_exit");
#else
  dlsym(handle, "tapenc_exit");
#endif
  if (!tapenc_exit)
    return LIBRARY_SYMBOLS_MISSING;

  tapenc_get_pulse =
#if defined(WIN32)
  (void*)GetProcAddress(handle, "tapenc_get_pulse");
#else
  dlsym(handle, "tapenc_get_pulse");
#endif
  if (!tapenc_get_pulse)
    return LIBRARY_SYMBOLS_MISSING;

  tapenc_flush =
#if defined(WIN32)
  (void*)GetProcAddress(handle, "tapenc_flush");
#else
  dlsym(handle, "tapenc_flush");
#endif
  if (!tapenc_flush)
    return LIBRARY_SYMBOLS_MISSING;

  tapenc_get_max =
#if defined(WIN32)
   (void*)GetProcAddress(handle, "tapenc_get_max");
#else
   dlsym(handle, "tapenc_get_max");
#endif
   if (!tapenc_get_max)
     return LIBRARY_SYMBOLS_MISSING;

  tapenc_invert =
#if defined(WIN32)
   (void*)GetProcAddress(handle, "tapenc_invert");
#else
   dlsym(handle, "tapenc_invert");
#endif
   if (!tapenc_invert)
     return LIBRARY_SYMBOLS_MISSING;

  tapenc_toggle_trigger_on_both_edges =
#if defined(WIN32)
   (void*)GetProcAddress(handle, "tapenc_toggle_trigger_on_both_edges");
#else
   dlsym(handle, "tapenc_toggle_trigger_on_both_edges");
#endif
   if (!tapenc_toggle_trigger_on_both_edges)
     return LIBRARY_SYMBOLS_MISSING;

  return LIBRARY_OK;
}

static enum library_status libtapdecoder_init(){
#if defined(WIN32)
  HMODULE
#else
  void *
#endif
  handle;

  static const char* tapdecoder_library_name =
#if (defined _WIN32 || defined __CYGWIN__) 
    "tapdecoder.dll"
#else
    "libtapdecoder.so"
#endif//__WIN32 or __CYGWIN__
    ;

  handle=
#if defined(WIN32)
  LoadLibraryA(tapdecoder_library_name);
#else
  dlopen(tapdecoder_library_name, RTLD_LAZY);

#endif
  if (!handle)
    return LIBRARY_MISSING;

  tapdec_init2 = 
#if defined(WIN32)
  (void*)GetProcAddress(handle, "tapdec_init2");
#else
  dlsym(handle, "tapdec_init2");
#endif
  if (!tapdec_init2)
    return LIBRARY_SYMBOLS_MISSING;

  tapdec_exit = 
#if defined(WIN32)
  (void*)GetProcAddress(handle, "tapdec_exit");
#else
  dlsym(handle, "tapdec_exit");
#endif
  if (!tapdec_exit)
    return LIBRARY_SYMBOLS_MISSING;

  tapdec_set_pulse =
#if defined(WIN32)
  (void*)GetProcAddress(handle, "tapdec_set_pulse");
#else

  dlsym(handle, "tapdec_set_pulse");
#endif
  if (!tapdec_set_pulse)
    return LIBRARY_SYMBOLS_MISSING;

  tapdec_get_buffer =
#if defined(WIN32)
  (void*)GetProcAddress(handle, "tapdec_get_buffer");
#else
  dlsym(handle, "tapdec_get_buffer");
#endif
  if (!tapdec_get_buffer)
    return LIBRARY_SYMBOLS_MISSING;

  tapdec_enable_halfwaves =
#if defined(WIN32)
  (void*)GetProcAddress(handle, "tapdec_enable_halfwaves");
#else
  dlsym(handle, "tapdec_enable_halfwaves");
#endif
  if (!tapdec_enable_halfwaves)
    return LIBRARY_SYMBOLS_MISSING;

  return LIBRARY_OK;
}

struct audiotap_init_status audiotap_initialize2(void){
  status.audiofile_init_status = audiofile_init();
  status.portaudio_init_status = portaudio_init();
  status.tapencoder_init_status = libtapencoder_init();
  status.tapdecoder_init_status = libtapdecoder_init();

  return status;
}

void audiotap_terminate_lib(void)
{
  if (status.portaudio_init_status == LIBRARY_OK)
    Pa_Terminate();
}

