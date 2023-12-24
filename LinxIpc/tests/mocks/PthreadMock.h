#include <pthread.h>
#include "CMock2.h"

class PthreadMock : public CMockMocker<PthreadMock> {
  public:
    MOCK_METHOD(int, pthread_mutex_init, (pthread_mutex_t * mutex, const pthread_mutexattr_t *attr));
    MOCK_METHOD(int, pthread_mutex_destroy, (pthread_mutex_t * mutex));
    MOCK_METHOD(int, pthread_mutex_lock, (pthread_mutex_t * mutex));
    MOCK_METHOD(int, pthread_mutex_trylock, (pthread_mutex_t * mutex));
    MOCK_METHOD(int, pthread_mutex_unlock, (pthread_mutex_t * mutex));

    MOCK_METHOD(int, pthread_condattr_init, (pthread_condattr_t * attr));
    MOCK_METHOD(int, pthread_condattr_destroy, (pthread_condattr_t * attr));

    MOCK_METHOD(int, pthread_cond_init, (pthread_cond_t * cond, const pthread_condattr_t *attr));
    MOCK_METHOD(int, pthread_cond_destroy, (pthread_cond_t * cond));
    MOCK_METHOD(int, pthread_cond_timedwait,
                (pthread_cond_t * cond, pthread_mutex_t *mutex, const struct timespec *abstime));
    MOCK_METHOD(int, pthread_cond_wait, (pthread_cond_t * cond, pthread_mutex_t *mutex));
    MOCK_METHOD(int, pthread_cond_signal, (pthread_cond_t * cond));
    MOCK_METHOD(int, pthread_cond_broadcast, (pthread_cond_t * cond));
};
