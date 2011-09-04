/* TAP shared library: a library for converting audio data (as a stream of
 * PCM signed 32-bit samples, mono) to raw TAP data and back
 *
 * TAP specification was written by Per Håkan Sundell and is available at
 * http://www.computerbrains.com/tapformat.html
 *
 * The algorithm to decode TAP data into square, sine or triangular waves
 * is by Fabrizio Gennari.
 *
 * Copyright (c) Fabrizio Gennari, 2003-2011
 *
 * The program is distributed under the GNU Lesser General Public License.
 * See file LESSER-LICENSE.TXT for details.
 */

#include <stdint.h>
#include "tap_types.h"

struct tap_dec_t;

struct tap_dec_t *(*tapdecoder_init)(uint32_t volume, uint8_t inverted, uint8_t semiwaves, enum tapdec_waveform waveform);
void (*tapdec_set_pulse)(struct tap_dec_t *tap, uint32_t pulse);
uint32_t (*tapdec_get_buffer)(struct tap_dec_t *tap, int32_t *buffer, unsigned int buflen);

