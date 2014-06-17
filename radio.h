#ifndef RADIO_H
#define RADIO_H

#include <Wt/WObject>
#include <Wt/WSignal>

#include "location.h"

#include "/home/shantanu/dev/pi/mnt_rpi/home/pi/Desktop/RF24/librf24-rpi/librf24/RF24.h"

#include <unordered_map>

class Radio : public Wt::WObject
{
public:
    Radio(WObject* parent = 0);
    bool configureLocation(Location* location);
    bool activateAppliance(Appliance *appliance);
    bool deactivateAppliance(Appliance *appliance);

    void startListening();
    void readRadioData();
    std::unordered_map<int, Location*> configuredLocations();

private:

    // CE and CSN pins On header using GPIO numbering (not pin numbers)
    RF24 mRadio = RF24("/dev/spidev0.0", 8000000, 25);  // Setup for GPIO 25 CSN
    int mReadingPipesOpened = 0;
    bool mIsListening = false;
    std::unordered_map<int, Location*> mConfiguredLocations;

    bool writeToSensor(int sensorId, const std::string &data);
    void startListeningToSensor(Location *location);

    void processDataFromRadio(const std::string &data);
    void processSensorStatusData(int sensorId, const std::string &pinStatus);
};

#endif // RADIO_H
