/* Audiotap shared library: a higher-level interface to TAP shared library
 * 
 * Header file for pablio library, modified by replacing prototypes with
 * pointers to functions, so functions can be loaded with dlsym/GetProcAddress.
 * Also some definitions taken from portaudio.h
 *
 * Original files (C) 1999-2000, Ross Bencina and Phil Burk
 * This file (C) Fabrizio Gennari, 2003
 *
 * The program is distributed under the GNU Lesser General Public License.
 * See file LESSER-LICENSE.TXT for details.
 */

typedef int PaError;
typedef void PortAudioStream;
typedef unsigned long PaSampleFormat;
#define paInt32 ((PaSampleFormat) (1<<2))

typedef enum {
    paNoError = 0,

    paHostError = -10000,
    paInvalidChannelCount,
    paInvalidSampleRate,
    paInvalidDeviceId,
    paInvalidFlag,
    paSampleFormatNotSupported,
    paBadIODeviceCombination,
    paInsufficientMemory,
    paBufferTooBig,
    paBufferTooSmall,
    paNullCallback,
    paBadStreamPtr,
    paTimedOut,
    paInternalError,
    paDeviceUnavailable
} PaErrorNum;

typedef struct
{
    long   bufferSize; /* Number of bytes in FIFO. Power of 2. Set by RingBuffer_Init. */
    long   writeIndex; /* Index of next writable byte. Set by RingBuffer_AdvanceWriteIndex. */
    long   readIndex;  /* Index of next readable byte. Set by RingBuffer_AdvanceReadIndex. */
    long   bigMask;    /* Used for wrapping indices with extra bit to distinguish full/empty. */
    long   smallMask;  /* Used for fitting indices to buffer. */
    char *buffer;
}
RingBuffer;

typedef struct
{
    RingBuffer   inFIFO;
    RingBuffer   outFIFO;
    PortAudioStream *stream;
    int          bytesPerFrame;
    int          samplesPerFrame;
}
PABLIO_Stream;

/* Values for flags for OpenAudioStream(). */
#define PABLIO_READ     (1<<0)
#define PABLIO_WRITE    (1<<1)
#define PABLIO_READ_WRITE    (PABLIO_READ|PABLIO_WRITE)
#define PABLIO_MONO     (1<<2)
#define PABLIO_STEREO   (1<<3)

static PaError (*OpenAudioStream)( PABLIO_Stream **aStreamPtr, double sampleRate,
                         PaSampleFormat format, long flags );

static long (*ReadAudioStream)( PABLIO_Stream *aStream, void *data, long numFrames );

static long (*WriteAudioStream)( PABLIO_Stream *aStream, void *data, long numFrames );
static PaError (*CloseAudioStream)( PABLIO_Stream *aStream );
