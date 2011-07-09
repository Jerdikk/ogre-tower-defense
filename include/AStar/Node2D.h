#pragma once
#include "GridNode.h"
#include "OgreIteratorWrappers.h" // For Ogre::MapIterator

class Node2D;
typedef std::vector<Node2D*> NodeList;
typedef std::map<int, Node2D*> NodeMap;
typedef Ogre::MapIterator<NodeMap> NodeMapItr;

#define INFINITY 9999999

class Node2D
{
public:
  Node2D(int index, int row, int col) : mIndex(index), mPos(row, col), f(0), g(INFINITY), h(0) {}
  ~Node2D(void) {}

  int getIndex(void) const {return mIndex;}
  virtual void addNeighbor(Node2D* node) {if (node != this) mNeighbors[node->getIndex()] = node;}
  virtual void removeNeighbor(Node2D* node) {if (mNeighbors.find(node->getIndex()) != mNeighbors.end()) {mNeighbors.erase(mNeighbors.find(node->getIndex()));}}
  NodeMapItr getIterator(void) {return NodeMapItr(mNeighbors.begin(), mNeighbors.end());}

  virtual float computeG(Node2D* neighbor) {return (neighbor->getPosition()-mPos).length();}
  virtual float computeH(Node2D* goal)     {return (goal->getPosition()-mPos).length();}
  virtual float updateF() {return (f = g + h);} // Compute f() for the node (g+h)

  const GridNode& getPosition(void) const {return mPos;}

public:
  float h;    // Heuristic Value - estimated cost from this node to goal
  float g;    // Total cost value from start to this node
  float f;    // Estimated total cost from start to goal through this node

private:
  const int mIndex;
  NodeMap mNeighbors;
  GridNode mPos;
};