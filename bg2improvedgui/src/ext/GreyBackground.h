#include "uibutton.h"
#include "chitin.h"

void GreyBackground_asm();

class CUIButtonGreyBackground : public CUIButton {
public:
    CUIButtonGreyBackground(CPanel& panel, ChuFileControlInfoBase& controlInfo);

    virtual ~CUIButtonGreyBackground(); //v0
    virtual void OnLClicked(POINT pt); //v5c
    virtual BOOL Redraw(BOOL bForceRedraw);
};