#ifndef LOCATION_H
#define LOCATION_H

#include <Wt/WObject>

#include <vector>
#include <unordered_map>

class Radio;
class Appliance;

class Location : public Wt::WObject
{
public:
    Location(std::string sessionId, int sensorId, Radio* radio, WObject* parent = 0);

    std::unordered_map<int, Appliance*> initAppliances(std::vector<int> applianceNumbers);
    std::unordered_map<int, Appliance*> appliances() const;

    int sensorId() const;
    void applianceUpdate(int pinNumber, int value);

    void setConfigured(bool configured);
    bool configured() const;

    std::string sessionId() const;

private:
    int mSensorId;
    std::unordered_map<int, Appliance*> mAppliances;
    Radio *mRadio;
    bool mConfigured = false;
    std::string mSessionId;
};

#endif // LOCATION_H
