#include "radio.h"

#include <Wt/WString>
#include <Wt/WTimer>

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

Radio *Radio::mInstance = nullptr;

Radio* Radio::instance()
{
    if (!mInstance) {
        mInstance = new Radio();
    }
    return mInstance;
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

    std::cout << "START TIMER " << std::endl;
    auto timer = new Wt::WTimer(this);
    timer->setInterval(100);
    timer->timeout().connect(this, &Radio::readRadioData);
    timer->start();

    mRadio.startListening();
    startListeningToSensor(0);
}

bool Radio::configureLocation(Location *location)
{
    std::vector<std::string> outputPins;
    for (auto appliance : location->appliances()) {
        outputPins.push_back(boost::lexical_cast<string>(appliance.second->applianceNumber()));
    }

    std::string outputConfigurationString = boost::algorithm::join(outputPins, ",");
    std::string configuration = ";" + outputConfigurationString;

    startListeningToSensor(location);
    writeToSensor(location->sensorId(), configuration);
}

bool Radio::activateAppliance(Appliance* appliance)
{
    auto sensorId = appliance->location()->sensorId();
    auto data = Wt::WString("{1},1").arg(appliance->applianceNumber()).toUTF8();
    return mRadio.write(data.c_str(), data.length()+1);
}

bool Radio::writeToSensor(int sensorId, const string& data)
{
    mRadio.openWritingPipe(BASE_PIPE + 1 + 2*sensorId);
    return mRadio.write(data.c_str(), data.length()+1);
}

void Radio::startListeningToSensor(Location* location)
{
    std::cout << "Starting to listen to " << location->sensorId() << std::endl;
    mRadio.openReadingPipe(mReadingPipesOpened,
                           BASE_PIPE + 2 + 2*location->sensorId());
    mReadingPipesOpened++;

    mConfiguredLocations.insert(make_pair(location->sensorId(), location));
}

void Radio::readRadioData()
{
    std::cout << "Attempt radio data" << std::endl;
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

    std::vector<std::string> pinStatuses;
    boost::split(pinStatuses, stringList.at(1), boost::is_any_of(";"));

    for (auto pinStatus : pinStatuses) {
        processSensorStatusData(sourceSensorId, pinStatus);
    }
}

void Radio::processSensorStatusData(int sensorId, const string& pinStatus)
{
    std::cout << "Processing pin data " << pinStatus << std::endl;

    if (pinStatus.length() == 0) {
        return;
    }

    std::vector<std::string> pinAndValue;
    boost::split(pinAndValue, pinStatus, boost::is_any_of(","));
    int pinNumber = boost::lexical_cast<int>(pinAndValue.at(0));
    int value = boost::lexical_cast<int>(pinAndValue.at(1));

    Location *location = mConfiguredLocations[sensorId];
    location->applianceUpdate(pinNumber, value);
}
