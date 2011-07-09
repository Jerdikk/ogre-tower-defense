#include "Level.h"
#include "Graph.h"
#include "Node2D.h"
#include "AStar.h"
#include "Common.h"


bool isInLos(const Map& map, const GridNode& thisnode, const GridNode& node)
{
  int x0 = thisnode.col;
  int y0 = thisnode.row;
  int x1 = node.col;
  int y1 = node.row;

  bool steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep)
  {
    std::swap(x0, y0);
    std::swap(x1, y1);
  }
  if (x0 > x1)
  {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  // Delta X and Y
  int dx = x1 - x0;
  int dy = abs(y1 - y0);

  // Error and Delta Error
  float e = 0;
  float de = (float)dy / (float)dx;
  int ystep;
  int y = y0;
  int z;  // Used when calculating line thickness

  // Thickness is based on the ratio dy/dx.  Could be anything
  // from h/2 to h
  float h = 0.99; // height of the cell with a small bit of tolerance
  float thickness = h * (0.5 + (abs(de)*0.5));

  // Since y0 and y1 may have swapped, we need to know which is the
  // min and max
  int minY = std::min<int>(y0, y1);
  int maxY = std::max<int>(y0, y1);

  // Adjust ystep based on whether y0 is below y1
  if (y0 < y1) ystep = 1;
  else         ystep = -1;


  for (int x = x0; x <= x1; ++x)
  {
    if (steep) 
    {
      if (map.isBarrier(x, y)) return false;
      
      // calculate the upper and lower bounds of the region
      // around the line at this step.  Make sure we never
      // check above/below the end points
      z = round(y - (abs(e)+thickness));
      if (z >= minY) if(map.isBarrier(x, z)) return false;

      z = round(y + (abs(e)+thickness));
      if (z <= maxY) if(map.isBarrier(x, z)) return false;
    }
    else       
    {
      if (map.isBarrier(y, x)) return false;

      z = round(y - (abs(e)+thickness));
      if (z >= minY) if (map.isBarrier(z, x)) return false;

      z = round(y + (abs(e)+thickness));
      if (z <= maxY) if (map.isBarrier(z, x)) return false;
    }

    e = e + de;
    if (e >= 0.5)
    {
      y = y + ystep;
      e -= 1.0f;
    }
  }
        
  // If we get here, then a clear path exists between thisnode and node
  return true;
}

void backgroundThread(void* arg)
{
  Level::ThreadArgs* args = (Level::ThreadArgs*)(arg);

  int width = args->map.width();
  int height = args->map.height();

  int startIndex = args->map.index(args->entrance.row, args->entrance.col);
  int exitIndex = args->map.index(args->exit.row, args->exit.col);
  int goalIndex = args->map.index(args->cores.row, args->cores.col);

  ////////////////////////////////////////////////////
  //          POSSIBLY REMOVE ALL TOWERS            //
  ////////////////////////////////////////////////////
  if (args->avoidTowers == false)
  {
    // If the args say not to avoid towers, replace all closed tower bases
    // with open ones.  This should only be the case when there is no path
    // to the data or exit
    for (int row = 0; row < height; ++row)
      for (int col = 0; col < width; ++col)
        if (args->map[row][col] == TOWER_BASE_CLOSED) args->map[row][col] = TOWER_BASE_OPEN;
  }
  
  // Add the starting point, exist point, and goal point to the graph
  Graph graph;
  graph.addNode(new Node2D(startIndex, args->entrance.row, args->entrance.col));
  graph.addNode(new Node2D(exitIndex, args->exit.row, args->exit.col));
  graph.addNode(new Node2D(goalIndex, args->cores.row, args->cores.col));

  ////////////////////////////////////////////////////
  //        LOCATE ALL CORNER PATH POINTS           //
  ////////////////////////////////////////////////////
  for (int row = 0; row < height; ++row)
  {
    for (int col = 0; col < width; ++col)
    {
      // Skip cells that are barriers
      if (args->map.isBarrier(row, col)) continue;

      // Build the mask for what the area around this cell looks like
      //
      //   1   2   4
      //   8   X   16
      //   32  64 128 
      //
      short mask = 0;
      if (args->map.isBarrier(row-1, col-1))  mask |= 1;
      if (args->map.isBarrier(row-1, col))    mask |= 2;
      if (args->map.isBarrier(row-1, col+1))  mask |= 4;
      if (args->map.isBarrier(row,   col-1))  mask |= 8;
      if (args->map.isBarrier(row,   col+1))  mask |= 16;
      if (args->map.isBarrier(row+1, col-1))  mask |= 32;
      if (args->map.isBarrier(row+1, col))    mask |= 64;
      if (args->map.isBarrier(row+1, col+1))  mask |= 128;

      // Look for open corners around this cell. 
      static short mask_11 = 11;
      static short mask_22 = 22;
      static short mask_104 = 104;
      static short mask_208 = 208;

      if ((mask & mask_11) == 1   || (mask & mask_22) == 4 ||
          (mask & mask_104) == 32 || (mask & mask_208) == 128)
      {
        args->map[row][col] = CORNER;

        int index = args->map.index(row, col);
        graph.addNode(new Node2D(index, row, col));
      }
    }
  }

  // :TEMP: Print out what the map looks like for debuging
  std::ofstream out;
  out.open("map.txt");
  out << "Initial Map:" << std::endl;
  for (int row = height-1; row >= 0; --row)
  {
    for (int col = 0; col < width; ++col)
    {
      int val = args->map[row][col];
      if (val == WALL)        out << "X";
      else if (val == CORNER) out << "o";
      else if (val == TOWER_BASE_CLOSED) out << "T";
      else                    out << " ";
    }
    out << std::endl;
  }
  out << std::endl;
  out.flush();
  out.close();
  // :TEMP:

  ////////////////////////////////////////////////////
  //    FIGURE OUT WHICH NODES CAN SEE OTHERS       //
  ////////////////////////////////////////////////////
  NodeMapItr itr1 = graph.getIterator();
  while (itr1.hasMoreElements())
  {
    Node2D* node1 = static_cast<Node2D*>(itr1.getNext()); // This advances the iterator

    NodeMapItr itr2 = itr1;  // Can itr2 always be the itr after itr1??
    while (itr2.hasMoreElements())
    {
      Node2D* node2 = static_cast<Node2D*>(itr2.getNext()); // This advances the iterator

      // Ignore the same index
      if (node1->getIndex() == node2->getIndex()) continue;

      // See if these two nodes have line of sight to each other
      if (isInLos(args->map, node1->getPosition(), node2->getPosition()))
      {
        // If so, create an edge between these two
        graph.addEdge(node1->getIndex(), node2->getIndex());
      }
    }
  }

  ////////////////////////////////////////////////////
  //              PERFORM A* SEARCH                 //
  ////////////////////////////////////////////////////
  if (args->genDefaultPath)
  {
    Path pathToData = AStar(graph, startIndex, goalIndex);
    Path pathToExit = AStar(graph, goalIndex, exitIndex);
    if (pathToData.empty() || pathToExit.empty())
    {
      std::cout << "No default path found!" << std::endl;
      if (args->avoidTowers)
      {
        args->avoidTowers = false;
        backgroundThread(args);
      }
      return;

      // :TODO: Remove all tower obstacles and plan again to get A path
    }
    else
    {
      args->level->notifyDefaultPaths(pathToData, pathToExit);
    }
  }

  // These maps will hold paths that have already been created from a specific
  // node, indicated by the index (key), to either the cores or the exit
  std::map<int, Path> pathsToCores;
  std::map<int, Path> pathsToExit;

  // Now perform AStar for each Alien
  std::map<unsigned int, std::pair<Ogre::Vector3, Level::ThreadArgs::DESTINATION> >::iterator itr = args->alienPositions.begin();
  while (itr != args->alienPositions.end())
  {
    unsigned int alienID = itr->first;
    const Ogre::Vector3& alienPos = itr->second.first;
    Level::ThreadArgs::DESTINATION dest = itr->second.second;

    // Create a Node2D where the alien is currently at
    GridNode alienStart = gridNodeFromPos(itr->second.first);
    int alienIndex = args->map.index(alienStart.row, alienStart.col);

    // Determine if this node is a barrier
    if (args->map.isBarrier(alienStart.row, alienStart.col))
    {
      GridNode newNode = alienStart;
      Ogre::Vector3 center = centerPosFromCoords(newNode);
      float dz = center.z - alienPos.z;   // Positive dz should be above the center
      float dx = alienPos.x - center.x;   // Positive dx should be right of the center

      // Move to the nearest row or column.  If that is also a barrier, move
      // in the other direction instead.
      if (abs(dz) > abs(dx))
      {
        newNode.col = alienStart.col;
        newNode.row = alienStart.row + sign(dz);
        if (args->map.isBarrier(newNode.row, newNode.col))
        {
          newNode.row = alienStart.row;
          newNode.col = alienStart.col + sign(dx);
        }
      }
      else
      {
        newNode.row = alienStart.row;
        newNode.col = alienStart.col + sign(dx);
        if (args->map.isBarrier(newNode.row, newNode.col))
        {
          newNode.col = alienStart.col;
          newNode.row = alienStart.row + sign(dz);
        }
      }

      std::cout << "Moving alien start from (" << alienStart.row << ", " << alienStart.col << ") to (" <<
                                                  newNode.row << ", " << newNode.col << ")" << std::endl;
      alienStart = newNode;
    }

    // First see if a path from this node to the cores or exit (depending on this alien's destination) already exists
    if (dest == Level::ThreadArgs::CORES && pathsToCores.find(alienIndex) != pathsToCores.end())
    {
      // Path to cores
      args->level->notifyPath(alienID, pathsToCores[alienIndex]);
    }
    else if (dest == Level::ThreadArgs::EXIT && pathsToExit.find(alienIndex) != pathsToExit.end())
    {
      // Path to exit
      args->level->notifyPath(alienID, pathsToExit[alienIndex]);
    }
    else
    {
      Node2D* alienNode = NULL;
      // Only create the alien node if there's not currently a node on the graph here
      if (graph[alienIndex] == NULL)
      {
        alienNode = new Node2D(alienIndex, alienStart.row, alienStart.col);
        graph.addNode(alienNode);

        // Do all the LOS stuff.    
        NodeMapItr graphItr = graph.getIterator();
        while (graphItr.hasMoreElements())
        {
          Node2D* node2 = static_cast<Node2D*>(graphItr.getNext()); // This advances the iterator

          // Ignore the same index
          if (alienIndex == node2->getIndex()) continue;

          // See if these two nodes have line of sight to each other
          if (isInLos(args->map, alienStart, node2->getPosition()))
          {
            // If so, create an edge between these two
            graph.addEdge(alienIndex, node2->getIndex());
          }
        }
      }

      // Do the A* thingy
      int alienGoal = (dest == Level::ThreadArgs::CORES) ? (goalIndex) : (exitIndex);
      Path alienPath = AStar(graph, alienIndex, alienGoal);

      // Pass this path back to the Level
      if (!alienPath.empty())
      {
        // :TODO: Handle paths not found for the alien
        if (dest == Level::ThreadArgs::CORES) pathsToCores[alienIndex] = alienPath;
        else                                  pathsToExit[alienIndex] = alienPath;
        args->level->notifyPath(alienID, alienPath);
      }
      else
      {
        std::cout << "No path found for alien at (" << alienStart.row << ", " << alienStart.col << ")" << std::endl;
      }

      // Remove the alienNode from the graph if it was created
      if (alienNode) graph.removeNode(alienNode);
    }

    // Move to the next alien position
    itr = args->alienPositions.erase(itr);
  } // while(itr)

  // Tell the Graph it can destroy all its nodes now
  graph.clearNodes();

  delete args;
}