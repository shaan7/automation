#include "location.h"

#include "appliance.h"

Location::Location(int sensorId, Wt::WObject* parent)
    : WObject(parent)
    , mSensorId(sensorId)
{
}

std::unordered_map<int, Appliance*> Location::initAppliances(
    std::vector< int > applianceNumbers)
{
    for (auto number : applianceNumbers) {
        mAppliances.insert(std::make_pair(number, new Appliance(number, this, this)));
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
}
