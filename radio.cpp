#include "radio.h"

#include <Wt/WString>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "appliance.h"

#include <iostream>

namespace {
static const uint64_t BASE_PIPE = 0xF0F0F0F0E0LL;
static const int BUFFER_SIZE = 128;
}

Radio::Radio(Wt::WObject* parent): WObject(parent)
{
    mRadio.begin();
    mRadio.enableDynamicPayloads();
    mRadio.setAutoAck(1);
    mRadio.setRetries(15,15);
    mRadio.setDataRate(RF24_1MBPS);
    mRadio.setPALevel(RF24_PA_MAX);
    mRadio.setChannel(76);
    mRadio.setCRCLength(RF24_CRC_16);

    //
    // Dump the configuration of the rf unit for debugging
    //
    mRadio.printDetails();
}

bool Radio::configureLocation(Location *location)
{
    if (isLocationConfigured(location->sensorId())) {
        std::cerr << "Location " << location->sensorId() << " already configured" << std::endl;
        return false;
    }

    startListeningToSensor(location);
    mConfiguredLocations.insert(std::make_pair(location->sensorId(), location));
}

bool Radio::sendConfigurationToLocation(Location *location)
{
    std::vector<std::string> outputPins;
    for (auto appliance : location->appliances()) {
        outputPins.push_back(boost::lexical_cast<string>(appliance.second->applianceNumber()));
    }

    std::string outputConfigurationString = boost::algorithm::join(outputPins, ",");
    std::string configuration = ";" + outputConfigurationString;

    writeToSensor(location->sensorId(), configuration);
}

bool Radio::activateAppliance(Appliance* appliance)
{
    auto sensorId = appliance->location()->sensorId();
    std::cout << "ACTIVATING " << sensorId << " APPLIANCE " << appliance->applianceNumber() << std::endl;

    auto data = Wt::WString("{1},1").arg(appliance->applianceNumber()).toUTF8();

    return writeToSensor(sensorId, data);
}

bool Radio::deactivateAppliance(Appliance* appliance)
{
    auto sensorId = appliance->location()->sensorId();
    std::cout << "DEACTIVATING " << sensorId << " APPLIANCE " << appliance->applianceNumber() << std::endl;

    auto data = Wt::WString("{1},0").arg(appliance->applianceNumber()).toUTF8();

    return writeToSensor(sensorId, data);
}

bool Radio::writeToSensor(int sensorId, const string& data)
{
    std::cout << "Writing " << data << " to " << sensorId << "... ";
    mRadio.stopListening();
    mRadio.openWritingPipe(BASE_PIPE + 1 + 2*sensorId);
    bool result = mRadio.write(data.c_str(), data.length()+1);
    mRadio.startListening();

    std::cout << (result ? "OK" : "FAIL") << std::endl;
    return result;
}

void Radio::startListeningToSensor(Location* location)
{
    mReadingPipesOpened++;
    std::cout << "Starting to listen to " << location->sensorId() << " on pipe " <<  mReadingPipesOpened << std::endl;
    mRadio.openReadingPipe(mReadingPipesOpened,
                           BASE_PIPE + 2 + 2*location->sensorId());
}

void Radio::readRadioData()
{
    char receivePayload[BUFFER_SIZE];
    uint8_t pipe;

    while (mRadio.available(&pipe)) {
        uint8_t len = mRadio.getDynamicPayloadSize();
        mRadio.read(receivePayload, len);
        receivePayload[len]='\0';

        processDataFromRadio(std::string(receivePayload));
    }
}

void Radio::processDataFromRadio(const string& data)
{
    std::vector<std::string> stringList;
    boost::split(stringList, data, boost::is_any_of(" "));

    int sourceSensorId = boost::lexical_cast<int>(stringList.at(0));

    std::string pinStatusString = stringList.at(1);

    if (pinStatusString == "-1") {      //The location for this sensor is not configured yet
        if (isLocationConfigured(sourceSensorId)) {
            auto location = mConfiguredLocations.at(sourceSensorId);
            location->setConfigured(false);
            sendConfigurationToLocation(mConfiguredLocations.at(sourceSensorId));
        }
    } else {    //The location for this sensor is configured and sending us state
        auto location = mConfiguredLocations.at(sourceSensorId);
        if (!location->configured()) {
            location->setConfigured(true);
        }

        std::vector<std::string> pinStatuses;
        boost::split(pinStatuses, pinStatusString, boost::is_any_of(";"));

        for (auto pinStatus : pinStatuses) {
            processSensorStatusData(sourceSensorId, pinStatus);
        }
    }
}

void Radio::processSensorStatusData(int sensorId, const string& pinStatus)
{
    if (pinStatus.length() == 0) {
        return;
    }

    std::vector<std::string> pinAndValue;
    boost::split(pinAndValue, pinStatus, boost::is_any_of(","));
    int pinNumber = boost::lexical_cast<int>(pinAndValue.at(0));
    int value = boost::lexical_cast<int>(pinAndValue.at(1));

    try {
        Location *location = mConfiguredLocations.at(sensorId);
        location->applianceUpdate(pinNumber, value);
    } catch (std::out_of_range e) {
        std::cerr << "Failed to fetch location for " << sensorId << std::endl;
    }
}

void Radio::startListening()
{
    if (!mIsListening) {
        mRadio.startListening();
        mIsListening = true;
    }
}

std::unordered_map< int, Location* > Radio::configuredLocations()
{
    return mConfiguredLocations;
}

bool Radio::isLocationConfigured(int sensorId)
{
    return mConfiguredLocations.count(sensorId) > 0;
}
