#pragma once
#include "gui_predef.h"

namespace GUI
{
  // A LabelGroup has no graphical representation of its own, but 
  // rather associates a group of Labels together.  The owner of the
  // Labels is still responsible for creating them, positioning then,
  // editing them and destroying them, but by using the LabelGroup you 
  // can ensure that only one Label is ever selected at a time, and you 
  // can cycle forward and backward through the labels.
  // The LabelGroup does not own the labels, so it will not inherintly
  // try to destroy them at any point, but by calling destroyAllLabels()
  // you can force the LabelGroup to destroy all Labels that it currently
  // contains.
  class LabelGroup
  {
  public:
    LabelGroup(void);
    ~LabelGroup(void);

    // Add a Label to the list
    void addLabel(Label* label, int index = -1);

    // Add a Label to the list with a hot ket value
    void addLabel(char key, Label* label, int index = -1);

    // Get a Label at the specified index
    Label* getLabel(int index);

    // Destroy all Labels in this group
    void destroyAllLabels(void);

    // Set the Label above the current one to be the active one
    void cycleUp(void);

    // Set the Label below the current one to be the active one
    void cycleDown(void);

    // Set the specified Label to be the current active one.  Returns
    // true if the desired Label is set
    bool setCurrentLabel(Label* label);
    bool setCurrentLabel(int index);

    // Returns the current active Label
    Label* getCurrentLabel(void);

    // Shows all Labels in the group.
    void show(void);

    // Hides all labels in the group.
    void hide(void);

    // Returns true if the Labels in this group are currently visible
    bool isVisible(void);

    // Remove all Labels from the group.  Does not destroy them
    void clear(void);

  protected:
    Ogre::String _getHotKeyLabel(char key);

  protected:
    Labels mLabels;
    int mCurrentIndex;
  };
} // namespace GUI