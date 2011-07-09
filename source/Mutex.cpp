#include "Mutex.h"
#include <windows.h>

Mutex::Mutex(void)
{
  // Create the mutex
  handle = CreateMutex(NULL, false, NULL);
}

Mutex::~Mutex(void)
{
  // Release the mutex
  CloseHandle(handle);
}

void Mutex::lock(void)
{
  tryLock(INFINITE);
}

bool Mutex::tryLock(void)
{
  return tryLock(INFINITE);
}

bool Mutex::tryLock(int timeout)
{
  return (WaitForSingleObject(handle, timeout) == WAIT_OBJECT_0);
}

void Mutex::unlock()
{
  ReleaseMutex(handle);
}

bool Mutex::locked()
{
  bool waslocked = (ReleaseMutex(handle) != 0);
  if (waslocked) lock();
  return waslocked;
}


MutexLocker::MutexLocker(Mutex* p)
{
  m = p;
  m->lock();
}

MutexLocker::~MutexLocker(void)
{
  m->unlock();
}