#include "LabelGroup.h"
#include "Label.h"
#include <OgreStringConverter.h>
#include <OIS\OIS.h>

namespace GUI
{
  LabelGroup::LabelGroup(void)
  {
    mCurrentIndex = -1;
  }

  LabelGroup::~LabelGroup(void)
  {
  }

  void LabelGroup::addLabel(Label* label, int index)
  {
    addLabel('1' + mLabels.size(), label, index);
  }

  void LabelGroup::addLabel(char key, Label* label, int index)
  {
    // Make sure this Label is not already part of this group
    for (size_t i = 0; i < mLabels.size(); ++i)
      if (mLabels[i] == label) return;
    
    // Add the label to the end
    if (index < 0 || index > (int)mLabels.size())
    {
      index = mLabels.size();
      mLabels.push_back(label);
    }

    // Insert it into the desired location
    else
      mLabels.insert(mLabels.begin() + index, label);

    // Update the display text for the label to include its index
    Ogre::String strIdx = _getHotKeyLabel(key);
    label->setDisplayText(strIdx + label->getText());

    // Update the current index if this is the first Label added
    if (mLabels.size() == 1)
    {
      mCurrentIndex = 0;
      label->activate();
    }
  }

  Label* LabelGroup::getLabel(int index)
  {
    Label* label = NULL;

    // Make sure the index is valid
    if (index >= 0 && index < (int)mLabels.size()) 
      label = mLabels[index];

    return label;
  }

  void LabelGroup::destroyAllLabels(void)
  {
    // Delete all Labels
    for (size_t i = 0; i < mLabels.size(); ++i)
      delete mLabels[i];

    // Clear the group
    clear();
  }

  void LabelGroup::cycleUp(void)
  {
    //setCurrentLabel(mCurrentIndex-1);
    int index = mCurrentIndex;
    while (index >= 0)
    {
      if (setCurrentLabel(--index)) break;
    }
  }

  void LabelGroup::cycleDown(void)
  {
    int index = mCurrentIndex;
    while (index < (int)mLabels.size())
    {
      if (setCurrentLabel(++index)) break;
    }
  }

  bool LabelGroup::setCurrentLabel(Label* label)
  {
    bool set = false;

    for (size_t i = 0; i < mLabels.size(); ++i)
    {
      if (mLabels[i] == label)
      {
        // We found the Label.  Only update if it's not the 
        // current active Label
        if (i != mCurrentIndex)
        {
          // Deactivate the current label
          mLabels[mCurrentIndex]->deactivate();
          
          // Activate this Label
          mLabels[i]->activate();

          // Update the current index
          mCurrentIndex = (int)i;

          set = true;
          break;
        }
      }
    }

    return set;
  }

  bool LabelGroup::setCurrentLabel(int index)
  {
    bool set = false;

    if (index >= 0 && index < (int)mLabels.size() && index != mCurrentIndex && mLabels[index]->isEnabled())
    {
      // Deactivate the current label
      mLabels[mCurrentIndex]->deactivate();
      
      // Activate this Label
      mLabels[index]->activate();

      // Update the current index
      mCurrentIndex = index;

      set = true;
    }

    return set;
  }

  Label* LabelGroup::getCurrentLabel(void)
  {
    return mLabels[mCurrentIndex];
  }

  void LabelGroup::show(void)
  {
    // Loop through all Labels, setting their visibility to true
    for (size_t i = 0; i < mLabels.size(); ++i)
      mLabels[i]->setVisible(true);
  }

  void LabelGroup::hide(void)
  {
    // Loop through all Labels, setting their visibility to false
    for (size_t i = 0; i < mLabels.size(); ++i)
      mLabels[i]->setVisible(false);
  }

  bool LabelGroup::isVisible(void)
  {
    bool visible = false;

    // Not very sophisticated right now...
    if (mLabels.size() > 0 && mLabels.front()->isVisible()) visible = true;

    return visible;
  }

  void LabelGroup::clear(void)
  {
    mLabels.clear();
    mCurrentIndex = -1;
  }

  Ogre::String LabelGroup::_getHotKeyLabel(char key)
  {
    Ogre::String retval;
    OIS::KeyCode keyCode = static_cast<OIS::KeyCode>(key);

    switch (keyCode)
    {
      case OIS::KC_ESCAPE: 
        retval = "Esc";
        break;

      case OIS::KC_0:
        retval = '0';
        break;

      case OIS::KC_1:
      case OIS::KC_2:
      case OIS::KC_3:
      case OIS::KC_4:
      case OIS::KC_5:
      case OIS::KC_6:
      case OIS::KC_7:
      case OIS::KC_8:
      case OIS::KC_9:
        retval = '1' + (keyCode - OIS::KC_1);
        break;

      default:
        if (keyCode >= OIS::KC_A && keyCode <= OIS::KC_Z)
          retval = 'A' + (keyCode - OIS::KC_A);
        else
          retval = keyCode; // undeterministic results
    } // end switch

    return retval + ": ";
  }
} // namespace GUI