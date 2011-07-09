#pragma once
#include "Mutex.h"

#define MAX_THREADS 40
typedef void (*threadFunc)(void*);

struct ThreadData
{
  void* handle;
  unsigned long id;
  threadFunc func;
  void* userarg;
};

class ThreadManager
{
public:
  static ThreadManager* instance(void);
  static void msleep(unsigned int msec);
  static void sleep(unsigned int sec);

  unsigned long startThread(threadFunc func, void* userArgs);
  void releaseThread(void*);
  void releaseThread(unsigned long threadID);
  void waitForAllThreads(void);

private:
  static ThreadManager* mpsMe;

  ThreadData mThreads[MAX_THREADS];  // Array of ThreadData structures to preserve data passed to threads
  Mutex* mpMutex;

  ThreadManager(void);
  ~ThreadManager(void);
};