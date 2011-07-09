#pragma once
#include <OgrePrerequisites.h>
#include <OgreString.h>
#include <OgreVector2.h>
#include <OgreVector3.h>
#include <vector>
#include <queue>
#include <stack>
#include "GridNode.h"

class Alien;
class Tower;

typedef std::map<unsigned int, Alien*> Aliens;
typedef Aliens::iterator AlienItr;
typedef Aliens::const_iterator AlienCIter;

typedef std::vector<Tower*> Towers;
typedef Towers::iterator TowerItr;
typedef Towers::const_iterator TowerCItr;

typedef std::vector<int> IntVector;
typedef std::vector<unsigned int> UIntVector;
typedef std::stack<GridNode> Path;

#define MASK_CURSOR_TARGET 0x8000
#define MASK_TOWER_BASE    0x4000
#define MASK_ALIEN         0x2000

// States for each cell on the Map.
const int WALL              = 0;
const int OPEN              = 1;
const int TOWER_BASE        = 2;
const int TOWER_BASE_OPEN   = 3;
const int TOWER_BASE_CLOSED = 4;
const int POWER_CORE        = 5;
const int CORNER            = 100;
const int PATH_NODE         = 101;

// Convenience values for calculations involving map cells
const float CELL_WIDTH      = 100.0f;
const float HALF_CELL_WIDTH = 50.0f;
const float SQUARED_HALF_CELL_WIDTH = 2500.0f;
const Ogre::Vector3 WORLD_CELL(CELL_WIDTH, 0, -CELL_WIDTH);
const Ogre::Vector3 HALF_WORLD_CELL(HALF_CELL_WIDTH, 0, -HALF_CELL_WIDTH);

// Does a traditional rounding on a number.
// Round UP if the decimal portion is >= 0.5
// Round DOWN if the decimal portion is < 0.5
inline float round(const float& x)
{
  float dec = x - floor(x);
  if (dec >= 0.5f) return ceil(x);
  return floor(x);
}

// Returns the sign of a number
inline int sign(const float& x)
{
  return (x<0) ? -1 : 1;
}

// This returns the (x, y, z) position in worls space for the
// bottom left corner of a cell with the coordinates (row, col).
// The y-coordinate will always be 0.
inline Ogre::Vector3 cornerPosFromCoords(int row, int col)
{
  return WORLD_CELL * Ogre::Vector3(Ogre::Real(col), 0.0f, Ogre::Real(row));
}
inline Ogre::Vector3 cornerPosFromCoords(const GridNode& node)
{
  return cornerPosFromCoords(node.row, node.col);
}

// This returns the (x, y, z) position in world space for the 
// center of a cell with the coordinates (row, col).
// The y-coordinate will always be 0.
inline Ogre::Vector3 centerPosFromCoords(int row, int col)
{
 return cornerPosFromCoords(row, col) + HALF_WORLD_CELL;
}
inline Ogre::Vector3 centerPosFromCoords(const GridNode& node)
{
  return centerPosFromCoords(node.row, node.col);
}

// This returns the GridNode (row, col) for the cell which
// contains the point (x, y, z).  The y-coordinate is not used
// in this calculation.
inline GridNode gridNodeFromPos(const Ogre::Vector3& pos)
{
  return GridNode(int(-pos.z/CELL_WIDTH), int(pos.x/CELL_WIDTH));
}

// This checks the string str to see if it begins with the sub string first.
// If so, it will set after to be the rest of the string after first and return true.
// Otherwise, it simply returns false;
inline bool getStrAfterIfMatch(const Ogre::String& str, const Ogre::String& first, Ogre::String& after)
{
  if (str.length() > first.length() && str.substr(0, first.length()) == first)
  {
    after = str.substr(first.length());
    Ogre::StringUtil::trim(after);
    return true;
  }
  return false;
}