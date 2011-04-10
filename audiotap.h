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
 * Copyright (c) Fabrizio Gennari, 1998-2003
 *
 * The program is distributed under the GNU Lesser General Public License.
 * See file LESSER-LICENSE.TXT for details.
 */

#include <stdint.h>
#include "tap_types.h"

enum library_status {
  LIBRARY_UNINIT,
  LIBRARY_MISSING,
  LIBRARY_SYMBOLS_MISSING,
  LIBRARY_OK,
  LIBRARY_INIT_FAILED
};

struct audiotap_init_status {
  enum library_status audiofile_init_status;
  enum library_status portaudio_init_status;
  enum library_status tapencoder_init_status;
  enum library_status tapdecoder_init_status;
};

enum audiotap_status{
  AUDIOTAP_OK,
  AUDIOTAP_ERR,
  AUDIOTAP_NO_MEMORY,
  AUDIOTAP_LIBRARY_UNAVAILABLE,
  AUDIOTAP_NO_FILE,
  AUDIOTAP_LIBRARY_ERROR,
  AUDIOTAP_EOF,
  AUDIOTAP_INTERRUPTED,
  AUDIOTAP_WRONG_ARGUMENTS,
  AUDIOTAP_WRONG_FILETYPE
};

/* Machine entry used in extended TAP header */
#define TAP_MACHINE_C64 0
#define TAP_MACHINE_VIC 1
#define TAP_MACHINE_C16 2
#define TAP_MACHINE_MAX 2

/* Video standards */
#define TAP_VIDEOTYPE_PAL  0
#define TAP_VIDEOTYPE_NTSC 1
#define TAP_VIDEOTYPE_MAX  1

struct audiotap; /* hide structure of audiotap from applications */

struct audiotap_init_status audiotap_initialize(void);


struct tapdec_params {
  uint32_t min_duration;
  uint8_t sensitivity;
  uint8_t initial_threshold;
  enum tap_trigger inverted;
};

enum audiotap_status audio2tap_open_from_file(struct audiotap **audiotap,
                                              char *file,
                                              struct tapdec_params *params,
                                              uint8_t *machine,
                                              uint8_t *videotype,
                                              uint8_t *semiwaves);

enum audiotap_status audio2tap_from_soundcard(struct audiotap **audiotap,
                                              uint32_t freq,
                                              struct tapdec_params *params,
                                              uint8_t machine,
                                              uint8_t videotype);

enum audiotap_status audio2tap_get_pulses(struct audiotap *audiotap, uint32_t *pulse, uint32_t *raw_pulse);

int audio2tap_get_total_len(struct audiotap *audiotap);

int audio2tap_get_current_pos(struct audiotap *audiotap);

int32_t audio2tap_get_current_sound_level(struct audiotap *audiotap);

void audiotap_terminate(struct audiotap *audiotap);

void audio2tap_close(struct audiotap *audiotap);

/* ----------------- TAP2AUDIO ----------------- */

enum audiotap_status tap2audio_open_to_soundcard(struct audiotap **audiotap
                                                ,uint32_t volume
                                                ,uint32_t freq
                                                ,enum tap_trigger inverted
                                                ,enum tapdec_waveform waveform
                                                ,uint8_t machine
                                                ,uint8_t videotype);

enum audiotap_status tap2audio_open_to_wavfile(struct audiotap **audiotap
                                              ,char *file
                                              ,uint32_t volume
                                              ,uint32_t freq
                                              ,enum tap_trigger inverted
                                              ,enum tapdec_waveform waveform
                                              ,uint8_t machine
                                              ,uint8_t videotype);

enum audiotap_status tap2audio_open_to_tapfile(struct audiotap **audiotap
                                              ,char *name
                                              ,uint8_t version
                                              ,uint8_t machine
                                              ,uint8_t videotype);

enum audiotap_status tap2audio_set_pulse(struct audiotap *audiotap, uint32_t pulse);

void tap2audio_close(struct audiotap *audiotap);
