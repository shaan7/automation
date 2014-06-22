#ifndef APPLIANCE_H
#define APPLIANCE_H

#include <Wt/WObject>
#include <Wt/WSignal>

class Radio;
class Location;

class Appliance : public Wt::WObject
{
public:
    Appliance(int applianceNumber, Location* location, Radio* radio, WObject* parent = 0);

    int applianceNumber() const;
    Location *location() const;

    bool activate();
    bool deactivate();
    void toggle();
    bool sync();

    Wt::Signal<void> activated;
    Wt::Signal<void> deactivated;

    void setActive(bool active);

private:
    int mApplianceNumber;
    Location *mLocation;
    Radio *mRadio;
    bool mActive = false;
};

#endif // APPLIANCE_H
