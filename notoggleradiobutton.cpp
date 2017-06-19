#include "notoggleradiobutton.h"

NoToggleRadioButton::NoToggleRadioButton(QGroupBox* grp): QRadioButton(grp)
{
    this->setChecked(true);
}
void NoToggleRadioButton::nextCheckState()   {
    //do nothing
}
