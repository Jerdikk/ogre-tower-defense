#pragma once
#include <vector>
#include "GridNode.h"
#include "common.h"


// Stores the map data and provides convenient access functions
class Map
{
public:
  Map(void);
  Map(const Map&);
  ~Map(void);

  class bracket {
  public:
    bracket(IntVector* data, int offset) : _data(data), _offset(offset) {}
    int& operator [] (int col) {
      return _data->operator [](_offset + col);
    }
  private:
    int _offset;
    IntVector* _data;
  };

  void clear();
  void resize(int width, int height);
  bool isValid(int row, int col) const;
  bool isBarrier(int row, int col) const;
  int width(void) const {return w;}
  int height(void) const {return h;}
  int index(int row, int col) const;
  GridNode getRowCol(int index) const;

  int getValue(int index) const;              // DOES NOT VALIDATE
  int getValue(int row, int col) const;       // DOES NOT VALIDATE
  void setValue(int row, int col, int val);   // DOES NOT VALIDATE
  bracket operator [] (int row);              // DOES NOT VALIDATE

  

private:
  int w;
  int h;
  IntVector data;
};
