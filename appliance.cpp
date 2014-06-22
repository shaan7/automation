#include "appliance.h"
#include "radio.h"

Appliance::Appliance(int applianceNumber, Location* location, Radio* radio, Wt::WObject* parent)
    : WObject(parent)
    , mApplianceNumber(applianceNumber)
    , mLocation(location)
    , mRadio(radio)
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

bool Appliance::activate()
{
    return mRadio->activateAppliance(this);
}

bool Appliance::deactivate()
{
    return mRadio->deactivateAppliance(this);
}

void Appliance::toggle()
{
    if (mActive)
        deactivate();
    else
        activate();
}

void Appliance::setActive(bool active)
{
    if (mActive != active) {
        mActive = active;

        if (mActive) {
            activated.emit();
        } else {
            deactivated.emit();
        }
    }
}

bool Appliance::sync()
{
    return mActive ? activate() : deactivate();
}
