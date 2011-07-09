#pragma once
#include "Tower.h"

class TowerBase;

class TowerBarrier : public Tower
{
public:
  TowerBarrier(TowerBase* parent);
  ~TowerBarrier(void);

  // Inherited from Tower
  void upgrade(void);
  void sell(void);
  bool createGraphics(void);
  void update(float t);
};