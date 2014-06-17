#include "appliance.h"
#include "radio.h"

Appliance::Appliance(int applianceNumber, Location* location, Wt::WObject* parent)
    : WObject(parent)
    , mApplianceNumber(applianceNumber)
    , mLocation(location)
{
}

int Appliance::applianceNumber() const
{
    return mApplianceNumber;
}

Location* Appliance::location() const
{
    return mLocation;
}

void Appliance::activate()
{
    Radio::instance()->activateAppliance(this);
}

void Appliance::deactivate()
{

}

void Appliance::toggle()
{

}

void Appliance::setActive(bool active)
{
    if (mActive != active) {
        mActive = active;
    }
}
