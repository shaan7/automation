/*
 * Copyright (C) 2008 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WBootstrapTheme>
#include <Wt/WEnvironment>
#include <Wt/WCssTheme>
#include <Wt/WCheckBox>
#include <Wt/WTimer>

#include "radio.h"
#include "appliance.h"

// c++0x only, for std::bind
// #include <functional>

#include <unordered_map>

using namespace Wt;

Radio radio;

/*
 * A simple hello world application class which demonstrates how to react
 * to events, read input, and give feed-back.
 */
class HelloApplication : public WApplication
{
public:
    HelloApplication(const WEnvironment& env);
    virtual ~HelloApplication();

private:
    WLineEdit *nameEdit_;
    WText *greeting_;

    void readRadioData();
    void createWidgets(std::vector<Location*> configuredLocations);
    void applyTheme(const WEnvironment& env);

    std::vector<Location*> mLocations;
    std::unordered_map<int, std::unordered_map<int, Wt::WCheckBox*> > mLocationCheckBoxes;
};

/*
 * The env argument contains information about the new session, and
 * the initial request. It must be passed to the WApplication
 * constructor so it is typically also an argument for your custom
 * application constructor.
*/
HelloApplication::HelloApplication(const WEnvironment& env)
    : WApplication(env)
{
    WApplication::instance()->enableUpdates(true);

    applyTheme(env);

    setTitle("Home Automation");                               // application title

    greeting_ = new WText(root());                         // empty text

    auto sessionId = Wt::WApplication::instance()->sessionId();
    auto l0 = new Location(sessionId, 0, &radio, this);
    auto l1 = new Location(sessionId, 1, &radio, this);

    auto appliances0 = l0->initAppliances(std::vector<int> { 2, 3, 4, 5 });
    auto appliances1 = l1->initAppliances(std::vector<int> { 2, 3, 4, 5 });

    radio.configureLocation(l0);
    radio.configureLocation(l1);
    radio.startListening();

    mLocations = {l0, l1};
    createWidgets(mLocations);

    std::cout << "START TIMER " << std::endl;
    auto timer = new Wt::WTimer(this);
    timer->setInterval(1000);
    timer->timeout().connect(this, &HelloApplication::readRadioData);
    timer->start();
}

void HelloApplication::readRadioData()
{
    greeting_->setText("Refreshing...");
    Wt::WApplication::instance()->processEvents();
    radio.readRadioData();
    greeting_->setText("Done");
}

HelloApplication::~HelloApplication()
{
    for (auto location : mLocations) {
        radio.removeLocation(location);
    }
}

void HelloApplication::createWidgets(std::vector<Location*> configuredLocations)
{
    for (auto l : configuredLocations) {
        int sensorId = l->sensorId();

        for (auto appliance : l->appliances()) {
            int applianceNumber = appliance.first;
            Appliance *a = appliance.second;

            root()->addWidget(new WBreak());
            Wt::WCheckBox *checkBox = new Wt::WCheckBox(Wt::WString("Room {1} Appliance {2}").arg(sensorId).arg(applianceNumber), root());
            checkBox->setEnabled(false);
            a->activated.connect(checkBox, &Wt::WCheckBox::setChecked);
            a->deactivated.connect(checkBox, &Wt::WCheckBox::setUnChecked);

            Wt::WPushButton *button = new Wt::WPushButton("Toggle", root());
            button->clicked().connect(std::bind([a] () {
                a->toggle();
            }));

            button = new Wt::WPushButton("Turn ON", root());
            button->clicked().connect(std::bind([a] () {
                a->activate();
            }));

            button = new Wt::WPushButton("Turn OFF", root());
            button->clicked().connect(std::bind([a] () {
                a->deactivate();
            }));
        }
    }
}

void HelloApplication::applyTheme(const WEnvironment &env)
{
    // Choice of theme: defaults to bootstrap3 but can be overridden using
    // a theme parameter (for testing)
    const std::string *themePtr = env.getParameter("theme");
    std::string theme;
    if (!themePtr)
        theme = "bootstrap3";
    else
        theme = *themePtr;

    if (theme == "bootstrap3") {
        Wt::WBootstrapTheme *bootstrapTheme = new Wt::WBootstrapTheme(this);
        bootstrapTheme->setVersion(Wt::WBootstrapTheme::Version3);
        bootstrapTheme->setResponsive(true);
        setTheme(bootstrapTheme);

        // load the default bootstrap3 (sub-)theme
        useStyleSheet("resources/themes/bootstrap/3/bootstrap-theme.min.css");
    } else if (theme == "bootstrap2") {
        Wt::WBootstrapTheme *bootstrapTheme = new Wt::WBootstrapTheme(this);
        bootstrapTheme->setResponsive(true);
        setTheme(bootstrapTheme);
    } else {
        setTheme(new Wt::WCssTheme(theme));
    }
}

WApplication *createApplication(const WEnvironment& env)
{
    /*
     * You could read information from the environment to decide whether
     * the user has permission to start a new application
     */
    return new HelloApplication(env);
}

int main(int argc, char **argv)
{
    /*
     * Your main method may set up some shared resources, but should then
     * start the server application (FastCGI or httpd) that starts listening
     * for requests, and handles all of the application life cycles.
     *
     * The last argument to WRun specifies the function that will instantiate
     * new application objects. That function is executed when a new user surfs
     * to the Wt application, and after the library has negotiated browser
     * support. The function should return a newly instantiated application
     * object.
     */
    return WRun(argc, argv, &createApplication);
}

