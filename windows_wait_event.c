#include <windows.h>

struct wait_event {
  HANDLE wait_event;
};

#if __GNUC__ >= 4
 __attribute__ ((visibility ("hidden")))
#endif
int size_of_wait_event(void){
  return sizeof(struct wait_event);
}

#if __GNUC__ >= 4
 __attribute__ ((visibility ("hidden")))
#endif
struct wait_event *create_wait_event(void) {
  return CreateEvent(NULL, TRUE, TRUE, NULL);
}

#if __GNUC__ >= 4
 __attribute__ ((visibility ("hidden")))
#endif
 char should_pause(struct wait_event *wait_event){
  return WaitForSingleObject(wait_event->wait_event, 0) == WAIT_TIMEOUT;
}

#if __GNUC__ >= 4
 __attribute__ ((visibility ("hidden")))
#endif
 void pause_if_necessary(struct wait_event *wait_event){
  WaitForSingleObject(wait_event->wait_event, INFINITE);
}

#if __GNUC__ >= 4
 __attribute__ ((visibility ("hidden")))
#endif
void set_pause(struct wait_event *wait_event) {
  ResetEvent(wait_event->wait_event);
}

#if __GNUC__ >= 4
 __attribute__ ((visibility ("hidden")))
#endif
void resume_from_pause(struct wait_event *wait_event) {
  SetEvent(wait_event->wait_event);
}

#if __GNUC__ >= 4
 __attribute__ ((visibility ("hidden")))
#endif
void destroy_wait_event(struct wait_event *wait_event) {
  CloseHandle(&wait_event->wait_event);
}

