#ifndef MACHINESETTINGS_H
#define MACHINESETTINGS_H
#include "notoggleradiobutton.h"
#include "perfectio.h"
#include <QCheckBox>
#include <QWidget>
#include <QRadioButton>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QButtonGroup>
#include <QLineEdit>
#include <QLabel>
namespace Ui {
class MachineSettings;
}

class MachineSettings : public QWidget
{
    Q_OBJECT

public:
    std::vector<QRadioButton*> typeSelector;
    QGroupBox* selectorBox;
    QButtonGroup* typeSelectorGroup;
    QTableWidget* speedConfigTable;
    int type;
    explicit MachineSettings(QWidget *parent = 0, int _type = 3);
    ~MachineSettings();
    void setIconForType(QString path, int i);
    void setBonuses(double intProd = 0, double intSpd = 0, double defProd = 0, double defSpd = 0);
    virtual machine_config getMachineConfig();
signals:
    void dataChanged();
private slots:
    void lineTextEdited(QString);
    void radioChanged(bool);
private:
    Ui::MachineSettings *ui;


};
class MiningSettings : public MachineSettings
{
    Q_OBJECT

public:
    explicit MiningSettings(QWidget* parent = 0);
    virtual machine_config getMachineConfig() override;
private:
};
class SmeltingSettings : public MachineSettings
{
    Q_OBJECT
public:
    explicit SmeltingSettings(QWidget* parent = 0);

    virtual machine_config getMachineConfig() override;
};
class PumpjackSettings : public MachineSettings
{
    Q_OBJECT
public:
    QLineEdit* oilRate;
    explicit PumpjackSettings(QWidget* parent = 0);
private:

};
class RefinerySettings: public MachineSettings
{
    Q_OBJECT
public:
    QGroupBox* processSelector;
    std::vector<QRadioButton*> processes;
    QButtonGroup* processSelectorGroup;
    QCheckBox* crackBox;
    explicit RefinerySettings(QWidget* parent = 0);
private:

};
class ChemistrySettings: public MachineSettings
{
    Q_OBJECT
public:
    QGroupBox* fuelSelector;
    std::vector<QRadioButton*>fuels;
    QButtonGroup* fuelSelectorGroup;
    explicit ChemistrySettings(QWidget* parent = 0);
    virtual machine_config getMachineConfig() override;
private:

};
class AssemblingSettings : public MachineSettings
{
    Q_OBJECT
public:
    explicit AssemblingSettings(QWidget* parent = 0);
    virtual machine_config getMachineConfig() override;
};
class RocketrySettings: public MachineSettings
{
    Q_OBJECT
public:
    explicit RocketrySettings(QWidget* parent = 0);
};

#endif // MACHINESETTINGS_H
