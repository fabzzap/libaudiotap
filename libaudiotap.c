/* Audiotap shared library: a higher-level interface to TAP shared library
 *
 * TAP->audio: feeds TAP data to TAP library, gets audio data from it and feeds
 * it either to audiofile library (for writing WAV files) or to pablio library
 * (for playing)
 * audio->TAP: gets audio data from audiofile library (for WAV and other audio
 * file formats) or from pablio library (sound card's line in), feeds it to
 * TAP library and gets TAP data from it
 *
 * Audiotap shared library can work without audiofile or without pablio, but
 * it is useless without both.
 *
 * Copyright (c) Fabrizio Gennari, 2003
 *
 * The program is distributed under the GNU Lesser General Public License.
 * See file LESSER-LICENSE.TXT for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifndef _WIN32
#include <dlfcn.h>
#else
#include <windows.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include "audiofile.h"
#include "pablio.h"
#include "tapencoder.h"
#include "tapdecoder.h"
#include "audiotap.h"

struct audio2tap_functions {
  enum audiotap_status(*get_pulse)(struct audiotap *audiotap, uint32_t *pulse, uint32_t *raw_pulse);
  enum audiotap_status(*set_buffer)(void *priv, int32_t *buffer, uint32_t bufsize, uint32_t *numframes);
  int (*get_total_len)(struct audiotap *audiotap);
  int (*get_current_pos)(struct audiotap *audiotap);
  void                (*close)(void *priv);
};

struct tap2audio_functions {
  void                (*set_pulse)(struct audiotap *audiotap, uint32_t pulse);
  uint32_t            (*get_buffer)(struct audiotap *audiotap);
  enum audiotap_status(*dump_buffer)(uint8_t *buffer, uint32_t bufroom, void *priv);
  void                (*close)(void *priv);
};

struct tap_handle {
  FILE *file;
  unsigned char version;
  uint32_t next_pulse;
  uint8_t just_finished_with_overflow;
};

static const char c64_tap_header[] = "C64-TAPE-RAW";
static const char c16_tap_header[] = "C16-TAPE-RAW";

struct audiotap {
  struct tap_enc_t *tapenc;
  struct tap_dec_t *tapdec;
  uint8_t *buffer, bufstart[2048];
  uint32_t bufroom;
  int terminated;
  int has_flushed;
  float factor;
  uint32_t accumulated_samples;
  const struct tap2audio_functions *tap2audio_functions;
  const struct audio2tap_functions *audio2tap_functions;
  void *priv;
};

static struct audiotap_init_status status = {
  LIBRARY_UNINIT,
  LIBRARY_UNINIT
};

static const float tap_clocks[][2]={
  {985248,1022727}, /* C64 */
  {1108405,1022727}, /* VIC */
  {886724,894886}  /* C16 */
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
"cygaudiofile-0.dll"
#else //not AUDIOFILE_COMPILED_WITH_CYGWIN_SHELL
"libaudiofile-0.dll"
#endif //_WIN32 or __CYGWIN__,AUDIOFILE_COMPILED_WITH_CYGWIN_SHELL
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
  };

#if defined(WIN32)
  afOpenFile = (void*)GetProcAddress(handle, "afOpenFile");
#else
  afOpenFile = dlsym(handle, "afOpenFile");
#endif
  if (!afOpenFile) {
    return LIBRARY_SYMBOLS_MISSING;
  };

#if defined(WIN32)
  afCloseFile = (void*)GetProcAddress(handle, "afCloseFile");
#else
  afCloseFile = dlsym(handle, "afCloseFile");
#endif
  if (!afCloseFile) {
    return LIBRARY_SYMBOLS_MISSING;
  };

#if defined(WIN32)
  afReadFrames = (void*)GetProcAddress(handle, "afReadFrames");
#else
  afReadFrames = dlsym(handle, "afReadFrames");
#endif
  if (!afReadFrames) {
    return LIBRARY_SYMBOLS_MISSING;
  };

#if defined(WIN32)
  afWriteFrames = (void*)GetProcAddress(handle, "afWriteFrames");
#else
  afWriteFrames = dlsym(handle, "afWriteFrames");
#endif
  if (!afWriteFrames) {
    return LIBRARY_SYMBOLS_MISSING;
  };

#if defined(WIN32)
  afSetVirtualChannels = (void*)GetProcAddress(handle, "afSetVirtualChannels");
#else
  afSetVirtualChannels = dlsym(handle, "afSetVirtualChannels");
#endif
  if (!afSetVirtualChannels) {
    return LIBRARY_SYMBOLS_MISSING;
  };

#if defined(WIN32)
  afGetSampleFormat = (void*)GetProcAddress(handle, "afGetSampleFormat");
#else
  afGetSampleFormat = dlsym(handle, "afGetSampleFormat");
#endif
  if (!afGetSampleFormat) {
    return LIBRARY_SYMBOLS_MISSING;
  };

#if defined(WIN32)
  afSetVirtualSampleFormat = (void*)GetProcAddress(handle, "afSetVirtualSampleFormat");
#else
  afSetVirtualSampleFormat = dlsym(handle, "afSetVirtualSampleFormat");
#endif
  if (!afSetVirtualSampleFormat) {
    return LIBRARY_SYMBOLS_MISSING;
  };

#if defined(WIN32)
  afGetVirtualFrameSize = (void*)GetProcAddress(handle, "afGetVirtualFrameSize");
#else
  afGetVirtualFrameSize = dlsym(handle, "afGetVirtualFrameSize");
#endif
  if (!afGetVirtualFrameSize) {
    return LIBRARY_SYMBOLS_MISSING;
  };

#if defined(WIN32)
  afGetFrameCount = (void*)GetProcAddress(handle, "afGetFrameCount");
#else
  afGetFrameCount = dlsym(handle, "afGetFrameCount");
#endif
  if (!afGetFrameCount) {
    return LIBRARY_SYMBOLS_MISSING;
  };

#if defined(WIN32)
  afTellFrame = (void*)GetProcAddress(handle, "afTellFrame");
#else
  afTellFrame = dlsym(handle, "afTellFrame");
#endif
  if (!afTellFrame) {
    return LIBRARY_SYMBOLS_MISSING;
  };

#if defined(WIN32)
  afInitFileFormat = (void*)GetProcAddress(handle, "afInitFileFormat");
#else
  afInitFileFormat = dlsym(handle, "afInitFileFormat");
#endif
  if (!afInitFileFormat) {
    return LIBRARY_SYMBOLS_MISSING;
  };

#if defined(WIN32)
  afInitSampleFormat = (void*)GetProcAddress(handle, "afInitSampleFormat");
#else
  afInitSampleFormat = dlsym(handle, "afInitSampleFormat");
#endif
  if (!afInitSampleFormat) {
    return LIBRARY_SYMBOLS_MISSING;
  };

#if defined(WIN32)
  afInitChannels = (void*)GetProcAddress(handle, "afInitChannels");
#else
  afInitChannels = dlsym(handle, "afInitChannels");
#endif
  if (!afInitChannels) {
    return LIBRARY_SYMBOLS_MISSING;
  };

#if defined(WIN32)
  afInitRate = (void*)GetProcAddress(handle, "afInitRate");
#else
  afInitRate = dlsym(handle, "afInitRate");
#endif
  if (!afInitRate) {
    return LIBRARY_SYMBOLS_MISSING;
  };

#if defined(WIN32)
  afGetRate = (void*)GetProcAddress(handle, "afGetRate");
#else
  afGetRate = dlsym(handle, "afGetRate");
#endif
  if (!afGetRate) {
    return LIBRARY_SYMBOLS_MISSING;
  };

#if defined(WIN32)
  afNewFileSetup = (void*)GetProcAddress(handle, "afNewFileSetup");
#else
  afNewFileSetup = dlsym(handle, "afNewFileSetup");
#endif
  if (!afNewFileSetup) {
    return LIBRARY_SYMBOLS_MISSING;
  };

#if defined(WIN32)
  afFreeFileSetup = (void*)GetProcAddress(handle, "afFreeFileSetup");
#else
  afFreeFileSetup = dlsym(handle, "afFreeFileSetup");
#endif
  if (!afFreeFileSetup) {
    return LIBRARY_SYMBOLS_MISSING;
  };

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

static enum library_status tapencoder_init(){
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

  tapenc_init = 
#if defined(WIN32)
  (void*)GetProcAddress(handle, "tapenc_init");
#else
  dlsym(handle, "tapenc_init");
#endif
  if (!tapenc_init)
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

  return LIBRARY_OK;
}

static enum library_status tapdecoder_init(){
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

  tapdec_init = 
#if defined(WIN32)
  (void*)GetProcAddress(handle, "tapdec_init");
#else
  dlsym(handle, "tapdec_init");
#endif
  if (!tapdec_init)
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

  return LIBRARY_OK;
}

struct audiotap_init_status audiotap_initialize(void){
  status.audiofile_init_status = audiofile_init();
  status.portaudio_init_status = portaudio_init();
  status.tapencoder_init_status = tapencoder_init();
  status.tapdecoder_init_status = tapdecoder_init();

  return status;
}

static uint32_t convert_samples(struct audiotap *audiotap, uint32_t raw_samples){
  audiotap->accumulated_samples += raw_samples;
  return (uint32_t)(raw_samples * audiotap->factor);
}

static enum audiotap_status audio2tap_open_common(struct audiotap **audiotap,
                                           uint32_t freq,
                                           struct tapdec_params *tapdec_params,
                                           uint8_t machine,
                                           uint8_t videotype,
                                           const struct audio2tap_functions *audio2tap_functions,
                                           void *priv){
  struct audiotap *obj;
  enum audiotap_status error = AUDIOTAP_WRONG_ARGUMENTS;

  do{
    if (machine > TAP_MACHINE_MAX || videotype > TAP_VIDEOTYPE_MAX)
      break;

    error = AUDIOTAP_NO_MEMORY;
    obj=calloc(1, sizeof(struct audiotap));
    if (obj == NULL)
      break;

    obj->priv = priv;
    obj->audio2tap_functions = audio2tap_functions;

    if (tapdec_params != NULL){
      if (
          (obj->tapenc=tapenc_init(tapdec_params->min_duration,
                                   tapdec_params->sensitivity,
                                   tapdec_params->initial_threshold,
                                   tapdec_params->inverted
                                   )
          )==NULL
         )
        break;
      obj->bufroom = 0;
    }
    obj->factor = tap_clocks[machine][videotype] / freq;
    error = AUDIOTAP_OK;
  }while(0);

  if (error == AUDIOTAP_OK)
    *audiotap = obj;
  else
    audio2tap_close(obj);

  return error;
}

static enum audiotap_status tapfile_get_pulse(struct audiotap *audiotap, uint32_t *pulse, uint32_t *raw_pulse){
  struct tap_handle *handle = (struct tap_handle *)audiotap->priv;
  uint8_t byte, threebytes[3];

  *pulse = 0;

  while(1){
    if (fread(&byte, 1, 1, handle->file) != 1)
      return AUDIOTAP_EOF;
    if (byte != 0){
      *raw_pulse = byte;
      *pulse = byte * 8;
      return AUDIOTAP_OK;
    }
    if (handle->version == 0)
      *pulse += 256 * 8;
    else{
      if (fread(threebytes, 3, 1, handle->file) != 1)
        return AUDIOTAP_EOF;
      return threebytes[0]        +
            (threebytes[1] <<  8) +
            (threebytes[2] << 16);
    }
  }
}

static int tapfile_get_total_len(struct audiotap *audiotap){
  struct tap_handle *handle = (struct tap_handle *)audiotap->priv;
  struct stat stats;

  if (fstat(fileno(handle->file), &stats) == -1)
    return -1;
  return stats.st_size;
}

static int tapfile_get_current_pos(struct audiotap *audiotap){
  struct tap_handle *handle = (struct tap_handle *)audiotap->priv;
  long res;

  if ((res = ftell(handle->file)) == -1)
    return -1;
  return (int)(res - 1);
};

static void tapfile_close(void *priv){
  struct tap_handle *handle = (struct tap_handle *)priv;

  fclose(handle->file);
  free(handle);
}

static const struct audio2tap_functions tapfile_read_functions = {
  tapfile_get_pulse,
  NULL,
  tapfile_get_total_len,
  tapfile_get_current_pos,
  tapfile_close
};

static enum audiotap_status tapfile_init(struct audiotap **audiotap,
                                         char *file){
  struct tap_handle *handle;
  enum audiotap_status err;

  handle = malloc(sizeof(struct tap_handle));
  if (handle == NULL)
    return AUDIOTAP_NO_MEMORY;


  do {
    char file_header[12];
    handle->file = fopen(file, "rb");
    if (handle->file == NULL){
      err = errno == ENOENT ? AUDIOTAP_NO_FILE : AUDIOTAP_LIBRARY_ERROR;
      break;
    }
    err = AUDIOTAP_LIBRARY_ERROR;
    if (fread(file_header, sizeof(file_header), 1, handle->file) < 1)
      break;
    err = AUDIOTAP_WRONG_FILETYPE;
    if (
        memcmp(c64_tap_header, file_header, sizeof(file_header))
     && memcmp(c16_tap_header, file_header, sizeof(file_header))
       )
      break;
    if (fread(&handle->version, 1, 1, handle->file) < 1)
      break;
    if (handle->version > 2)
      break;
    if (fseek(handle->file, 20, SEEK_SET) != 0)
      break;
    err = AUDIOTAP_OK;
  } while (0);
  if (err == AUDIOTAP_OK)
    return audio2tap_open_common(audiotap,
                                 0,
                                 NULL,
                                 0, /*unused*/
                                 0, /*unused*/
                                 &tapfile_read_functions,
                                 handle);
  if (handle->file != NULL)
    fclose(handle->file);
  free(handle);
  return err;
}

static enum audiotap_status audio_get_pulse(struct audiotap *audiotap, uint32_t *pulse, uint32_t *raw_pulse){
  while(1){
    uint8_t got_pulse;
    uint32_t done_now;
    enum audiotap_status error;
    uint32_t numframes;

    if(audiotap->terminated)
      return AUDIOTAP_INTERRUPTED;
    if (audiotap->has_flushed)
      return AUDIOTAP_EOF;

    done_now=tapenc_get_pulse(audiotap->tapenc, (int32_t*)audiotap->buffer, audiotap->bufroom, &got_pulse, raw_pulse);
    audiotap->buffer += done_now * sizeof(int32_t);
    audiotap->bufroom -= done_now;
    if(got_pulse){
      *pulse = convert_samples(audiotap, *raw_pulse);
      return AUDIOTAP_OK;
    }
    
    error = audiotap->audio2tap_functions->set_buffer(audiotap->priv, (int32_t*)audiotap->bufstart, sizeof(audiotap->bufstart) / sizeof(int32_t), &numframes);
    if (error != AUDIOTAP_OK)
      return error;
    if (numframes == 0){
      *raw_pulse = tapenc_flush(audiotap->tapenc);
      *pulse = convert_samples(audiotap, *raw_pulse);
      audiotap->has_flushed=1;
      return AUDIOTAP_OK;
    }
    audiotap->bufroom = numframes;
  }
}

static enum audiotap_status audiofile_set_buffer(void *priv, int32_t *buffer, uint32_t bufsize, uint32_t *numframes) {
  *numframes=afReadFrames((AFfilehandle)priv, AF_DEFAULT_TRACK, buffer, bufsize);
  return *numframes == -1 ? AUDIOTAP_LIBRARY_ERROR : AUDIOTAP_OK;
}

static void audiofile_close(void *priv){
  afCloseFile((AFfilehandle)priv);
}

static int audiofile_get_total_len(struct audiotap *audiotap){
  return (int)(afGetFrameCount((AFfilehandle)audiotap->priv, AF_DEFAULT_TRACK));
}

static int audiofile_get_current_pos(struct audiotap *audiotap){
   return audiotap->accumulated_samples;
}

const struct audio2tap_functions audiofile_read_functions = {
  audio_get_pulse,
  audiofile_set_buffer,
  audiofile_get_total_len,
  audiofile_get_current_pos,
  audiofile_close
};

static enum audiotap_status audiofile_read_init(struct audiotap **audiotap,
                                                 char *file,
                                                 struct tapdec_params *params,
                                                 uint8_t machine,
                                                 uint8_t videotype){
  uint32_t freq;
  enum audiotap_status error = AUDIOTAP_LIBRARY_ERROR;
  AFfilehandle fh;

  if (status.audiofile_init_status != LIBRARY_OK
   || status.tapencoder_init_status != LIBRARY_OK)
    return AUDIOTAP_LIBRARY_UNAVAILABLE;
  fh=afOpenFile(file,"r", NULL);
  if (fh == AF_NULL_FILEHANDLE)
    return AUDIOTAP_LIBRARY_ERROR;
  do{
    if ( (freq=(uint32_t)afGetRate(fh, AF_DEFAULT_TRACK)) == -1)
      break;
    if (afSetVirtualChannels(fh, AF_DEFAULT_TRACK, 1) == -1)
      break;
    if (afSetVirtualSampleFormat(fh, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 32) == -1)
      break;
    if (afGetVirtualFrameSize(fh, AF_DEFAULT_TRACK, 0) != 4)
      break;
    error = AUDIOTAP_OK;
  }while(0);
  if(error != AUDIOTAP_OK){
    afCloseFile(fh);
    return error;
  }
  return audio2tap_open_common(audiotap,
                               freq,
                               params,
                               machine,
                               videotype,
                               &audiofile_read_functions,
                               fh);
}

enum audiotap_status audio2tap_open_from_file(struct audiotap **audiotap,
                                              char *file,
                                              struct tapdec_params *params,
                                              uint8_t machine,
                                              uint8_t videotype){
  enum audiotap_status error;

  if (machine > TAP_MACHINE_MAX || videotype > TAP_VIDEOTYPE_MAX)
    return AUDIOTAP_WRONG_ARGUMENTS;

  error = tapfile_init(audiotap, file);
  if (error == AUDIOTAP_OK)
    return AUDIOTAP_OK;
  if (error != AUDIOTAP_WRONG_FILETYPE)
    return error;
  return audiofile_init(audiotap,
                        file,
                        params,
                        machine,
                        videotype);
}

static enum audiotap_status portaudio_set_buffer(void *priv, int32_t *buffer, uint32_t bufsize, uint32_t *numframes){
  if (Pa_ReadStream((PaStream*)priv, buffer, bufsize) != paNoError)
    return AUDIOTAP_LIBRARY_ERROR;
  *numframes=bufsize;
  return AUDIOTAP_OK;
}

static void portaudio_close(void *priv){
  Pa_StopStream((PaStream*)priv);
  Pa_CloseStream((PaStream*)priv);
}

static int portaudio_get_total_len(struct audiotap *audiotap){
  return -1;
}

static int portaudio_get_current_pos(struct audiotap *audiotap){
  return -1;
}

static const struct audio2tap_functions portaudio_read_functions = {
  audio_get_pulse,
  portaudio_set_buffer,
  portaudio_get_total_len,
  portaudio_get_current_pos,
  portaudio_close
};

enum audiotap_status audio2tap_from_soundcard(struct audiotap **audiotap,
                                              uint32_t freq,
                                              struct tapdec_params *params,
                                              uint8_t machine,
                                              uint8_t videotype){
  enum audiotap_status error=AUDIOTAP_LIBRARY_ERROR;
  PaStream *pastream;

  if (status.portaudio_init_status != LIBRARY_OK
   || status.tapencoder_init_status != LIBRARY_OK)
    return AUDIOTAP_LIBRARY_UNAVAILABLE;
  if (Pa_OpenDefaultStream(&pastream, 1, 0, paInt32, freq, sizeof((*audiotap)->bufstart) / sizeof(int32_t), NULL, NULL) != paNoError)
    return AUDIOTAP_LIBRARY_ERROR;
  if (Pa_StartStream(pastream) != paNoError){
    Pa_CloseStream(pastream);
    return AUDIOTAP_LIBRARY_ERROR;
  }
  return audio2tap_open_common(audiotap,
                               freq,
                               params,
                               machine,
                               videotype,
                               &portaudio_read_functions,
                               pastream);
}

enum audiotap_status audio2tap_get_pulses(struct audiotap *audiotap, uint32_t *pulse, uint32_t *raw_pulse){
  return audiotap->audio2tap_functions->get_pulse(audiotap, pulse, raw_pulse);
}

int audio2tap_get_total_len(struct audiotap *audiotap){
  return audiotap->audio2tap_functions->get_total_len(audiotap);
}

int audio2tap_get_current_pos(struct audiotap *audiotap){
  return audiotap->audio2tap_functions->get_current_pos(audiotap);
}

int32_t audio2tap_get_current_sound_level(struct audiotap *audiotap){
  if (!audiotap->tapenc)
    return -1;
  return tapenc_get_max(audiotap->tapenc);
}

void audiotap_terminate(struct audiotap *audiotap){
  audiotap->terminated = 1;
}

void audio2tap_close(struct audiotap *audiotap){
  if (audiotap){
    audiotap->audio2tap_functions->close(audiotap->priv);
    free(audiotap->tapenc);
  }
  free(audiotap);
}

/* ----------------- TAP2AUDIO ----------------- */

static void tapfile_set_pulse(struct audiotap *audiotap, uint32_t pulse){
  struct tap_handle *handle = (struct tap_handle *)audiotap->priv;
  handle->next_pulse = pulse;
}

static uint32_t tapfile_get_buffer(struct audiotap *audiotap){
  struct tap_handle *handle = (struct tap_handle *)audiotap->priv;
  uint32_t max_in_one_byte = 0x800;
  uint32_t overflow_value = handle->version == 0 ? max_in_one_byte : 0xFFFFFF;
  uint8_t *buffer = audiotap->bufstart;
  uint32_t bufroom = sizeof(audiotap->bufstart);
  uint8_t done = 0;

  while(handle->next_pulse >= overflow_value){
    if(handle->version == 0){
      if(bufroom){
        *buffer++ = 0;
        bufroom--;
        done = 1;
      }
    }
    else if(bufroom>=4){
      *buffer++ = 0;
      bufroom--;
      *buffer++ = 0xFF;
      bufroom--;
      *buffer++ = 0xFF;
      bufroom--;
      *buffer++ = 0xFF;
      bufroom--;
      done = 1;
      handle->just_finished_with_overflow = 1;
    }
    if(done)
      handle->next_pulse -= overflow_value;
  }

  done = 0;
  if(
     (
       handle->next_pulse >= max_in_one_byte || 
        (
         handle->next_pulse == 0
      && handle->version > 0
      && handle->just_finished_with_overflow
        )
     )
     && bufroom >=4
    ){
    *buffer++ = 0;
    bufroom--;
    *buffer++ = (handle->next_pulse      )&0xFF;
    bufroom--;
    *buffer++ = (handle->next_pulse >>  8)&0xFF;
    bufroom--;
    *buffer++ = (handle->next_pulse >> 16)&0xFF;
    bufroom--;
    done = 1;
  }
  else if (handle->next_pulse > 0 && bufroom > 0){
    handle->next_pulse += 7;
    if (handle->next_pulse > 0x7F8)
      handle->next_pulse = 0x7F8;
    *buffer++ = (handle->next_pulse + 7) / 8;
    bufroom--;
    done = 1;
  }
  if(done){
    handle->just_finished_with_overflow = 0;
    handle->next_pulse = 0;
  }
  return sizeof(audiotap->bufstart) - bufroom;
}

static enum audiotap_status tapfile_dump_buffer(uint8_t *buffer, uint32_t bufsize, void *priv){
  return fwrite(buffer, bufsize, 1, ((struct tap_handle *)priv)->file) == 1
   ? AUDIOTAP_OK
   : AUDIOTAP_LIBRARY_ERROR;
}

static void tapfile_write_close(void *file){
  struct tap_handle *handle = file;
  long size;
  unsigned char size_header[4];

  do{
    if ((fseek(handle->file, 0, SEEK_END)) != 0)
      break;
    if ((size = ftell(handle->file)) == -1)
      break;
    size -= 20;
    if (size < 0)
      break;
    size_header[0] = (unsigned char) (size & 0xFF);
    size_header[1] = (unsigned char) ((size >> 8) & 0xFF);
    size_header[2] = (unsigned char) ((size >> 16) & 0xFF);
    size_header[3] = (unsigned char) ((size >> 24) & 0xFF);
    if ((fseek(handle->file, 16, SEEK_SET)) != 0)
      break;
    fwrite(size_header, 4, 1, handle->file);
  }while(0);
  fclose(handle->file);
  free(handle);
};

static const struct tap2audio_functions tapfile_write_functions = {
  tapfile_set_pulse,
  tapfile_get_buffer,
  tapfile_dump_buffer,
  tapfile_write_close,
};

static void audio_set_pulse(struct audiotap *audiotap, uint32_t pulse){
  tapdec_set_pulse(audiotap->tapdec, (uint32_t)(pulse / audiotap->factor));
}

static uint32_t audio_get_buffer(struct audiotap *audiotap){
  return tapdec_get_buffer(audiotap->tapdec, (int32_t*)audiotap->bufstart, (uint32_t)(sizeof(audiotap->bufstart) / sizeof(uint32_t)) );
}

static enum audiotap_status audiofile_dump_buffer(uint8_t *buffer, uint32_t bufsize, void *priv){
  return (afWriteFrames((AFfilehandle)priv, AF_DEFAULT_TRACK, buffer, (int)(bufsize/4)) == bufsize/4) ? AUDIOTAP_OK : AUDIOTAP_LIBRARY_ERROR;
}

static const struct tap2audio_functions audiofile_write_functions = {
  audio_set_pulse,
  audio_get_buffer,
  audiofile_dump_buffer,
  audiofile_close,
};

static enum audiotap_status portaudio_dump_buffer(uint8_t *buffer, uint32_t bufsize, void *priv){
  return Pa_WriteStream((PaStream*)priv, buffer, bufsize/4) == paNoError ? AUDIOTAP_OK : AUDIOTAP_LIBRARY_ERROR;
}

static const struct tap2audio_functions portaudio_write_functions = {
  audio_set_pulse,
  audio_get_buffer,
  portaudio_dump_buffer,
  portaudio_close,
};

static enum audiotap_status tap2audio_open_common(struct audiotap **audiotap
                                                 ,uint32_t volume
                                                 ,enum tap_trigger inverted
                                                 ,enum tapdec_waveform waveform
                                                 ,uint32_t freq
                                                 ,uint8_t machine
                                                 ,uint8_t videotype
                                                 ,const struct tap2audio_functions *functions
                                                 ,void *priv){
  struct audiotap *obj;
  enum audiotap_status error = AUDIOTAP_NO_MEMORY;

  obj=calloc(1, sizeof(struct audiotap));
  if (obj != NULL){
    obj->factor = tap_clocks[machine][videotype] / freq;
    obj->priv = priv;
    obj->tap2audio_functions = functions;
    if (freq == 0 ||
         ( (obj->tapdec = tapdec_init(volume, inverted, waveform)) != NULL)
       )
      error = AUDIOTAP_OK;
  }

  if (error == AUDIOTAP_OK)
    *audiotap = obj;
  else {
    functions->close(priv);
    free(obj);
    *audiotap = NULL;
  }

  return error;
}

enum audiotap_status tap2audio_open_to_soundcard(struct audiotap **audiotap
                                                 ,uint32_t volume
                                                 ,uint32_t freq
                                                 ,enum tap_trigger inverted
                                                 ,enum tapdec_waveform waveform
                                                 ,uint8_t machine
                                                 ,uint8_t videotype){
  PaStream *pastream;

  if (status.portaudio_init_status != LIBRARY_OK
   || status.tapdecoder_init_status != LIBRARY_OK)
    return AUDIOTAP_LIBRARY_UNAVAILABLE;
  if (Pa_OpenDefaultStream(&pastream, 0, 1, paInt32, freq, sizeof(((struct audiotap*)NULL)->bufstart), NULL, NULL) != paNoError)
    return AUDIOTAP_LIBRARY_ERROR;
  if (Pa_StartStream(pastream) != paNoError){
    Pa_CloseStream(pastream);
    return AUDIOTAP_LIBRARY_ERROR;
  }

  return tap2audio_open_common(audiotap
                              ,volume
                              ,inverted
                              ,waveform
                              ,freq
                              ,machine
                              ,videotype
                              ,&portaudio_write_functions
                              ,pastream);
}

enum audiotap_status tap2audio_open_to_wavfile(struct audiotap **audiotap
                                                 ,char *file
                                                 ,uint32_t volume
                                                 ,uint32_t freq
                                                 ,enum tap_trigger inverted
                                                 ,enum tapdec_waveform waveform
                                                 ,uint8_t machine
                                                 ,uint8_t videotype){
  AFfilehandle fh;
  AFfilesetup setup;

  if (status.audiofile_init_status != LIBRARY_OK
   || status.tapdecoder_init_status != LIBRARY_OK)
    return AUDIOTAP_LIBRARY_UNAVAILABLE;
  setup=afNewFileSetup();
  if (setup == AF_NULL_FILESETUP)
    return AUDIOTAP_NO_MEMORY;
  afInitRate(setup, AF_DEFAULT_TRACK, freq);
  afInitChannels(setup, AF_DEFAULT_TRACK, 1);
  afInitFileFormat(setup, AF_FILE_WAVE);
  afInitSampleFormat(setup, AF_DEFAULT_TRACK, AF_SAMPFMT_UNSIGNED, 8);
  fh=afOpenFile(file,"w", setup);
  afFreeFileSetup(setup);
  if (fh == AF_NULL_FILEHANDLE)
    return AUDIOTAP_LIBRARY_ERROR;
  if (afSetVirtualSampleFormat(fh, AF_DEFAULT_TRACK, AF_SAMPFMT_TWOSCOMP, 32) == -1
   || afGetVirtualFrameSize(fh, AF_DEFAULT_TRACK, 0) != 4){
    afCloseFile(fh);
    return AUDIOTAP_LIBRARY_ERROR;
  }

  return tap2audio_open_common(audiotap
                              ,volume
                              ,inverted
                              ,waveform
                              ,freq
                              ,machine
                              ,videotype
                              ,&audiofile_write_functions
                              ,fh);
}

enum audiotap_status tap2audio_open_to_tapfile(struct audiotap **audiotap
                                                 ,char *name
                                                 ,uint8_t version
                                                 ,uint8_t machine
                                                 ,uint8_t videotype){
  struct tap_handle *handle;
  const char *tap_header = (machine == TAP_MACHINE_C16 ? c16_tap_header : c64_tap_header);
  enum audiotap_status error = AUDIOTAP_LIBRARY_ERROR;

  if (version > TAP_MACHINE_MAX || videotype > TAP_VIDEOTYPE_MAX)
    return AUDIOTAP_WRONG_ARGUMENTS;
  if((handle = malloc(sizeof(struct tap_handle))) == NULL)
    return AUDIOTAP_NO_MEMORY;

  handle->file = fopen(name, "wb");
  if (handle->file == NULL) {
    free(handle);
    return AUDIOTAP_NO_FILE;
  }

  handle->version = version;
  handle->next_pulse = 0;
  handle->just_finished_with_overflow = 0;

  do{
    if (fwrite(tap_header, strlen(tap_header), 1, handle->file) != 1)
      break;
    if (fwrite(&machine, 1, 1, handle->file) != 1)
      break;
    if (fwrite(&videotype, 1, 1, handle->file) != 1)
      break;
    if (fseek(handle->file, 20, SEEK_SET) != 0)
      break;
    error = AUDIOTAP_OK;
  }while(0);

  if(error != AUDIOTAP_OK){
    fclose(handle->file);
    free(handle);
    return error;
  }
  return tap2audio_open_common(audiotap
                              ,0 /*unused*/
                              ,TAP_TRIGGER_ON_BOTH_EDGES /*unused*/
                              ,TAPDEC_SQUARE /*unused*/
                              ,0
                              ,machine
                              ,videotype
                              ,&tapfile_write_functions
                              ,handle);
}

enum audiotap_status tap2audio_set_pulse(struct audiotap *audiotap, uint32_t pulse){
  uint32_t numframes;
  enum audiotap_status error = AUDIOTAP_OK;

  audiotap->tap2audio_functions->set_pulse(audiotap, pulse);

  while(error == AUDIOTAP_OK && (numframes = audiotap->tap2audio_functions->get_buffer(audiotap)) > 0){
  error = audiotap->terminated ? AUDIOTAP_INTERRUPTED :
    audiotap->tap2audio_functions->dump_buffer(audiotap->bufstart, numframes, audiotap->priv);
  }

  return error;
}

void tap2audio_close(struct audiotap *audiotap){
  audiotap->tap2audio_functions->close(audiotap->priv);
  free(audiotap->tapdec);
  free(audiotap);
}

