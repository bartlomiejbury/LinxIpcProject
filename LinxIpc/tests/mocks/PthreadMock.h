#include <pthread.h>
#include "CMock2.h"

class PthreadMock : public CMockMocker<PthreadMock> {
  public:
    MOCK_METHOD(int, pthread_mutex_init, (pthread_mutex_t * mutex, const pthread_mutexattr_t *attr), (const));
    MOCK_METHOD(int, pthread_mutex_destroy, (pthread_mutex_t * mutex), (const));
    MOCK_METHOD(int, pthread_mutex_lock, (pthread_mutex_t * mutex), (const));
    MOCK_METHOD(int, pthread_mutex_trylock, (pthread_mutex_t * mutex), (const));
    MOCK_METHOD(int, pthread_mutex_unlock, (pthread_mutex_t * mutex), (const));

    MOCK_METHOD(int, pthread_condattr_init, (pthread_condattr_t * attr), (const));
    MOCK_METHOD(int, pthread_condattr_destroy, (pthread_condattr_t * attr), (const));

    MOCK_METHOD(int, pthread_cond_init, (pthread_cond_t * cond, const pthread_condattr_t *attr), (const));
    MOCK_METHOD(int, pthread_cond_destroy, (pthread_cond_t * cond), (const));
    MOCK_METHOD(int, pthread_cond_timedwait,
                (pthread_cond_t * cond, pthread_mutex_t *mutex, const struct timespec *abstime), (const));
    MOCK_METHOD(int, pthread_cond_wait, (pthread_cond_t * cond, pthread_mutex_t *mutex), (const));
    MOCK_METHOD(int, pthread_cond_signal, (pthread_cond_t * cond), (const));
    MOCK_METHOD(int, pthread_cond_broadcast, (pthread_cond_t * cond), (const));

    MOCK_METHOD(int, pthread_attr_init, (pthread_attr_t *attr), (const));
    MOCK_METHOD(int, pthread_attr_destroy, (pthread_attr_t *attr), (const));

    MOCK_METHOD(int, pthread_attr_setinheritsched, (pthread_attr_t *attr, int inheritsched), (const));
    MOCK_METHOD(int, pthread_attr_setschedpolicy, (pthread_attr_t *attr, int policy), (const));
    MOCK_METHOD(int, pthread_attr_setschedparam, (pthread_attr_t * attr, const struct sched_param * param), (const));

    MOCK_METHOD(int, pthread_create, (pthread_t * thread, const pthread_attr_t * attr, void *(*start_routine)(void *), void * arg), (const));
    MOCK_METHOD(int, pthread_cancel, (pthread_t thread), (const));
    MOCK_METHOD(int, pthread_join, (pthread_t thread, void **retval), (const));
};
