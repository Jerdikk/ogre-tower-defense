#include "Label.h"
#include <OgreStringConverter.h>
#include <OgreOverlayManager.h>
#include <OgreTextAreaOverlayElement.h>
#include <OgreBorderPanelOverlayElement.h>
#include "Window.h"

namespace GUI
{
  int Label::msCounter = 0;

  class Label::p_Label
  {
  public:
    p_Label(Window* _parent) : parent(_parent), textArea(NULL), border(NULL) {}
    ~p_Label() {Ogre::OverlayManager::getSingleton().destroyOverlayElement(border);}

    Window* parent;
    Ogre::String text;  // Displayed text may be different
    Ogre::TextAreaOverlayElement* textArea;
    Ogre::BorderPanelOverlayElement* border;
  };


  Label::Label(Window* parent)
    : p(new p_Label(parent))
  {
    _create();
  }

  Label::Label(const Ogre::String& text, Window* parent)
    : p(new p_Label(parent))
  {
    _create();
    setText(text);
  }

  Label::Label(const Ogre::String& text, const Ogre::String& displayText, Window* parent)
    : p(new p_Label(parent))
  {
    _create();
    setText(text);
    setDisplayText(displayText);
  }

  Label::~Label(void)
  {
    p->parent->removeWidget(this);
    delete p;
  }

  void* Label::getContainer()
  {
    return (void*)p->border;
  }

  Ogre::String Label::getName()
  {
    return p->border->getName();
  }

  void Label::activate(void)
  {
    p->textArea->setColour(mColors[ACTIVATED]);
  }

  void Label::deactivate(void)
  {
    if (mEnabled) p->textArea->setColour(mColors[TEXT]);
    else          p->textArea->setColour(mColors[DISABLED]);
  }

  void Label::setSize(int width, int height)
  {
    p->textArea->setDimensions(width, height);
    p->border->setDimensions(width, height);
  }

  void Label::setPosition(int x, int y)
  {
    //mpTextArea->setPosition(x+5, y + (0.5 * (mpTextArea->getHeight()-mpTextArea->getCharHeight())));
    p->border->setPosition(x, y);
  }

  void Label::setText(const Ogre::String& text)
  {
    p->textArea->setCaption(text);
    p->text = text;
  }

  void Label::setDisplayText(const Ogre::String& text)
  {
    p->textArea->setCaption(text);
  }

  void Label::setVisible(bool show)
  {
    if (show) p->textArea->show();
    else      p->textArea->hide();
  }

  void Label::setBorderVisible(bool show)
  {
    //if (show)   ;
    //else        ;
  }

  void Label::setEnabled(bool enabled)
  {
    mEnabled = enabled;

    if (mEnabled) p->textArea->setColour(mColors[TEXT]);
    else          p->textArea->setColour(mColors[DISABLED]);
  }

  Ogre::String Label::getText(void) const
  {
    return p->text;
  }

  Ogre::Vector2 Label::getSize(void) const
  {
    return Ogre::Vector2(p->textArea->getWidth(), p->textArea->getHeight());
  }

  Ogre::Vector2 Label::getPosition(void) const
  {
    return Ogre::Vector2(p->textArea->getLeft(), p->textArea->getTop());
  }

  bool Label::isVisible(void) const
  {
    return p->textArea->isVisible();
  }

  bool Label::isBorderVisible(void) const
  {
    return true;
  }

  bool Label::isEnabled(void) const
  {
    return mEnabled;
  }

  void Label::_create(void)
  {
    Ogre::String strId = Ogre::StringConverter::toString(msCounter++);
    mFontSize = 14;

    // Default colors
    mColors[BACKGROUND] = Ogre::ColourValue(0, 0, 0);
    mColors[TEXT]       = Ogre::ColourValue(0.5, 0.7, 0.5);
    mColors[ACTIVATED]  = Ogre::ColourValue(1, 1, 0.7);
    mColors[DISABLED]   = Ogre::ColourValue(0.5, 0.5, 0.5);

    Ogre::OverlayManager& overlayManager = Ogre::OverlayManager::getSingleton();

    // Create border area
    p->border = static_cast<Ogre::BorderPanelOverlayElement*>(overlayManager.createOverlayElement("BorderPanel", "LabelBorder_" + strId));
    p->border->setMetricsMode(Ogre::GMM_PIXELS);
    p->border->setPosition(0, 0);
    p->border->setDimensions(100, 20);
    p->border->setMaterialName("Core/StatsBlockCenter");
    p->border->setBorderMaterialName("Core/StatsBlockBorder");
    p->border->setBorderSize(2, 2, 1, 1);
    p->border->setTopLeftBorderUV      (0.0000, 1.0000, 0.0039, 0.9961);
	  p->border->setTopBorderUV          (0.0039, 1.0000, 0.9961, 0.9961);
	  p->border->setTopRightBorderUV     (0.9961, 1.0000, 1.0000, 0.9961);
	  p->border->setLeftBorderUV         (0.0000, 0.9961, 0.0039, 0.0039);
	  p->border->setRightBorderUV        (0.9961, 0.9961, 1.0000, 0.0039);
	  p->border->setBottomLeftBorderUV   (0.0000, 0.0039, 0.0039, 0.0000);
    p->border->setBottomBorderUV       (0.0039, 0.0039, 0.9961, 0.0000);
    p->border->setBottomRightBorderUV  (0.9961, 0.0039, 1.0000, 0.0000);
    p->parent->addWidget(this);
    
    // Create a text area
    p->textArea = static_cast<Ogre::TextAreaOverlayElement*>(overlayManager.createOverlayElement("TextArea", "Label_" + strId));
    p->textArea->setMetricsMode(Ogre::GMM_PIXELS);
    p->textArea->setPosition(5, 3);
    p->textArea->setDimensions(98, 18);
    p->textArea->setCharHeight(mFontSize);
    p->textArea->setFontName("BlueHighway");
    p->textArea->setColour(mColors[TEXT]);
    p->border->addChild(p->textArea);

    // Default values
    mEnabled = true;
  }
} // namespace GUI