#pragma once
#include "Alien.h"

class AlienSwarmer : public Alien
{
public:
  AlienSwarmer(void);
  ~AlienSwarmer(void);

  // Inherited from Alien
  bool createGraphics(const GridNode& startPos);
};
