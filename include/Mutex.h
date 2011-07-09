#pragma once

#define SCOPELOCK(m) MutexLocker locker(m);

class Mutex
{
public:
  Mutex(void);
  ~Mutex(void);

  void lock(void);
  bool tryLock(void);
  bool tryLock(int timeout);
  void unlock();
  bool locked();

private:
  // Disable copying
  Mutex(const Mutex&) {}
  Mutex& operator = (const Mutex&) {}

private:
  void* handle;
};


class MutexLocker
{
public:
  MutexLocker(Mutex*);
  ~MutexLocker(void);

  Mutex* m;
};