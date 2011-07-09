#include "TowerBarrier.h"

TowerBarrier::TowerBarrier(TowerBase* parent)
  : Tower(parent, "Barrier", DEFENSIVE)
{
}

TowerBarrier::~TowerBarrier(void)
{
}

void TowerBarrier::upgrade(void)
{
}

void TowerBarrier::sell(void)
{
}

bool TowerBarrier::createGraphics(void)
{
  bool retval = false;

  if (mpsSceneMgr)
  {
    mMaterialName = "Material/Tower/Barrier";

    // Create simple graphics
    retval = createSimpleGraphics(mStrUid);
  }

  return retval;
}

void TowerBarrier::update(float t)
{
}