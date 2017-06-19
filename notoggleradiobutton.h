#ifndef NOTOGGLERADIOBUTTON_H
#define NOTOGGLERADIOBUTTON_H
#include <QRadioButton>
#include <QGroupBox>

class NoToggleRadioButton : public QRadioButton
{
public:
    NoToggleRadioButton(QGroupBox* grp = nullptr);
    virtual void nextCheckState() override;
};

#endif // NOTOGGLERADIOBUTTON_H
