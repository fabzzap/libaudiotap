#include <pthread.h>
#include <stdbool.h>
#include "wait_event.h"

struct wait_event {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  bool triggered;
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
void create_wait_event(struct wait_event *wait_event) {
  pthread_mutex_init(&wait_event->mutex, 0);
  pthread_cond_init(&wait_event->cond, 0);
  wait_event->triggered = true;
}

#if __GNUC__ >= 4
 __attribute__ ((visibility ("hidden")))
#endif
void pause_if_necessary(struct wait_event *wait_event){
  pthread_mutex_lock(&wait_event->mutex);
  while (!wait_event->triggered)
    pthread_cond_wait(&wait_event->cond, &wait_event->mutex);
  pthread_mutex_unlock(&wait_event->mutex);
}

#if __GNUC__ >= 4
 __attribute__ ((visibility ("hidden")))
#endif
void set_pause(struct wait_event *wait_event) {
  pthread_mutex_lock(&wait_event->mutex);
  wait_event->triggered = false;
  pthread_mutex_unlock(&wait_event->mutex);
}

#if __GNUC__ >= 4
 __attribute__ ((visibility ("hidden")))
#endif
void resume_from_pause(struct wait_event *wait_event) {
  pthread_mutex_lock(&wait_event->mutex);
  wait_event->triggered = true;
  pthread_cond_signal(&wait_event->cond);
  pthread_mutex_unlock(&wait_event->mutex);
}

#if __GNUC__ >= 4
 __attribute__ ((visibility ("hidden")))
#endif
void destroy_wait_event(struct wait_event *wait_event) {
  pthread_cond_destroy(&wait_event->cond);
  pthread_mutex_destroy(&wait_event->mutex);
}

