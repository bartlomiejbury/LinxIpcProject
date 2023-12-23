#include "PthreadMock.h"

CMOCK_MOCK_FUNCTION(PthreadMock, int, pthread_mutex_init, (pthread_mutex_t *mutex, const pthread_mutexattr_t *attr));
CMOCK_MOCK_FUNCTION(PthreadMock, int, pthread_mutex_destroy, (pthread_mutex_t *mutex));
CMOCK_MOCK_FUNCTION(PthreadMock, int, pthread_mutex_lock, (pthread_mutex_t *mutex));
CMOCK_MOCK_FUNCTION(PthreadMock, int, pthread_mutex_trylock, (pthread_mutex_t *mutex));
CMOCK_MOCK_FUNCTION(PthreadMock, int, pthread_mutex_unlock, (pthread_mutex_t *mutex));

CMOCK_MOCK_FUNCTION(PthreadMock, int, pthread_condattr_init, (pthread_condattr_t *attr));
CMOCK_MOCK_FUNCTION(PthreadMock, int, pthread_condattr_destroy, (pthread_condattr_t *attr));

CMOCK_MOCK_FUNCTION(PthreadMock, int, pthread_cond_init, (pthread_cond_t *cond, const pthread_condattr_t *attr));
CMOCK_MOCK_FUNCTION(PthreadMock, int, pthread_cond_destroy, (pthread_cond_t *cond));
CMOCK_MOCK_FUNCTION(PthreadMock, int, pthread_cond_timedwait, (pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime));
CMOCK_MOCK_FUNCTION(PthreadMock, int, pthread_cond_wait, (pthread_cond_t *cond, pthread_mutex_t *mutex));
CMOCK_MOCK_FUNCTION(PthreadMock, int, pthread_cond_signal, (pthread_cond_t *cond));
CMOCK_MOCK_FUNCTION(PthreadMock, int, pthread_cond_broadcast, (pthread_cond_t *cond));