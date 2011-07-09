#include "Map.h"

Map::Map(void)
{
  w = 0;
  h = 0;
}

Map::Map(const Map& rhs)
{
  resize(rhs.w, rhs.h);
  std::copy(rhs.data.begin(), rhs.data.end(), data.begin());
}

Map::~Map(void)
{
}

void Map::clear()
{
  w = 0;
  h = 0;
  data.clear();
}

void Map::resize(int width, int height)
{
  w = width;
  h = height;
  data.clear();
  data.resize(width*height, 0);
}

bool Map::isValid(int row, int col) const
{
  return (row >= 0 && col >= 0 && index(row, col) < (int)data.size());
}

bool Map::isBarrier(int row, int col) const
{
  if (!isValid(row, col)) return true;
  int val = getValue(row, col);

  // Return true if this node is not any of the things which are not barriers
  return (val != TOWER_BASE_OPEN && val != OPEN && val != POWER_CORE && val != CORNER);
}

int Map::getValue(int index) const
{
  return data[index];
}

int Map::getValue(int row, int col) const
{
  return data[index(row, col)];
}

void Map::setValue(int row, int col, int val)
{
  data[index(row, col)] = val;
}

Map::bracket Map::operator [] (int row)
{
  return bracket(&data, row*w);
}

int Map::index(int row, int col) const
{
  return (row*w + col);
}

GridNode Map::getRowCol(int index) const
{
  GridNode node;
  node.row = index/w;
  node.col = index - (int)node.row*w;
  return node;
}