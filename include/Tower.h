#pragma once
#include <OgrePrerequisites.h>
#include <OgreUserObjectBindings.h>
#include "common.h"

class TowerBase;

class Tower : public Ogre::UserObjectBindings
{
public:
  enum Role
  {
    OFFENSIVE,
    DEFENSIVE,
    UTILITY,
  };

  virtual ~Tower(void);

  static void initialize(Ogre::SceneManager* sceneMgr);
  
  inline const Ogre::String& getType(void) const {return mType;}
  inline const Role& getRole(void) const {return mRole;}
  inline int getLevel(void) const {return mLevel;}

  // Upgrade the tower to its next level
  virtual void upgrade(void) = 0;

  // Sell the tower;
  virtual void sell(void) = 0;

  // Create the graphics for this tower
  virtual bool createGraphics(void) = 0;

  // Called each frame to update the tower's graphics, target, etc...
  virtual void update(float t) = 0;

protected:
  // Can only be called by subclass
  Tower(TowerBase* parent, const Ogre::String& type, Role role);

  // Creates a simple tower manually rather than loading a mesh
  bool createSimpleGraphics(const Ogre::String& uniqueName);

  // Gets the ID of the alien with the highest priority within this tower's range
  virtual unsigned int getHighestPriorityTarget(void);

  // Returns the IDs of all aliens within range of this target
  virtual UIntVector getTargetsInRange(void);

  // Set the tower's range.  Should be called from the constructor of a superclass
  void setRange(Ogre::Real range);

  // Sets the tower's attack speed (Hz) and period (1/Hz), unless the speed is 0
  // (ie, Utility towers).  In that case, both speed and period are set to 0.
  void setAttackSpeed(Ogre::Real speedHz);

  // Returns true if the alien is within this tower's range, false otherwise.
  bool isTargetWithinRange(const Alien* alien);

protected:
  static Ogre::SceneManager* mpsSceneMgr;
  
  const int mUid;
  const Ogre::String mStrUid;
  const Ogre::String mType;
  const Role mRole;

  TowerBase* mpParent;
  Ogre::ManualObject* mpGraphics;
  Ogre::SceneNode* mpNode;
  Ogre::String mMaterialName;

  int mLevel;
  int mValue;
  Ogre::Real mRange;      // Range in world units
  Ogre::Real mRangeSqr;   // Range squared in world units
  Ogre::Real mAtkSpeed;   // Speed in Hz
  Ogre::Real mAtkPeriod;  // 1/AtkSpeed
  Ogre::Real mAtkPower;   // Power in damage/second
  Ogre::Real mTimeSinceLastAction;

private:
  // No Copy
  Tower(void) : mType(""), mUid(0), mStrUid(""), mRole(Role(0)) {}
  Tower(const Tower&) : mType(""), mUid(0), mStrUid(""), mRole(Role(0)) {}
  Tower& operator = (const Tower&) {return *this;}

private:
  static int msCounter;
};