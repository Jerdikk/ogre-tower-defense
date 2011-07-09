#include "Graph.h"

Graph::Graph(void) 
{
}

Graph::~Graph(void) 
{
}

void Graph::addNode(Node2D* node)
{
  mNodes[node->getIndex()] = node;
}

void Graph::addNodes(const NodeList& nodes)
{
  mNodes.clear();
  for (size_t i = 0; i < nodes.size(); ++i)
    addNode(nodes[i]);
}

void Graph::removeNode(Node2D* node)
{
  // Remove this node from all its neighbors
  NodeMapItr itr = node->getIterator();
  while (itr.hasMoreElements())
  {
    Node2D* neighbor = itr.getNext();
    if (neighbor != node) neighbor->removeNeighbor(node);
  }

  // Remove this node from the map
  mNodes.erase(mNodes.find(node->getIndex()));
}

void Graph::addEdge(int node1, int node2)
{
  addEdge(Edge(node1, node2));
}

void Graph::addEdge(const Edge& edge)
{
  NodeMap::iterator node1 = mNodes.find(edge.first);
  NodeMap::iterator node2 = mNodes.find(edge.second);

  if (node1 != mNodes.end() && node2 != mNodes.end())
  {
    node1->second->addNeighbor(node2->second);
    node2->second->addNeighbor(node1->second);
  }
}

void Graph::addEdges(const EdgeList& edges)
{
  for (size_t i = 0; i < edges.size(); ++i)
    addEdge(edges[i]);
}

Node2D* Graph::operator [] (int index)
{
  NodeMap::iterator itr = mNodes.find(index);
  if (itr != mNodes.end()) return itr->second;
  return NULL;
}

void Graph::clearNodes(void)
{
  NodeMap::iterator itr = mNodes.begin();
  for (; itr != mNodes.end(); ++itr)
    delete itr->second;
  mNodes.clear();
}

NodeMapItr Graph::getIterator(void)
{
  return NodeMapItr(mNodes.begin(), mNodes.end());
}