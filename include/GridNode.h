#pragma once
#include "OgreVector2.h"

/**
  A GridNode represents a node on a grid.  Duh.  It is essentially
  like a Vector2, but grid nodes are most commonly referenced by
  (row, col), which translates to (y, x) for a Vector2.  Since this
  is backward from normal, it's easier to just create a new object
  to represent the grid node so that there is no confusion.
  The GridNode class can convert to and from an Ogre::Vector2, but
  keep in mind that any Vector2 is assumed to be in the format of
  x==col, y==row.
**/
class GridNode
{
public:
  GridNode(void) : row(0), col(0) {}
  GridNode(const GridNode& node) : row(node.row), col(node.col) {}
  GridNode(const Ogre::Vector2& vec) : row((int)vec.y), col((int)vec.x) {}
  GridNode(int row, int col) : row(row), col(col) {}
  ~GridNode(void) {}

  Ogre::Real distance(const GridNode& to) const {return ((*this)-to).length();}
  Ogre::Real lengthSquared(void) const {return Ogre::Real(row*row + col*col);}
  Ogre::Real length(void) const {return sqrt(lengthSquared());}

  // Casting
  Ogre::Vector2 toVector(void) const {return Ogre::Vector2(Ogre::Real(col), Ogre::Real(row));}
  Ogre::Vector2 operator () (void) const {return Ogre::Vector2(Ogre::Real(col), Ogre::Real(row));}

  // Binary Operators
  inline GridNode operator - (const GridNode& rhs) const
    {return GridNode(row-rhs.row, col-rhs.col);}

  inline GridNode operator + (const GridNode& rhs) const
    {return GridNode(row+rhs.row, col+rhs.col);}

  inline bool operator == (const GridNode& rhs) const
    {return (row==rhs.row && col == rhs.col);}

  inline bool operator != (const GridNode& rhs) const
    {return !(row==rhs.row && col == rhs.col);}


  int row;
  int col;
};