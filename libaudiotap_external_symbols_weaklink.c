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

#include "audiotap.h"

__attribute__ ((visibility ("hidden")))
struct audiotap_init_status status = {
  LIBRARY_UNINIT,
  LIBRARY_UNINIT,
  LIBRARY_UNINIT,
  LIBRARY_UNINIT
};

extern void* afOpenFile __attribute__((weak));
extern void* afOpenFD __attribute__((weak));
extern void* afCloseFile __attribute__((weak));
extern void* afReadFrames __attribute__((weak));
extern void* afWriteFrames __attribute__((weak));
extern void* afSeekFrame __attribute__((weak));
extern void* afSetVirtualChannels __attribute__((weak));
extern void* afGetSampleFormat __attribute__((weak));
extern void* afSetVirtualSampleFormat __attribute__((weak));
extern void* afGetVirtualFrameSize __attribute__((weak));
extern void* afGetFrameCount __attribute__((weak));
extern void* afTellFrame __attribute__((weak));
extern void* afInitFileFormat __attribute__((weak));
extern void* afInitSampleFormat __attribute__((weak));
extern void* afInitChannels __attribute__((weak));
extern void* afInitRate __attribute__((weak));
extern void* afGetRate __attribute__((weak));
extern void* afNewFileSetup __attribute__((weak));
extern void* afFreeFileSetup __attribute__((weak));
static enum library_status audiofile_init(){
  if(!&afOpenFile
  && !&afOpenFD
  && !&afCloseFile
  && !&afReadFrames
  && !&afWriteFrames
  && !&afSeekFrame
  && !&afSetVirtualChannels
  && !&afGetSampleFormat
  && !&afSetVirtualSampleFormat
  && !&afGetVirtualFrameSize
  && !&afGetFrameCount
  && !&afTellFrame
  && !&afInitFileFormat
  && !&afInitSampleFormat
  && !&afInitChannels
  && !&afInitRate
  && !&afGetRate
  && !&afNewFileSetup
  && !&afFreeFileSetup)
    return LIBRARY_MISSING;

  if(!&afOpenFile
  || !&afOpenFD
  || !&afCloseFile
  || !&afReadFrames
  || !&afWriteFrames
  || !&afSeekFrame
  || !&afSetVirtualChannels
  || !&afGetSampleFormat
  || !&afSetVirtualSampleFormat
  || !&afGetVirtualFrameSize
  || !&afGetFrameCount
  || !&afTellFrame
  || !&afInitFileFormat
  || !&afInitSampleFormat
  || !&afInitChannels
  || !&afInitRate
  || !&afGetRate
  || !&afNewFileSetup
  || !&afFreeFileSetup)
    return LIBRARY_SYMBOLS_MISSING;

  return LIBRARY_OK;
}

extern int Pa_Initialize();
extern int Pa_Terminate();
extern void* Pa_OpenDefaultStream __attribute__((weak));
extern void* Pa_CloseStream __attribute__((weak));
extern void* Pa_StartStream __attribute__((weak));
extern void* Pa_StopStream __attribute__((weak));
extern void* Pa_ReadStream __attribute__((weak));
extern void* Pa_WriteStream __attribute__((weak));
static enum library_status portaudio_init(){
if (!&Pa_Initialize
 && !&Pa_Terminate
 && !&Pa_OpenDefaultStream
 && !&Pa_CloseStream
 && !&Pa_StartStream
 && !&Pa_StopStream
 && !&Pa_ReadStream
 && !&Pa_WriteStream)
    return LIBRARY_MISSING;

if (!&Pa_Initialize
 || !&Pa_Terminate
 || !&Pa_OpenDefaultStream
 || !&Pa_CloseStream
 || !&Pa_StartStream
 || !&Pa_StopStream
 || !&Pa_ReadStream
 || !&Pa_WriteStream)
    return LIBRARY_SYMBOLS_MISSING;

  if (Pa_Initialize() != 0)
  {
    return LIBRARY_INIT_FAILED;
  }

  return LIBRARY_OK;
}

extern void* tapenc_init2 __attribute__((weak));
extern void* tapenc_exit __attribute__((weak));
extern void* tapenc_get_pulse __attribute__((weak));
extern void* tapenc_flush __attribute__((weak));
extern void* tapenc_get_max __attribute__((weak));
extern void* tapenc_invert __attribute__((weak));
extern void* tapenc_toggle_trigger_on_both_edges __attribute__((weak));
extern void* tapenc_set_silence_threshold __attribute__((weak));
static enum library_status libtapencoder_init(){
  if (!&tapenc_init2
   && !&tapenc_exit
   && !&tapenc_get_pulse
   && !&tapenc_flush
   && !&tapenc_get_max
   && !&tapenc_invert
   && !&tapenc_toggle_trigger_on_both_edges
   && !&tapenc_set_silence_threshold)
    return LIBRARY_MISSING;

  if (!&tapenc_init2
   || !&tapenc_exit
   || !&tapenc_get_pulse
   || !&tapenc_flush
   || !&tapenc_get_max
   || !&tapenc_invert
   || !&tapenc_toggle_trigger_on_both_edges
   || !&tapenc_set_silence_threshold)
    return LIBRARY_SYMBOLS_MISSING;

  return LIBRARY_OK;
}

extern void* tapdec_init2 __attribute__((weak));
extern void* tapdec_exit __attribute__((weak));
extern void* tapdec_set_pulse __attribute__((weak));
extern void* tapdec_get_buffer __attribute__((weak));
extern void* tapdec_enable_halfwaves __attribute__((weak));
static enum library_status libtapdecoder_init(){
  if (!&tapdec_init2
   && !&tapdec_exit
   && !&tapdec_set_pulse
   && !&tapdec_get_buffer
   && !&tapdec_enable_halfwaves)
    return LIBRARY_MISSING;

  if (!&tapdec_init2
   || !&tapdec_exit
   || !&tapdec_set_pulse
   || !&tapdec_get_buffer
   || !&tapdec_enable_halfwaves)
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

