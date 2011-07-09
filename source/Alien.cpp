#include "Alien.h"
#include "StatusBar.h"
#include <Ogre.h>

Ogre::SceneManager* Alien::mpsSceneMgr = NULL;
unsigned int Alien::msUidCounter = 0;

Alien::Alien(const Ogre::String& type)
: mcType(type), mcUid(msUidCounter++), mcEnterSpeed(66.0f), mcEnterHeight(300.0f)
{
  mpGraphics = NULL;
  mpNode = NULL;
  mSquaredPathTolerance = 1;
  mWalkSpeed = 66;  // Units per second
  mPickedUpData = false;
  mState = ENTERING;
  mpStatusBar = new StatusBar(this);
  mSpeedPerc = 1.0f;
  mSlowDuration = 0.0f;
  mCurHealth = 100.0f;
  mMaxHealth = 100.0f;
  mDistToGoal = 9999999.0f;

  tempDeathTime = 0;
}

Alien::~Alien(void)
{
  // Destroy StatusBar
  if (mpStatusBar)
  {
    delete mpStatusBar;
    mpStatusBar = NULL;
  }

  if (mpsSceneMgr)
  {
    // Destroy the SceneNode
    if (mpNode)
    {
      mpNode->detachAllObjects();
      mpsSceneMgr->destroySceneNode(mpNode);
      mpNode = NULL;
    }

    // Destroy the Entity
    if (mpGraphics)
    {
      mpsSceneMgr->destroyEntity(mpGraphics);
      mpGraphics = NULL;
    }
  }
}

// static
void Alien::initialize(Ogre::SceneManager* sceneMgr)
{
  mpsSceneMgr = sceneMgr;
  StatusBar::initialize(sceneMgr);
}

void Alien::update(float t)
{
  if (mState == DYING)
  {
    updateDeath(t);
  }
  else
  {
    // Update the status bar
    updateStatusBar();

    // Update speed
    mSlowDuration -= t;
    if (mSlowDuration <= 0)
    {
      mSpeedPerc = 1.0f;
      mSlowDuration = 99999; // Just to avoid this if again for awhile
    }
    updateLocal(t);
  }
}

const unsigned int& Alien::getUid(void) const
{
  return mcUid;
}

const Ogre::String& Alien::getType(void) const
{
  return mcType;
}

const Ogre::Vector3& Alien::getPosition(void) const
{
  if (mpNode) return mpNode->getPosition();
  return Ogre::Vector3::ZERO;
}

const Ogre::Vector3& Alien::getDirection(void) const
{
  return mDirection;
}

const Alien::State& Alien::getState(void) const
{
  return mState;
}

const Ogre::Real& Alien::getCenterOffset(void) const
{
  return mCenterOffset;
}

Ogre::Real Alien::getDistanceToGoal(void) const
{
  Ogre::Real distToGoal = 0;
  
  // Since the path is a stack, we need to make a copy so we can step through it
  Path tempPath(mPath);
  Ogre::Vector3 prevPos = getPosition();
  while (!tempPath.empty())
  {
    Ogre::Vector3 nextPos = centerPosFromCoords(tempPath.top());
    tempPath.pop();

    // This is actually the squared distance to goal, but it doesn't matter.
    distToGoal += prevPos.distance(nextPos);
    prevPos = nextPos;
  }

  return distToGoal;
}

Ogre::Real Alien::getWalkSpeed(void) const
{
  return mWalkSpeed;
}

void Alien::setPath(const Path& path)
{
  mPath = path;

  // If the first node in the path is the same as our current node, skip it
  if (mPath.size() > 1)
  {
    GridNode diff = gridNodeFromPos(getPosition()) - mPath.top();
    if (diff.row == 0 && diff.col == 0)
      mPath.pop();
  }

  // Set the target position to be the first node in the path, and then randomly offset
  // it a little so that this Alien isn't walking to the exact same point as any other
  // Alien.
  mTargetPos = centerPosFromCoords(mPath.top());
  mTargetPos.x += (fmod(rand(), HALF_CELL_WIDTH*0.4f) * (rand()%2 ? 1 : -1));
  mTargetPos.z += (fmod(rand(), HALF_CELL_WIDTH*0.4f) * (rand()%2 ? 1 : -1));

  // Update the state
  if      (mState == AWAITING_DATA_PATH) mState = TRACKING_DATA;
  else if (mState == AWAITING_EXIT_PATH) mState = TRACKING_EXIT;
}

void Alien::setStartingPositionWithOffset(const GridNode& start, Ogre::Real offset, bool randomize)
{
  Ogre::Vector3 pos = centerPosFromCoords(start.row, start.col);

  // Randomize the position withing the cell a little
  if (randomize)
  {
    pos.x += (fmod(rand(), HALF_CELL_WIDTH*0.8f) * (rand()%2 ? 1 : -1));
    pos.z += (fmod(rand(), HALF_CELL_WIDTH*0.8f) * (rand()%2 ? 1 : -1));
  }

  // Offset
  pos.y = mcEnterHeight + (offset * mcEnterSpeed);

  // Update the position
  mpNode->setPosition(pos);
}

void Alien::damage(Ogre::Real amount)
{
  mCurHealth -= amount;
  if (mCurHealth < 0)
  {
    // Update the status bar with health = 0 to make sure the
    // bar is displayed completely red when the alien dies.
    mCurHealth = 0;
    updateStatusBar();

    mState = DYING;
  }
}

void Alien::slow(Ogre::Real speedPerc, Ogre::Real secs)
{
  // Only do this if the new speed % is <= to the current one.  Otherwise,
  // a faster slow effect could override a slower one.
  if (speedPerc <= mSpeedPerc)
  {
    mSpeedPerc = speedPerc;
    mSlowDuration = secs;
  }
}

void Alien::followPath(float t)
{
  if (mState == ENTERING)
  {
    const Ogre::Vector3& pos = getPosition();

    // Lower the alien onto the map until its y-coordinate is 0
    if (pos.y > 0)
    {
      Ogre::Vector3 newPos(pos);
      newPos.y -= (t*mcEnterSpeed);
      mpNode->setPosition(newPos);
    }
    else
    {
      mpNode->setPosition(pos * Ogre::Vector3(1, 0, 1));
      mState = AWAITING_DATA_PATH;
    }
  }
  else if ((mState == TRACKING_DATA || mState == TRACKING_EXIT) && !mPath.empty())
  {
    Ogre::Vector3 pos = getPosition();
    
    // Move toward this target node
    mDirection = (mTargetPos - pos).normalisedCopy();
    Ogre::Vector3 newPos = pos + (mDirection * mWalkSpeed * mSpeedPerc * t);

    // Update distance to goal (since dir is the difference between the current position
    // and the target position, it's also the distance traveled)
    mDistToGoal -= (newPos-pos).squaredLength();

    bool reachedTarget = false;

    // See if we're within the tollerance to the target node
    if ((mTargetPos - pos).squaredLength() < mSquaredPathTolerance)
    {
      reachedTarget = true;
    }
    // See if we just over shot the target (low FPS may cause this)
    else if (sign(mTargetPos.x - pos.x) != sign(mTargetPos.x - newPos.x) &&
             sign(mTargetPos.z - pos.z) != sign(mTargetPos.z - newPos.z))
    {
      reachedTarget = true;

      // Update the alien's position with the target position
      mpNode->setPosition(mTargetPos);
    }

    // If the alien reach its target this frame, move to the next target
    // in the path
    if (reachedTarget)
    {
      mPath.pop();
      if (mPath.empty())
      {
        if (mState == TRACKING_DATA)
        {
          // If there are no more nodes in the path and our state was
          // tracking data, we've reached the data and now need to wait 
          // for a path to the exit
          mState = AWAITING_EXIT_PATH;
        }
        else if (mState == TRACKING_EXIT)
        {
          mState = EXITING;
        }
      }
      else
      {
        // The path is not empty, so follow the next point
        mTargetPos = centerPosFromCoords(mPath.top());
        mTargetPos.x += (fmod(rand(), HALF_CELL_WIDTH*0.4f) * (rand()%2 ? 1 : -1));
        mTargetPos.z += (fmod(rand(), HALF_CELL_WIDTH*0.4f) * (rand()%2 ? 1 : -1));
        followPath(t);
      }
    }

    // We're not within the tollerance of the target node, so move
    // closer toward it
    else
    {
      // Update the alien's position
      mpNode->setPosition(newPos);
    }
  }
  else if (mState == EXITING)
  {
    const Ogre::Vector3& pos = getPosition();

    // Lift up the alien until it exits the map
    if (pos.y < mcEnterHeight)
    {
      Ogre::Vector3 newPos(pos);
      newPos.y += (t*mcEnterSpeed);
      mpNode->setPosition(newPos);
    }
    else
    {
      mState = EXITED;
    }
  }
}

void Alien::updateStatusBar(void)
{
  if (mpStatusBar) mpStatusBar->update(getPosition(), mCurHealth / mMaxHealth);
}

//virtual 
void Alien::updateDeath(float t)
{
  // :TEMP: For now, just wait one second and go ahead and kill the Alien
  tempDeathTime += t;
  if (tempDeathTime >= 1.0f)
    mState = DEAD;
}

//virtual
void Alien::updateLocal(float t)
{
  followPath(t);
}