#pragma once
#include "Alien.h"

class AlienDrone : public Alien
{
public:
  AlienDrone(void);
  ~AlienDrone(void);

  // Inherited from Alien
  bool createGraphics(const GridNode& startPos);
};
