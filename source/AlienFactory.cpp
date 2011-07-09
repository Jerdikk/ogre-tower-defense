#include "AlienFactory.h"
#include <OgreString.h>
#include <OgreStringConverter.h>
#include "AlienDrone.h"
#include "AlienSwarmer.h"
#include "AlienTank.h"
#include "ThreadManager.h"

template<> AlienFactory* Ogre::Singleton<AlienFactory>::ms_Singleton = NULL;

/* A Wave represents a group of aliens that enter the scene around
 * the same time as each other.  It can be made up of any number of
 * aliens of any time, and in varying formations. */
struct AlienFactory::Wave
{
  /* A Wave is made up of one or more WaveForms.  A WaveForm contains
   * a form type (the "shape" of the group) which represents how the
   * group of aliens will file out.  A WaveForm can only contain one
   * type of alien (though it can have as many of that type as you
   * want), so if you want a cluster of two different alien types, you
   * have to create two cluster waveforms, one for each type. */
  struct WaveForm {
    enum Type {
      INVALID,
      SINGLE,
      SERIES,
      CLUSTER,
    } type;                     // The type of waveform
    Ogre::Real      time;       // When this waveform starts relative to the wave
    Ogre::String    alienType;  // The type of alien in this waveform
    unsigned short  count;      // How many of these aliens to create
    bool            scattered;  // (SERIES) If true, the aliens will not walk in a straight line
    Ogre::Real      interval;   // (SERIES) How far apart in time to space the aliens

    void clear() {memset(this, 0, sizeof(WaveForm)); alienType = Ogre::String(); count = 1;}
  };

  Ogre::Real offset;
  std::queue<WaveForm> waveforms;
};




AlienFactory::AlienFactory(void)
{
  mTimeSinceLastWave = 0;
  mpMutex = new Mutex();
}

AlienFactory::~AlienFactory(void)
{
}

bool AlienFactory::parseWave(Ogre::DataStreamPtr& stream)
{
  bool succeeded = true;
  bool parsing = true;
  bool parsingWaveForm = true;
  Ogre::String line;
  Ogre::String substr;
  Wave* wave = new Wave();
  Wave::WaveForm::Type newType = Wave::WaveForm::INVALID;
  Wave::WaveForm waveform;
  waveform.clear();

  // First line should always be the offset
  line = stream->getLine();
  if (getStrAfterIfMatch(line, "Offset:", substr)) 
  {
    wave->offset = Ogre::StringConverter::parseReal(substr);
  }
  else
  {
    // The first line was not the offset.  Invalide Wave definition
    parsing = false;
    succeeded = false;
  }

  while (parsing)
  {
    // Get the next line
    line = stream->getLine();

    if (line.length() > 0)
    {
      if      (line == "Series")  newType = Wave::WaveForm::SERIES;
      else if (line == "Single")  newType = Wave::WaveForm::SINGLE;
      else if (line == "Cluster") newType = Wave::WaveForm::CLUSTER;

      if (newType != Wave::WaveForm::INVALID)
      {
        // Add the current WaveForm to the queue
        if (waveform.type != Wave::WaveForm::INVALID)
          wave->waveforms.push(waveform);

        // Start a new WaveForm definition
        waveform.clear();
        waveform.type = newType;
        
        newType = Wave::WaveForm::INVALID;
      }
      else if (waveform.type != Wave::WaveForm::INVALID)
      {
        // Parse the WaveForm attributes
        if (getStrAfterIfMatch(line, "Time:", substr))
        {
          waveform.time = Ogre::StringConverter::parseReal(substr);
        }
        else if (getStrAfterIfMatch(line, "Alien:", substr))
        {
          waveform.alienType = substr;
        }
        else if (getStrAfterIfMatch(line, "Count:", substr))
        {
          waveform.count = Ogre::StringConverter::parseUnsignedInt(substr);
        }
        else if (getStrAfterIfMatch(line, "Scattered:", substr))
        {
          waveform.scattered = Ogre::StringConverter::parseBool(substr);
        }
        else if (getStrAfterIfMatch(line, "Interval:", substr))
        {
          waveform.interval = Ogre::StringConverter::parseReal(substr);
        }
        else
        {
          // Invalid waveform definition
          parsing = false;
          succeeded = false;
        }
      }
      else
      {
        // Invalid wave definition
        parsing = false;
        succeeded = false;
      }
    }
    else
    {
      // Add the current WaveForm to the queue
      if (waveform.type != Wave::WaveForm::INVALID) 
        wave->waveforms.push(waveform);

      // End of this Wave definition
      parsing = false;
    }
  } // while (parsing)
  
  if (succeeded) mPendingWaves.push(wave);
  else           delete wave;

  return succeeded;
}

bool AlienFactory::writeWaves(std::fstream& stream)
{
  Waves copyWaves(mPendingWaves);
  bool succeeded = !copyWaves.empty();
  
  while (!copyWaves.empty())
  {
    Wave* wave = copyWaves.front();
    copyWaves.pop();

    // break and return false if this wave has no waveforms
    if (wave->waveforms.empty())
    {
      succeeded = false;
      break;
    }

    stream << "Wave" << std::endl;
    stream << "  Offset: " << wave->offset << std::endl;
    while (!wave->waveforms.empty())
    {
      Wave::WaveForm form = wave->waveforms.front();
      wave->waveforms.pop();
      
      if (form.type == Wave::WaveForm::SINGLE)
        stream << "  Single" << std::endl;
      else if (form.type == Wave::WaveForm::SERIES)
        stream << "  Series" << std::endl;
      else if (form.type == Wave::WaveForm::CLUSTER)
        stream << "  Cluster" << std::endl;
      else 
      {
        succeeded = false;
        break;
      }

      // Common waveform details
      stream << "    Time: " << form.time << std::endl;
      stream << "    Alien: " << form.alienType << std::endl;

      // Type specific details
      switch (form.type)
      {
        case Wave::WaveForm::SERIES:
          stream << "    Scattered: " << (form.scattered ? "true" : "false") << std::endl;
          stream << "    Count: " << form.count << std::endl;
          stream << "    Interval: " << form.interval << std::endl;
          break;

        case Wave::WaveForm::CLUSTER:
          stream << "    Count: " << form.count << std::endl;
          break;
          
        default: break;
      }
    } // End waveform definition

    // newline after each wave
    stream << std::endl;
  } // End wave definition

  return succeeded;
}

void AlienFactory::clear(void)
{
  // Clear the pending waves
  while (mPendingWaves.empty())
  {
    delete mPendingWaves.front();
    mPendingWaves.pop();
  }

  // Clear the expired waves
  while (mExpiredWaves.empty())
  {
    delete mExpiredWaves.front();
    mExpiredWaves.pop();
  }

  // Destroy all aliens
  for (AlienItr itr = mAliens.begin(); itr != mAliens.end(); ++itr)
    delete itr->second;
  mAliens.clear();

  // Reset time since last wave
  mTimeSinceLastWave = 0;
}

void AlienFactory::destroyAlien(Alien* alien)
{
  if (alien) destroyAlien(alien->getUid());
}

void AlienFactory::destroyAlien(unsigned int id)
{
  // Remove from Alien list
  AlienItr itr = mAliens.find(id);
  if (itr != mAliens.end())
  {
    delete itr->second;
    mAliens.erase(itr);
  }

  // Remove from priority list if it's in there.  Must scopelock it
  // because mPriorityList can be accessed by a background thread.
  SCOPELOCK(mpMutex);
  UIntVector::iterator vitr = mPriorityList.begin();
  for (; vitr != mPriorityList.end(); ++vitr)
  {
    if (*vitr == id)
    {
      mPriorityList.erase(vitr);
      break;
    }
  }
}

Alien* AlienFactory::getAlien(unsigned int id)
{
  Alien* alien = NULL;

  Aliens::iterator itr = mAliens.find(id);
  if (itr != mAliens.end())
    alien = itr->second;
  
  return alien;
}

Alien* AlienFactory::createAlien(const Ogre::String& type)
{
  Alien* alien = NULL;
  if      (type == "Drone") alien = new AlienDrone();
  else if (type == "Swarmer") alien = new AlienSwarmer();
  else if (type == "Tank") alien = new AlienTank();
  else printf("Invalide alien type: \"%s\"\n", type.c_str());
  
  if (alien) mAliens[alien->getUid()] = alien;

  return alien;
}

void AlienFactory::setStartingPoint(const GridNode& startingPoint)
{
  mStartingPoint = startingPoint;
}

void AlienFactory::handleNextWave(float t, Aliens& aliens)
{
  if (!mPendingWaves.empty())
  {
    // Update tiem since last wave
    mTimeSinceLastWave += t;

    Wave* wave = mPendingWaves.front();
    if (mTimeSinceLastWave >= wave->offset)
    {
      // Remove this Wave from the pending waves and add it to the expired waves queue
      mPendingWaves.pop();
      mExpiredWaves.push(wave);

      // Process this wave
      Wave::WaveForm waveform;
      while (!wave->waveforms.empty())
      {
        // Get the next WaveForm
        waveform = wave->waveforms.front();
        wave->waveforms.pop();

        for (int i = 0; i < waveform.count; ++i)
        {
          Alien* alien = createAlien(waveform.alienType);
          if (alien)
          {
            Ogre::Real offset = waveform.time;
            bool randomize = false;

            // For a Series waveform, update the offset for each alien and optionally randomize
            if (waveform.type == Wave::WaveForm::SERIES) 
            {
              offset += (i * waveform.interval);
              randomize = (waveform.scattered);
            }

            // For a Cluster waveform, randomize the positions
            else if (waveform.type == Wave::WaveForm::CLUSTER)
            {
              randomize = true;
            }

            // Initialize this alien's starting position
            alien->createGraphics(mStartingPoint);
            alien->setStartingPositionWithOffset(mStartingPoint, offset, randomize);

            // Add it to the local container and the container passed in
            mAliens[alien->getUid()] = alien;
            aliens[alien->getUid()] = alien;
          }
        } // for (waveform.count)
      } // while (not waveforms empty)
    }
  } // if (not pending waves empty)
  updateAlienPriorityList();
}

void AlienFactory::updateAlienPriorityList(void)
{
  typedef std::pair<Ogre::Real, unsigned int> AlienDistPair;  // <distance, id>
  static clock_t sUpdatePeriod = 100; // milliseconds
  static clock_t sTimeOfLastUpdate = clock();
  
  clock_t now = clock();
  if (now - sTimeOfLastUpdate >= sUpdatePeriod)
  {
    sTimeOfLastUpdate = now;
    
    // Two vectors of AlienDistPairs used to sort aliens based on distance.  The first
    // one is for aliens on their way to the data, the second is for aliens on their
    // way to the exit.
    std::vector<AlienDistPair> distPairs1;
    std::vector<AlienDistPair> distPairs2;

    // Iterate through all aliens and add those that are tracking data to the first vector
    // and those that are tracking the exit to the second vector.  Ignore all others.
    AlienCIter citr = mAliens.begin();
    for (; citr != mAliens.end(); ++citr)
    {
      Alien* alien = citr->second;
      Ogre::Real dist = alien->getDistanceToGoal();
      unsigned int id = alien->getUid();

      if (alien->getState() == Alien::TRACKING_DATA)
      {
        distPairs1.push_back(AlienDistPair(dist, id));
      }
      else if (alien->getState() == Alien::TRACKING_EXIT)
      {
        distPairs2.push_back(AlienDistPair(dist, id));
      }
    }

    // Sort both vectors based on distance
    std::sort(distPairs1.begin(), distPairs1.end());
    std::sort(distPairs2.begin(), distPairs2.end());

    
    // Add all Alien IDs to the priority list, starting with those tracking the exist
    mPriorityList.clear();
    for (size_t i = 0; i < distPairs2.size(); ++i)
      mPriorityList.push_back(distPairs2[i].second);
    for (size_t i = 0; i < distPairs1.size(); ++i)
      mPriorityList.push_back(distPairs1[i].second);
  }
}