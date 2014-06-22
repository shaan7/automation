#include "appliance.h"
#include "radio.h"

#include <Wt/WTimer>
// #include <Wt/WApplication>

#include <iostream>

namespace {
static const int RETRY_TIMEOUT = 3000;
}

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
    mExpectedActiveState = true;
    Wt::WTimer::singleShot(RETRY_TIMEOUT, this, &Appliance::retryIfNeeded);
    return mRadio->activateAppliance(this);
}

bool Appliance::deactivate()
{
    mExpectedActiveState = false;
    Wt::WTimer::singleShot(RETRY_TIMEOUT, this, &Appliance::retryIfNeeded);
    return mRadio->deactivateAppliance(this);
}

bool Appliance::toggle()
{
    return mActive ? deactivate() : activate();
}

void Appliance::setActive(bool active)
{
    if (mActive != active) {
//         std::cout << Wt::WApplication::instance()->sessionId()
//             << " UPDATING APPLIANCE " << mLocation->sensorId()
//             << " " << mApplianceNumber << " " << active << std::endl;
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

void Appliance::retryIfNeeded()
{
    if (mActive != mExpectedActiveState) {
        std::cout << "Retrying location " << location()->sensorId()
                  << " appliance " << mApplianceNumber
                  << " to " << mExpectedActiveState << std::endl;

        if (mExpectedActiveState) {
            activate();
        } else {
            deactivate();
        }
    }
}
