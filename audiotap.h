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

#include "tap_types.h"

enum library_status {
  LIBRARY_UNINIT,
  LIBRARY_MISSING,
  LIBRARY_SYMBOLS_MISSING,
  LIBRARY_OK
};

struct audiotap_init_status {
  enum library_status audiofile_init_status;
  enum library_status pablio_init_status;
};

enum audiotap_status{
  AUDIOTAP_OK,
  AUDIOTAP_ERR,
  AUDIOTAP_NO_MEMORY,
  AUDIOTAP_LIBRARY_UNAVAILABLE,
  AUDIOTAP_NO_FILE,
  AUDIOTAP_LIBRARY_ERROR,
  AUDIOTAP_EOF,
  AUDIOTAP_INTERRUPTED
};

struct audiotap; /* hide structure of audiotap from applications */

struct audiotap_init_status audiotap_initialize(void);
enum audiotap_status audio2tap_open(struct audiotap **audiotap,
				    char *file,
					u_int32_t freq,
				    u_int32_t min_duration,
				    u_int32_t min_height,
				    int inverted);

enum audiotap_status audio2tap_get_pulse(struct audiotap *audiotap, u_int32_t *pulse);
int audio2tap_get_total_len(struct audiotap *audiotap);
int audio2tap_get_current_pos(struct audiotap *audiotap);
void audiotap_terminate(struct audiotap *audiotap);
void audio2tap_close(struct audiotap *audiotap);
enum audiotap_status tap2audio_open(struct audiotap **audiotap,
				    char *file,
				    int32_t volume,
				    u_int32_t freq,
				    int inverted,
				    u_int8_t machine,
				    u_int8_t videotype);
enum audiotap_status tap2audio_set_pulse(struct audiotap *audiotap, u_int32_t pulse);
void tap2audio_close(struct audiotap *audiotap);
int32_t audio2tap_get_current_sound_level(struct audiotap *audiotap);
