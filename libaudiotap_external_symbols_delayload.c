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

#include <windows.h>
#include <delayimp.h>
#include <excpt.h>
#include "audiotap.h"

#ifdef _MSC_VER
static int CheckDelayException(int exception_value, enum library_status* status)
{
  if (exception_value == VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND))
    *status = LIBRARY_MISSING;
  else if (exception_value == VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND))
    *status = LIBRARY_SYMBOLS_MISSING;
  return EXCEPTION_CONTINUE_EXECUTION;
}

static enum library_status TryDelayLoadAllImports(LPCSTR szDll)
{
  enum library_status retval = LIBRARY_OK;
  __try
  {
    HRESULT hr = __HrLoadAllImportsForDll(szDll);
    if (FAILED(hr))
    {
        return LIBRARY_MISSING;
    }
  }
  __except(CheckDelayException(GetExceptionCode(), &retval))
  {
  }
  return retval;
}
#else

static enum library_status retval_by_exception;

long CALLBACK
ehandler(EXCEPTION_POINTERS *pointers)
{
  switch (pointers->ExceptionRecord->ExceptionCode) {
  case VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND):
    retval_by_exception = LIBRARY_MISSING;
    break;
  case VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND):
    retval_by_exception = LIBRARY_SYMBOLS_MISSING;
    break;
  default:
    retval_by_exception = LIBRARY_INIT_FAILED;
  }
  return EXCEPTION_CONTINUE_EXECUTION;
}

static enum library_status TryDelayLoadAllImports(LPCSTR szDll)
{
  retval_by_exception = LIBRARY_OK;
  __try1(ehandler)
  {
    HRESULT hr = __HrLoadAllImportsForDll(szDll);
    if (FAILED(hr))
    {
      return LIBRARY_MISSING;
    }
  }
  __except1
  {
  }
  return retval_by_exception;
}
#endif

struct audiotap_init_status status = {
  LIBRARY_UNINIT,
  LIBRARY_UNINIT,
  LIBRARY_UNINIT,
  LIBRARY_UNINIT
};

static enum library_status audiofile_init(){
  return TryDelayLoadAllImports("libaudiofile-1.dll");
}

extern int Pa_Initialize();
extern int Pa_Terminate();

static enum library_status portaudio_init(){
  enum library_status import_result = TryDelayLoadAllImports("libportaudio-2.dll");
  if (import_result != LIBRARY_OK)
    return import_result;
  if (Pa_Initialize() != 0)
  {
    return LIBRARY_INIT_FAILED;
  }

  return LIBRARY_OK;
}

static enum library_status libtapencoder_init(){
  return TryDelayLoadAllImports("tapencoder.dll");
}

static enum library_status libtapdecoder_init(){
  return TryDelayLoadAllImports("tapdecoder.dll");
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

