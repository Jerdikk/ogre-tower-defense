#include "ThreadManager.h"
#include <windows.h>
#include <stdio.h>

unsigned long __stdcall _threadFunc(void*);

ThreadManager* ThreadManager::mpsMe = NULL;

ThreadManager::ThreadManager(void)
{
  mpMutex = new Mutex();
  ZeroMemory(mThreads, sizeof(mThreads));
}

ThreadManager::~ThreadManager(void)
{
}

ThreadManager* ThreadManager::instance(void)
{
  if (!mpsMe) mpsMe = new ThreadManager();
  return mpsMe;
}

unsigned long ThreadManager::startThread(threadFunc func, void* userArgs)
{
  SCOPELOCK(mpMutex);
  for (int idx = 0; idx < MAX_THREADS; ++idx)
  {
    void* handle = mThreads[idx].handle;

    // Only use this thread if the thread does not yet exist or is currently signaled
    if (handle == NULL || WaitForSingleObject(handle, 0) == WAIT_OBJECT_0)
    {
      printf("ThreadManager >> creating thread in index %d\n", idx);
      mThreads[idx].func = func;
      mThreads[idx].userarg = userArgs;
      mThreads[idx].handle = CreateThread(NULL,         // Default security Attributes
                                   0,            // Default stack size
                                   _threadFunc,  // Thread Function
                                   &mThreads[idx],  // Thread parameter
                                   0,            // Creation flags
                                   &(mThreads[idx].id));        // Thread ID (not the handle)

      // Return thread id
      return mThreads[idx].id;
    }
  }

  // Thread not created/started
  printf("ThreadManager >> Request for new thread denied.  No threads available.\n");
  return 0;
}

void ThreadManager::releaseThread(void* handle)
{
  for (int idx = 0; idx < MAX_THREADS; ++idx)
  {
    if (mThreads[idx].handle == handle)
    {
      CloseHandle(handle);
      mThreads[idx].handle = NULL;
      break;
    }
  }
}

void ThreadManager::releaseThread(unsigned long threadID)
{
  for (int idx = 0; idx < MAX_THREADS; ++idx)
  {
    if (mThreads[idx].id == threadID)
    {
      CloseHandle(mThreads[idx].handle);
      ZeroMemory(&mThreads[idx], sizeof(ThreadData));
      break;
    }
  }
}

void ThreadManager::waitForAllThreads(void)
{
  // Copy the valid handles into a seperate array
  int numValid = 0;
  void* validHandles[MAX_THREADS];
  for (int i = 0; i < MAX_THREADS; ++i)
  {
    if (mThreads[i].handle != NULL)
      validHandles[numValid++] = mThreads[i].handle;
  }

  // Only proceed if at least 1 valid handle
  if (numValid > 0)
  {
    DWORD retval = WaitForMultipleObjects(numValid, validHandles, true, INFINITE);
    if (retval == WAIT_TIMEOUT)
      printf("WaitForMultipleObjects timed out\n");
    else if (retval == WAIT_FAILED)
      printf("WaitForMultipleObjects failed.  Error code %d\n", GetLastError());
    else if (retval >= 0x00000080L)
      printf("WaitForMultipleObjects abandoned.\n");
  }
}

void ThreadManager::msleep(unsigned int msec)
{
  ::Sleep(msec);
}

void ThreadManager::sleep(unsigned int sec)
{
  ::Sleep(sec*1000);
}



unsigned long __stdcall _threadFunc(void* threadData)
{
  ThreadData* data = (ThreadData*)(threadData);
  data->func(data->userarg);
  return 0;
}