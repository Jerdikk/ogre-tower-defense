#pragma once
#include "Alien.h"

class AlienTank : public Alien
{
public:
  AlienTank(void);
  ~AlienTank(void);

  // Inherited from Alien
  bool createGraphics(const GridNode& startPos);
};
