/***
 * p2pvc
 ***/
/*
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <p2plib.h>

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#define PA_USE_ALSA 1
#include <portaudio.h>
#include "pa_ringbuffer.h"
#include "pa_util.h"

#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (512)
#define NUM_CHANNELS    (2)
#define NUM_WRITES_PER_BUFFER   (4)

#define SIZE_TO_SEND (1024)
#define MIN_SIZE_TO_SEND (512)

#define MIN(x, y) ((x) > (y) ? (y) : (x))
/* Select sample format. */
#if 0
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
#elif 0
#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt8
typedef char SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#else
#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#define SAMPLE_SILENCE  (128)
#define PRINTF_S_FORMAT "%d"
#endif

typedef struct {
  SAMPLE             *inputRingBufferData;
  PaUtilRingBuffer    inputRingBuffer;
  SAMPLE             *outputRingBufferData;
  PaUtilRingBuffer    outputRingBuffer;
} paTestData;

static paTestData data;

/* Connection structure. */
static connection_t *cons;
static size_t conslen;
static pthread_mutex_t conslock;

/* This routine will be called by the PortAudio engine when audio is needed.
 ** It may be called at interrupt level on some machines so don't do anything
 ** that could mess up the system like calling malloc() or free().
 */
static int readCallback(const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData) {
  paTestData *data = (paTestData*)userData;
  ring_buffer_size_t elementsWriteable = PaUtil_GetRingBufferWriteAvailable(&data->inputRingBuffer);
  ring_buffer_size_t elementsToWrite = MIN(elementsWriteable, (ring_buffer_size_t)(framesPerBuffer * NUM_CHANNELS));
  const SAMPLE *rptr = (const SAMPLE*)inputBuffer;

  (void) outputBuffer; /* Prevent unused variable warnings. */
  (void) timeInfo;
  (void) statusFlags;
  (void) userData;

  PaUtil_WriteRingBuffer(&data->inputRingBuffer, rptr, elementsToWrite);

  return paContinue;
}

/* This routine will be called by the PortAudio engine when audio is needed.
 ** It may be called at interrupt level on some machines so don't do anything
 ** that could mess up the system like calling malloc() or free().
 */
static int writeCallback(const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData) {
  paTestData *data = (paTestData*)userData;
  ring_buffer_size_t elementsToPlay = PaUtil_GetRingBufferReadAvailable(&data->outputRingBuffer);
  ring_buffer_size_t elementsToRead = MIN(elementsToPlay, (ring_buffer_size_t)(framesPerBuffer * NUM_CHANNELS));
  SAMPLE* wptr = (SAMPLE*)outputBuffer;

  memset(wptr, SAMPLE_SILENCE, framesPerBuffer * NUM_CHANNELS * sizeof(SAMPLE));

  (void) inputBuffer; /* Prevent unused variable warnings. */
  (void) timeInfo;
  (void) statusFlags;
  (void) userData;

  PaUtil_ReadRingBuffer(&data->outputRingBuffer, wptr, elementsToRead);

  return paContinue;
}

static void callback(connection_t *con, void *buf, size_t length) {
  ring_buffer_size_t elementsWriteable = PaUtil_GetRingBufferWriteAvailable(&data.outputRingBuffer);
  ring_buffer_size_t elementsToWrite = MIN(elementsWriteable, length / data.outputRingBuffer.elementSizeBytes);
  const SAMPLE *rptr = (const SAMPLE*)buf;

  PaUtil_WriteRingBuffer(&data.outputRingBuffer, rptr, elementsToWrite);
}

static void new_callback(connection_t *con, void *data, size_t datalen) {
  pthread_mutex_lock(&conslock);
  conslen++;
  cons = realloc(cons, conslen * sizeof(connection_t));
  memcpy(&(cons[conslen-1]), con, sizeof(connection_t));
  pthread_mutex_unlock(&conslock);
}

static void *dolisten(void *args) {
  int socket;
  int port = atoi((char *)args);
  p2p_init(port, &socket);
  p2p_listener((connection_t **)&cons, &conslen, &conslock, &callback, &new_callback, socket, 4096);
  return NULL;
}

static unsigned NextPowerOf2(unsigned val) {
  val--;
  val = (val >> 1) | val;
  val = (val >> 2) | val;
  val = (val >> 4) | val;
  val = (val >> 8) | val;
  val = (val >> 16) | val;
  return ++val;
}

int start_audio(char *peer, char *port) {
  PaStreamParameters  inputParameters,
                      outputParameters;
  PaStream *inputStream;
  PaStream *outputStream;
  PaError             err = paNoError;
  unsigned            numSamples;
  unsigned            numBytes;

  /* Make a ring buffer that will buffer about half a second.  Reasonable
   * level of latency before droping.
   * Make one for both input and output.*/
  numSamples = NextPowerOf2((unsigned)(SAMPLE_RATE * 0.5 * NUM_CHANNELS));
  numBytes = numSamples * sizeof(SAMPLE);
  data.inputRingBufferData = (SAMPLE *) PaUtil_AllocateMemory(numBytes);
  if (data.inputRingBufferData == NULL)
  {
    fprintf(stderr, "Could not allocate ring buffer data.\n");
    goto done;
  }

  if (PaUtil_InitializeRingBuffer(&data.inputRingBuffer, sizeof(SAMPLE), numSamples, data.inputRingBufferData) < 0)
  {
    fprintf(stderr, "Failed to initialize ring buffer. Size is not power of 2 ?\n");
    goto done;
  }

  data.outputRingBufferData = (SAMPLE *) PaUtil_AllocateMemory(numBytes);
  if (data.outputRingBufferData == NULL)
  {
    fprintf(stderr, "Could not allocate ring buffer data.\n");
    goto done;
  }

  if (PaUtil_InitializeRingBuffer(&data.outputRingBuffer, sizeof(SAMPLE), numSamples, data.outputRingBufferData) < 0)
  {
    fprintf(stderr, "Failed to initialize ring buffer. Size is not power of 2 ?\n");
    goto done;
  }

  err = Pa_Initialize();
  if (err != paNoError) goto done;

  /* Set up output stream. */
  outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
  if (outputParameters.device == paNoDevice) {
    fprintf(stderr,"Error: No default output device.\n");
    goto done;
  }
  outputParameters.channelCount = NUM_CHANNELS;
  outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
  outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
  outputParameters.hostApiSpecificStreamInfo = NULL;

  err = Pa_OpenStream(
      &outputStream,
      NULL, /* no input */
      &outputParameters,
      SAMPLE_RATE,
      FRAMES_PER_BUFFER,
      paClipOff,
      writeCallback,
      &data);
  if (err != paNoError) goto done;

  /* Set up input stream. */
  
  inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
  if (inputParameters.device == paNoDevice) {
    fprintf(stderr,"Error: No default input device.\n");
    goto done;
  }
  inputParameters.channelCount = NUM_CHANNELS;
  inputParameters.sampleFormat = PA_SAMPLE_TYPE;
  inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
  inputParameters.hostApiSpecificStreamInfo = NULL;

  err = Pa_OpenStream(
      &inputStream,
      &inputParameters,
      NULL,
      SAMPLE_RATE,
      FRAMES_PER_BUFFER,
      paClipOff,
      readCallback,
      &data);
  if (err != paNoError) goto done;

  /* Create connection to remote client. */
  pthread_mutex_init(&conslock, NULL);

  cons = calloc(1, sizeof(connection_t));
  if (p2p_connect(peer, port, &(cons[0]))) {
    fprintf(stderr, "Unable to connect to server.\n");
  } else {
    conslen++;
  }

  pthread_t thr;
  pthread_create(&thr, NULL, &dolisten, port);

  /* Start the streams. */
  err = Pa_StartStream(inputStream);
  if (err != paNoError) goto done;

  err = Pa_StartStream(outputStream);
  if (err != paNoError) goto done;

  while (1) {
    /* Try to send out data that has been read in. */
    ring_buffer_size_t elementsInBuffer = PaUtil_GetRingBufferReadAvailable(&data.inputRingBuffer);
    while (elementsInBuffer * data.inputRingBuffer.elementSizeBytes > MIN_SIZE_TO_SEND) {
      uint8_t buf[SIZE_TO_SEND];

      ring_buffer_size_t elements_read =
        PaUtil_ReadRingBuffer(&data.inputRingBuffer, buf,
            SIZE_TO_SEND / data.inputRingBuffer.elementSizeBytes);

      p2p_broadcast(&cons, &conslen, &conslock, buf,
          elements_read * data.inputRingBuffer.elementSizeBytes); 

      elementsInBuffer = PaUtil_GetRingBufferReadAvailable(&data.inputRingBuffer);
    }
    Pa_Sleep(20);
  }

  /* Stop the streams. */
  err = Pa_CloseStream(inputStream);
  if (err != paNoError) goto done;

  err = Pa_CloseStream(outputStream);
  if (err != paNoError) goto done;
  pthread_mutex_destroy(&conslock);
done:
  Pa_Terminate();
  if (data.inputRingBufferData) PaUtil_FreeMemory(data.inputRingBufferData);
  if (data.outputRingBufferData) PaUtil_FreeMemory(data.outputRingBufferData);
  if (err != paNoError) {
    fprintf(stderr, "An error occured while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
    err = 1;          /* Always return 0 or 1, but no other return codes. */
  }
  return err;
}

#ifdef AUDIOONLY
int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Must pass client to connect to.\n");
  }
  return start_audio(argv[1], "55555");
}
#endif

