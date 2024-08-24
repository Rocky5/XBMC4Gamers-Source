#pragma once

#include "GUIDialog.h"

class CGUIDialogOverlay : public CGUIDialog
{
public:
  CGUIDialogOverlay(void);
  virtual ~CGUIDialogOverlay(void);
  virtual bool OnMessage(CGUIMessage& message);
protected:
};
