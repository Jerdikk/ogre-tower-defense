#pragma once
#include <OgreSingleton.h>
#include <OgreDataStream.h>
#include "Alien.h"
#include "common.h"

class Mutex;

class AlienFactory : public Ogre::Singleton<AlienFactory>
{
public:
  AlienFactory(void);
  ~AlienFactory(void);

  // Parse a Wave from file.  This will build the container of WaveForms
  bool parseWave(Ogre::DataStreamPtr& stream);
  bool writeWaves(std::fstream& stream);

  // Clear all pending and expired waves, as well as all existing Aliens
  void clear(void);

  // Destroy a specific alien
  void destroyAlien(Alien* alient);
  void destroyAlien(unsigned int id);

  // Get an alien by id
  Alien* getAlien(unsigned int id);

  // Create an alien of the specified type.  This alien will be owned by the AlienFactory,
  // so do not try to destroy it directly.
  Alien* createAlien(const Ogre::String& type);

  // Set the starting point where new aliens should be added
  void setStartingPoint(const GridNode& startingPoint);

  // Based on the time since the last frame, see if the next wave needs to be created.
  // If so, all of the aliens for the wave will be created and added to the Aliens
  // container.
  void handleNextWave(float t, Aliens& aliens);

  // Returns a vector of Alien IDs in order of priority.  Priority is based
  // on multiple factors, such as alien type, and distance to the end or data.
  inline const UIntVector& getAlienPriorityList(void) {return mPriorityList;}

protected:
  // Update the priorty list of aliens based on their current position.  Regardless of how
  // often this is called, it will only update the list periodically to reduce overhead.
  // The order of priorities probably won't change as fast as the frame rate.
  void updateAlienPriorityList(void);

protected:
  struct Wave;
  typedef std::queue<Wave*> Waves;
  
  // Waves are stored in a pending queue until they are used, then they
  // are stored in an expired queue.  This is to save all deallocations
  // until clear() is called and all waves can be destroyed at once.
  Waves mPendingWaves;
  Waves mExpiredWaves;
  Aliens mAliens;

  GridNode mStartingPoint;
  Ogre::Real mTimeSinceLastWave;

  Mutex* mpMutex;
  UIntVector mPriorityList;
};
