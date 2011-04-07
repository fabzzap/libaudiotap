/* TAP shared library: a library for converting audio data (as a stream of
 * PCM signed 32-bit samples, mono) to raw TAP data and back
 *
 * TAP specification was written by Per Håkan Sundell and is available at
 * http://www.computerbrains.com/tapformat.html
 *
 * The algorithm for TAP encoding was originally written by Janne Veli
 * Kujala, based on the one written by Andreas Matthies for Tape64.
 * Some modifications and adaptation to library format by Fabrizio Gennari
 *
 * Copyright (c) Fabrizio Gennari, 2003-2011
 *
 * The program is distributed under the GNU Lesser General Public License.
 * See file LESSER-LICENSE.TXT for details.
 */

#include <stdint.h>
#include "tap_types.h"

struct tap_enc_t;

struct tap_enc_t *(*tapenc_init)(uint32_t min_duration, uint8_t sensitivity, uint8_t initial_threshold, enum tap_trigger inverted);
uint32_t (*tapenc_get_pulse)(struct tap_enc_t *tap, int32_t *buffer, uint32_t buflen, uint8_t *got_pulse, uint32_t *pulse);
uint32_t (*tapenc_flush)(struct tap_enc_t *tap);
int32_t (*tapenc_get_max)(struct tap_enc_t *tap);

