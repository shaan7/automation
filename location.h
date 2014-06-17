#ifndef LOCATION_H
#define LOCATION_H

#include <Wt/WObject>

#include <vector>
#include <unordered_map>

class Appliance;

class Location : public Wt::WObject
{
public:
    Location(int sensorId, WObject* parent = 0);

    std::unordered_map<int, Appliance*> initAppliances(std::vector<int> applianceNumbers);
    std::unordered_map<int, Appliance*> appliances() const;

    int sensorId() const;
    void applianceUpdate(int pinNumber, int value);

private:
    int mSensorId;
    std::unordered_map<int, Appliance*> mAppliances;
};

#endif // LOCATION_H