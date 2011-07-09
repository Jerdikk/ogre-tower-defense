#pragma once
#include "Node2D.h"

typedef std::pair<int, int> Edge;
typedef std::vector<Edge> EdgeList;

class Graph
{
public:
  Graph(void);
  ~Graph(void);

  void addNode(Node2D* node);
  void addNodes(const NodeList& nodes);
  void removeNode(Node2D* node);
  void addEdge(int node1, int node2);
  void addEdge(const Edge& edge);
  void addEdges(const EdgeList& edges);
  void clearNodes(void);

  Node2D* operator [] (int index);
  NodeMapItr getIterator(void);

private:
  NodeMap mNodes;
};