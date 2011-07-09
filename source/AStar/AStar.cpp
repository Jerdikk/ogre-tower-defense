#include "AStar.h"

bool isInSet(Node2D* node, const NodeMap& set)
{
  return (set.find(node->getIndex()) != set.end());
}

Node2D* getNodeWithLowestF(NodeMap& set)
{
  NodeMap::iterator lowest = set.begin();
  if (set.size() != 1)
  {
    // Find the Node with the lowest f()
    NodeMap::iterator itr = lowest;
    for (; itr != set.end(); ++itr)
      if (itr->second->f < lowest->second->f) lowest = itr;
  }

  // Remove the node from the set
  Node2D* node = lowest->second;
  set.erase(lowest);

  return node;
}

Path AStar(Graph& graph, int startIndex, int goalIndex)
{
  Path path;
  NodeMap open_set;
  NodeMap closed_set;
  std::map<int, int> came_from;

  Node2D* startNode = graph[startIndex];
  Node2D* goalNode = graph[goalIndex];

  // Make sure both of the are valid Nodes
  assert(startNode && goalNode);

  startNode->g = 0;
  startNode->computeH(goalNode);
  startNode->updateF();
  open_set[startNode->getIndex()] = startNode;;
  
  while (!open_set.empty())
  {
    Node2D* node = getNodeWithLowestF(open_set);

    // See if we're at the goal
    if (node == goalNode)
    {
      path.push(goalNode->getPosition());
      break;
    }

    // Add this node to the closed set
    closed_set[node->getIndex()] = node;

    // Vist all children of the current node
    NodeMapItr itr = node->getIterator();
    while (itr.hasMoreElements())
    {
      Node2D* neighbor = itr.getNext(); // Advances iterator
      
      // See if this child is in the closed list
      if (isInSet(neighbor, closed_set)) continue;

      float tentative_g = node->g + neighbor->computeG(node);
      bool tentative_is_better = false;

      // See if child is in open set
      if (isInSet(neighbor, open_set) == false)
      {
        open_set[neighbor->getIndex()] = neighbor;
        tentative_is_better = true;
      }
      else if (tentative_g < neighbor->g)
      {
        tentative_is_better = true;
      }

      if (tentative_is_better)
      {
        came_from[neighbor->getIndex()] = node->getIndex();
        neighbor->g = tentative_g;
        neighbor->h = neighbor->computeH(goalNode);
        neighbor->updateF();
      }
    } // while
  } // while

  // Travel back through the nodes to get the path
  std::map<int, int>::iterator itr = came_from.find(goalNode->getIndex());
  while (itr != came_from.end())
  {
    path.push(graph[itr->second]->getPosition());
    itr = came_from.find(itr->second);
  }

  return path;
}