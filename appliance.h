#ifndef APPLIANCE_H
#define APPLIANCE_H

#include <Wt/WObject>
#include <Wt/WSignal>

class Location;

class Appliance : public Wt::WObject
{
public:
    Appliance(int applianceNumber, Location* location, WObject* parent = 0);

    int applianceNumber() const;
    Location *location() const;

    void activate();
    void deactivate();
    void toggle();

    Wt::Signal<void> activated;
    Wt::Signal<void> deactivated;

    void setActive(bool active);

private:
    int mApplianceNumber;
    Location* mLocation;
    bool mActive = false;
};

#endif // APPLIANCE_H
