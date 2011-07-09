#pragma once
#include <OgrePrerequisites.h>
#include "Common.h"

class StatusBar;

class Alien
{
public:
  enum State
  {
    ENTERING,       // First entering the map
    AWAITING_DATA_PATH, // Waiting for a path to the data
    TRACKING_DATA,  // Following a path to the data
    AWAITING_EXIT_PATH, // Waiting for a path to the exit
    TRACKING_EXIT,  // Following a path to the exit
    EXITING,        // Exiting the map (after reaching the exit)
    EXITED,         // The alien has exited the map and can be removed
    DYING,          // The alien is dying, but not yet dead
    DEAD,           // The alien is dead and can be removed from the map
  };
  virtual ~Alien(void);

  // Pure Virtuals
  virtual bool createGraphics(const GridNode& startPos) = 0;
  
  // Statics
  static void initialize(Ogre::SceneManager* sceneMgr);
  
  // Update will do some updates common to all aliens, then call updateLocal
  // which can be reimplemented for type-specific updates.
  void update(float t);

  // Gets
  const unsigned int& getUid(void) const;
  const Ogre::String& getType(void) const;
  const Ogre::Vector3& getPosition(void) const;
  const Ogre::Vector3& getDirection(void) const;
  const State& getState(void) const;
  const Ogre::Real& getCenterOffset(void) const;
  Ogre::Real getDistanceToGoal(void) const;
  Ogre::Real getWalkSpeed(void) const;
  
  // Sets
  void setPath(const Path& path);
  void setStartingPositionWithOffset(const GridNode& start, Ogre::Real offset, bool randomize=false);

  void damage(Ogre::Real amount);
  void slow(Ogre::Real speedPerc, Ogre::Real secs);

protected:
  // Aliens can only be created by the AlienFactory, so the ctor is protected.
  Alien(const Ogre::String& type);
  virtual void followPath(float t);
  void updateStatusBar(void);
  virtual void updateDeath(float t);
  virtual void updateLocal(float t);  // Derived class should reimplement for specific type updates

protected:
  static Ogre::SceneManager* mpsSceneMgr;
  
  // Constants
  const Ogre::String mcType;       // Set in Alien ctor
  const Ogre::Real mcEnterSpeed;   // Speed at which the alien enters/exits the map
  const Ogre::Real mcEnterHeight;  // Height at which the alien enters/exits the map
  const unsigned int mcUid;        // Unique ID for the alien
  
  Ogre::Real mWalkSpeed;          // Alien's standard movement speed
  Ogre::Real mMaxHealth;          // Alien's max health (constant for the Alien's life time)
  Ogre::Real mCurHealth;          // Alien's current health
  Ogre::Real mSquaredPathTolerance; // Squared tolerance distance from a target position for the Alien to consider reaching it
  Ogre::Real mSpeedPerc;          // % of the Alien's normal walk speed (used when slowing Aliens)
  Ogre::Real mSlowDuration;       // How much longer the Alien has of being slowed
  Ogre::Real mCenterOffset;       // Height above the ground of the Alien's center
  Ogre::Real mDistToGoal;         // Distance to the Alien's goal.
  Path mPath;
  Ogre::Vector3 mTargetPos;
  Ogre::Vector3 mDirection;       // Direction the alien is traveling.
  bool mPickedUpData;
  State mState;
  StatusBar* mpStatusBar;

  Ogre::Entity* mpGraphics;
  Ogre::SceneNode* mpNode;

  float tempDeathTime;

private:
  // The default ctor should never be used.
  Alien(void) : mcUid(-1), mcEnterSpeed(0), mcEnterHeight(0) {};

private:
  static unsigned int msUidCounter;
};