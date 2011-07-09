#pragma once
#include <vector>
#include <OgrePrerequisites.h>
#include <OgreVector2.h>
#include "Widget.h"

// GUI PREDEFINES

namespace Ogre {
  class OverlayContainer;
  class TextAreaOverlayElement;
  class BorderPanelOverlayElement;
}

namespace GUI {
  class Window;
  class Label;
  class LabelGroup;

  typedef std::map<Ogre::String, Widget*> Widgets;
  typedef std::vector<Label*> Labels;
}