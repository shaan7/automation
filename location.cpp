#include "location.h"
#include "appliance.h"
#include <Wt/WApplication>

#include <iostream>

Location::Location(std::string sessionId, int sensorId, Radio* radio, Wt::WObject* parent)
    : WObject(parent)
    , mSessionId(sessionId)
    , mSensorId(sensorId)
    , mRadio(radio)
{
}

std::unordered_map<int, Appliance*> Location::initAppliances(
    std::vector< int > applianceNumbers)
{
    for (auto number : applianceNumbers) {
        mAppliances.insert(std::make_pair(number, new Appliance(number, this, mRadio, this)));
    }

    return mAppliances;
}

std::unordered_map<int, Appliance*> Location::appliances() const
{
    return mAppliances;
}

int Location::sensorId() const
{
    return mSensorId;
}

void Location::applianceUpdate(int pinNumber, int value)
{
    try {
        Appliance *appliance = mAppliances.at(pinNumber);
        if (appliance) {
            appliance->setActive(value == 1);
        }
    } catch (std::out_of_range e) {
//         std::cerr << "Could not find appliance " << pinNumber << " in location " << mSensorId << std::endl;
    }

    Wt::WApplication::instance()->triggerUpdate();
}

bool Location::configured() const
{
    return mConfigured;
}

void Location::setConfigured(bool configured)
{
    if (mConfigured == configured) {
        std::cerr << "Location " << sensorId() << " is already configured" << std::endl;
    } else {
        mConfigured = configured;
        if (mConfigured) {
            for (auto appliance : mAppliances) {
                appliance.second->sync();
            }
        }
    }
}

std::string Location::sessionId() const
{
    return mSessionId;
}
