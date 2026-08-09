/***************************************************************************

    file                 : axiom.cpp
    created              : 2006-08-31 01:21:49 UTC
    copyright            : (C) 2017 D.Schellhammer
                           (C) 2019 Andrew Sumner
                           (C) 2024 Xavier Bertaux

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef _WIN32
#include <windows.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <utility>
#include <math.h>

#include <portability.h>
#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robottools.h>
#include <timeanalysis.h>
#include <robot.h>

#include "driver.h"

using ::std::string;
using ::std::vector;
using ::std::pair;

// The "AXIOM" logger instance
GfLogger* PLogAXIOM = 0;

// TORCS interface
static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s);
static void newRace(int index, tCarElt* car, tSituation *s);
static void drive(int index, tCarElt* car, tSituation *s);
static int  pitcmd(int index, tCarElt* car, tSituation *s);
static void shutdown(int index);
static int  InitFuncPt(int index, void *pt);
static void endRace(int index, tCarElt *car, tSituation *s);

// CSR interface
static const int BUFSIZE = 256;
static const int MAXNBBOTS = 30;

// Drivers info: pair(first:Name, second:Desc)
static vector< pair<string, string> > Drivers;

// Number of drivers defined in robot's xml-file
static string nameBuffer;   // Robot's name // NOLINT(runtime/string)
static string pathBuffer;   // Robot's xml-filename // NOLINT(runtime/string)

// Save start index offset from robot's xml file
static int indexOffset = 0;
// Marker for undefined drivers to be able to comment out drivers
// in the robot's xml-file between others, not only at the end of the list
const char *sUndefined = "undefined";

class Drivers
{
public:
    ~Drivers()
    {
        clear();
    }

    void push_back(Driver *d)
    {
        v.push_back(d);
    }

    void clear()
    {
        for (auto d : v)
            delete d;

        v.clear();
    }

    Driver *operator[](int index)
    {
        return v[index];
    }

private:
    std::vector<Driver *> v;
};

static class std::vector<Driver *> driver;

////////////////////////////////
// Utility
////////////////////////////////
// Set robots's name and xml file pathname
static
void setRobotName(const string &name)
{
    char buffer[BUFSIZE];
    snprintf(buffer, BUFSIZE, "drivers/%s/%s.xml", name.c_str(), name.c_str());
    nameBuffer = name;
    pathBuffer = buffer;
}

////////////////////////////////////////////////////////////////
// CSR Interface (new, fixed name scheme, from Andrew's USR code)
////////////////////////////////////////////////////////////////

// Module entry point (new fixed name scheme).
// Extended for use with schismatic robots

extern "C" int moduleWelcome(const tModWelcomeIn* welcomeIn,
                             tModWelcomeOut* welcomeOut)
{
    // Save module name and loadDir, and determine module XML file pathname.
    setRobotName(welcomeIn->name);
    PLogAXIOM = GfLogger::instance("AXIOM");

    std::string dirstr = std::string(GfLocalDir()) + "drivers/axiom";
    const char *dir = dirstr.c_str();

    if (GfDirCreate(dir) != GF_DIR_CREATED)
    {
        PLogAXIOM->error("GfDirCreate %s failed\n", dir);
        return -1;
    }

    PLogAXIOM->setLevelThreshold(GfLogger::eInfo);

    int ret = 0;
    // Filehandle for robot's xml-file
    void *pRobotSettings = GfParmReadFileLocal(pathBuffer,
        GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);

    PLogAXIOM = GfLogger::instance("AXIOM");

    if (pRobotSettings)          // robot settings XML could be read
    {
        // Loop over all possible drivers, clear all buffers,
        // save defined driver names and descriptions.
        Drivers.clear();

        int n = GfParmGetEltNb(pRobotSettings, ROB_SECT_ROBOTS "/" ROB_LIST_INDEX);

        for (int i = 0; i < n; i++)
        {
            char SectionBuffer[BUFSIZE];

            snprintf(SectionBuffer, BUFSIZE, "%s/%s/%d", ROB_SECT_ROBOTS, ROB_LIST_INDEX, i);

            std::string sDriverName = GfParmGetStr(pRobotSettings, SectionBuffer,
                ROB_ATTR_NAME, sUndefined);

            // This driver is defined in robot's xml-file
            string sDriverDesc = GfParmGetStr(pRobotSettings, SectionBuffer,
                ROB_ATTR_DESC, "");
            Drivers.push_back(make_pair(sDriverName, sDriverDesc));
        }  // for i

        if (GfParmWriteFile(nullptr, pRobotSettings, "axiom"))
        {
            PLogAXIOM->error("GfParmWriteFile failed\n");
            ret = -1;
        }

        GfParmReleaseHandle(pRobotSettings);
    }
    else        // if robot settings XML could not be read
    {
        // For schismatic robots NBBOTS is unknown! Handle error here
        // But this is not considered a real failure of moduleWelcome !
    }

    // Set max nb of interfaces to return.
    welcomeOut->maxNbItf = Drivers.size();

    return ret;
}

// Module entry point (new fixed name scheme).
extern "C" int moduleInitialize(tModInfo *modInfo)
{
    // Clear all structures.
    memset(modInfo, 0, Drivers.size() * sizeof(tModInfo));
    driver.clear();

    for (size_t i = 0; i < Drivers.size(); i++)
    {
        modInfo[i].name = Drivers[i].first.c_str();
        modInfo[i].desc = Drivers[i].second.c_str();
        modInfo[i].fctInit = InitFuncPt;       // Init function.
        modInfo[i].gfId    = ROB_IDENT;        // Supported framework version.
        modInfo[i].index   = i + indexOffset;  // Indices from robot's xml-file.
        driver.push_back(new Driver(i));
    }  // for i

    return 0;
}

// Module exit point (new fixed name scheme).
extern "C" int moduleTerminate()
{
    return 0;
}

////////////////////////////////////////////////////////////////
// TORCS backward compatibility scheme, from Andrew's USR code
////////////////////////////////////////////////////////////////

// Module entry point
extern "C" int axiom(tModInfo *modInfo)
{
    Drivers.clear();
    pathBuffer = "drivers/axiom/axiom.xml";
    nameBuffer = "axiom";

    // Filehandle for robot's xml-file
    void *pRobotSettings = GfParmReadFileLocal(pathBuffer,
        GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);

    if (pRobotSettings)               // Let's look what we have to provide here
    {
        char SectionBuffer[BUFSIZE];
        int ret = 0, n = GfParmGetEltNb(pRobotSettings, ROB_SECT_ROBOTS);

        for (int i = 0; i < n; i++)
        {
            snprintf(SectionBuffer, BUFSIZE, "%s/%s/%d", ROB_SECT_ROBOTS, ROB_LIST_INDEX, i);
            string sDriverName = GfParmGetStr(pRobotSettings, SectionBuffer, ROB_ATTR_NAME, "");
            string sDriverDesc = GfParmGetStr(pRobotSettings, SectionBuffer, ROB_ATTR_DESC, "");
            Drivers.push_back(make_pair(sDriverName, sDriverDesc));
        }

        if (GfParmWriteFile(nullptr, pRobotSettings, "axiom"))
            ret = -1;

        GfParmReleaseHandle(pRobotSettings);

        if (ret)
            return ret;
    }

    return moduleInitialize(modInfo);
}

// Module exit point (TORCS backward compatibility scheme).
extern "C" int axiomShut()
{
    return moduleTerminate();
}

// Module interface initialization.
static
int InitFuncPt(int index, void *pt)
{
    tRobotItf *itf = static_cast<tRobotItf *>(pt);

    // Create robot instance for index.
    driver[index]->mDriverName = nameBuffer.c_str();

    itf->rbNewTrack = initTrack;    // Give the robot the track view called.
    itf->rbNewRace  = newRace;      // Start a new race.
    itf->rbDrive    = drive;        // Drive during race.
    itf->rbPitCmd   = pitcmd;       // Pit commands.
    itf->rbEndRace  = endRace;      // End of the current race.
    itf->rbShutdown = shutdown;     // Called before the module is unloaded.
    itf->index      = index;        // Index used if multiple interfaces.
    return 0;
}

// Called for every track change or new race.
static
void initTrack(int index, tTrack* track, void *carHandle,
                      void **carParmHandle, tSituation *s)
{
    driver[index]->InitTrack(track, carHandle, carParmHandle, s);
}


// Start a new race.
static
void newRace(int index, tCarElt* car, tSituation *s)
{
    driver[index]->NewRace(car, s);
}

// Drive during race.
static
void drive(int index, tCarElt* car, tSituation *s)
{
    driver[index]->Drive();
}

// Pitstop callback.
static
int pitcmd(int index, tCarElt* car, tSituation *s)
{
    return driver[index]->PitCmd();
}

// End of the current race.
static
void endRace(int index, tCarElt *car, tSituation *s)
{
    driver[index]->EndRace();
}

// Called before the module is unloaded.
static
void shutdown(int index)
{
    driver[index]->Shutdown();
    delete driver[index];
}
