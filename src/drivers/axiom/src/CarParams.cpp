/***************************************************************************

    file        : CarParams.cpp
    created     : 8 Jun 2017
    copyright   : (C) 2017 D.Schellhammer
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

#include "CarParams.h"
#include "Utils.h"

#include <tgf.h>

extern GfLogger* PLogAXIOM;
#define LogAXIOM (*PLogAXIOM)

#include <iostream>

CarParams::CarParams() :
    HASCPD(false),
    HASABS(false),
    HASESP(false),
    HASTCL(false),
    mCar(nullptr)
{
}

CarParams::~CarParams()
{
}

void CarParams::init(tCarElt* car, MyTrack* track, int rain)
{
    LogAXIOM.debug("# Axiom Init !!\n");
    mCar = car;
    mTrack = track;
    mYaw = 0.0;
    mLastDamage = 0;
    mRain = 0;
    mPrevGear = mCar->_gear;
    mGlobalPos = Vec3d(mCar->_pos_X, mCar->_pos_Y, mCar->_pos_Z);
    mFrontAxleOffset = mCar->priv.wheel[0].relPos.x;
    double length = mTrack->length();
    mWheels.init(mCar, mMuScale, length, rain );
    initVars();
    initCa();
    initCw();
    initBrakes();
}

void CarParams::readPrivateSection(const MyParam& param, bool qualif, int rain)
{
    LogAXIOM.debug(" # Reading private section\n");
    mAbsSlip = param.getNum("private", "ABS slip");

    if( rain == 0)
    {
        mBrakeMuFactor = param.getNum("private", "brake mu factor");
        mBrakeMuFactorCold = param.getNum("private", "brake mu factor cold");
        mBrakeMuPitFactor= param.getNum("private", "brake pit mu factor");
        mMuScale = param.getNum("private", "mu scale");
        mRain = 0;
    }
    else if (rain == 1)
    {
        mBrakeMuFactor = param.getNum("private", "brake mu factor rain1");
        mBrakeMuFactorCold = param.getNum("private", "brake mu factor cold rain1");
        mBrakeMuPitFactor= param.getNum("private", "brake pit mu factor rain1");
        mMuScale = param.getNum("private", "mu scale rain1");
        mRain = 1;
    }
    else if(rain == 2)
    {
        mBrakeMuFactor = param.getNum("private", "brake mu factor rain2");
        mBrakeMuFactorCold = param.getNum("private", "brake mu factor cold rain2");
        mBrakeMuPitFactor= param.getNum("private", "brake pit mu factor rain2");
        mMuScale = param.getNum("private", "mu scale rain2");
        mRain = 2;
    }
    else
    {
        mBrakeMuFactor = param.getNum("private", "brake mu factor rain3");
        mBrakeMuFactorCold = param.getNum("private", "brake mu factor cold rain3");
        mBrakeMuPitFactor= param.getNum("private", "brake pit mu factor rain3");
        mMuScale = param.getNum("private", "mu scale rain3");
        mRain = 3;
    }

    if(mBrakeMuPitFactor == 0)
        mBrakeMuPitFactor = 0.70;

    if (mBrakeMuFactorCold == 0)
        mBrakeMuFactorCold = mBrakeMuFactor;

    mMuScaleLR = param.getNum("private", "LR mu scale");
    mBumpSpeedFactor = param.getNum("private", "bump speed factor");
    mPathL = param.getNum("private", "pathl");
    mPathR = param.getNum("private", "pathr");
    mFuelPerMeter = param.getNum("private", "fuel per meter");

    mFuelWeightFactor = param.getNum("private", "fuel weight factor");
    mTireWearPerMeter = param.getNum("private", "tire wear per meter");
    mSideSlipTCL = param.getNum("private", "TCL side slip");
    mSideSlipTCLQualy = param.getNum("private", "TCL side slip qualy");
    mSideSlipTCLFactor = param.getNum("private", "TCL side slip factor");
    mMaxFuel = param.getNum("private", "max fuel");

    if(mPathL == 0.0)
        mPathL = -1.0;

    if(mPathR == 0.0)
        mPathR = -1.0;

    if (mSideSlipTCLFactor == 0.0)
        mSideSlipTCLFactor = 0.2;

    LogAXIOM.debug("# ABS SLIP              = %.2f\n", mAbsSlip);
    LogAXIOM.debug("# Brake mu Factor       = %.2f\n", mBrakeMuFactor);
    LogAXIOM.debug("# Brake mu Factor cold  = %.2f\n", mBrakeMuFactorCold);
    LogAXIOM.debug("# LR mu Scale           = %.2f\n", mMuScaleLR);
    LogAXIOM.debug("# mu Scale              = %.2f\n", mMuScale);
    //LogAXIOM.info("# mu Scale cold         = %.2f\n", mMuScaleCold);
    LogAXIOM.debug("# Bump Speed Factor     = %.2f\n", mBumpSpeedFactor);
    LogAXIOM.debug("# Path L                = %.2f\n", mPathL);
    LogAXIOM.debug("# Path R                = %.2f\n", mPathR);
    LogAXIOM.debug("# Fuel per Meter        = %.6f\n", mFuelPerMeter);
    LogAXIOM.debug("# Fuel weight factor    = %.2f\n", mFuelWeightFactor);
    LogAXIOM.debug("# Tire Wear per Meter   = %.8f\n", mTireWearPerMeter);
    LogAXIOM.debug("# TCL side Slip         = %.2f\n", mSideSlipTCL);
    LogAXIOM.debug("# TCL side Slip quality = %.2f\n", mSideSlipTCLQualy);
    LogAXIOM.debug("# TCL side Slip Factor  = %.2f\n", mSideSlipTCLFactor);
}

void CarParams::readStrategySection(const MyParam& param, double temperature, int rain, double distance, int laps)
{
    mMaxFuel5Laps  = param.getNum("strategy fuel", "fuel 5 laps");
    mMaxFuel10Laps = param.getNum("strategy fuel", "fuel 10 laps");
    mMaxFuel80KM   = param.getNum("strategy fuel", "fuel 80 km");

    if (rain == 0 || rain == 1 || rain == 2)
    {
        mMaxFuel160KM  = param.getNum("strategy fuel", "fuel 160 km");
        mMaxFuel320KM  = param.getNum("strategy fuel", "fuel 320 km");
        mMaxFuel330KM  = param.getNum("strategy fuel", "fuel others");
    }
    else
    {
        mMaxFuel160KM  = param.getNum("strategy fuel", "rain fuel 160 km");
        mMaxFuel320KM  = param.getNum("strategy fuel", "rain fuel 320 km");
        mMaxFuel330KM  = param.getNum("strategy fuel", "rain fuel others");
    }

    if(HASCPD == false)
    {
        if (temperature < 8.0)
        {
            mPressureFR = param.getNum("strategy 8", "pressure FR");
            mPressureFL = param.getNum("strategy 8", "pressure FL");
            mPressureRR = param.getNum("strategy 8", "pressure RR");
            mPressureRL = param.getNum("strategy 8", "pressure RL");
            mCamberFR = param.getNum("strategy 8", "camber FR");
            mCamberFL = param.getNum("strategy 8", "camber FL");
            mCamberRR = param.getNum("strategy 8", "camber RR");
            mCamberRL = param.getNum("strategy 8", "camber RL");
            mMaxArrets     = param.getNum("strategy 8", "num arrets");
            LogAXIOM.debug("# AXIOM Strategy temperature < 8\n");
        }
        else if (temperature < 15.0)
        {
            mPressureFR = param.getNum("strategy 15", "pressure FR");
            mPressureFL = param.getNum("strategy 15", "pressure FL");
            mPressureRR = param.getNum("strategy 15", "pressure RR");
            mPressureRL = param.getNum("strategy 15", "pressure RL");
            mCamberFR = param.getNum("strategy 15", "camber FR");
            mCamberFL = param.getNum("strategy 15", "camber FL");
            mCamberRR = param.getNum("strategy 15", "camber RR");
            mCamberRL = param.getNum("strategy 15", "camber RL");
            mMaxArrets     = param.getNum("strategy 15", "num arrets");
            LogAXIOM.debug("# AXIOM Strategy temperature <= 15\n");
        }
        else if (temperature < 20.0)
        {
            mPressureFR = param.getNum("strategy 20", "pressure FR");
            mPressureFL = param.getNum("strategy 20", "pressure FL");
            mPressureRR = param.getNum("strategy 20", "pressure RR");
            mPressureRL = param.getNum("strategy 20", "pressure RL");
            mCamberFR = param.getNum("strategy 20", "camber FR");
            mCamberFL = param.getNum("strategy 20", "camber FL");
            mCamberRR = param.getNum("strategy 20", "camber RR");
            mCamberRL = param.getNum("strategy 20", "camber RL");
            mMaxArrets     = param.getNum("strategy 20", "num arrets");
            LogAXIOM.debug("# AXIOM Strategy temperature <= 20\n");
        }
        else if( temperature < 25.0)
        {
            mPressureFR = param.getNum("strategy 25", "pressure FR");
            mPressureFL = param.getNum("strategy 25", "pressure FL");
            mPressureRR = param.getNum("strategy 25", "pressure RR");
            mPressureRL = param.getNum("strategy 25", "pressure RL");
            mCamberFR = param.getNum("strategy 25", "camber FR");
            mCamberFL = param.getNum("strategy 25", "camber FL");
            mCamberRR = param.getNum("strategy 25", "camber RR");
            mCamberRL = param.getNum("strategy 25", "camber RL");
            mMaxArrets     = param.getNum("strategy 25", "num arrets");
            LogAXIOM.debug("# AXIOM Strategy temperature <= 25\n");
        }
        else if (temperature < 30.0)
        {
            mPressureFR = param.getNum("strategy 30", "pressure FR");
            mPressureFL = param.getNum("strategy 30", "pressure FL");
            mPressureRR = param.getNum("strategy 30", "pressure RR");
            mPressureRL = param.getNum("strategy 30", "pressure RL");
            mCamberFR = param.getNum("strategy 30", "camber FR");
            mCamberFL = param.getNum("strategy 30", "camber FL");
            mCamberRR = param.getNum("strategy 30", "camber RR");
            mCamberRL = param.getNum("strategy 30", "camber RL");
            mMaxArrets     = param.getNum("strategy 30", "num arrets");
            LogAXIOM.debug("# AXIOM Strategy temperature <= 30\n");
        }
        else
        {
            mPressureFR = param.getNum("strategy 35", "pressure FR");
            mPressureFL = param.getNum("strategy 35", "pressure FL");
            mPressureRR = param.getNum("strategy 35", "pressure RR");
            mPressureRL = param.getNum("strategy 35", "pressure RL");
            mCamberFR = param.getNum("strategy 35", "camber FR");
            mCamberFL = param.getNum("strategy 35", "camber FL");
            mCamberRR = param.getNum("strategy 35", "camber RR");
            mCamberRL = param.getNum("strategy 35", "camber RL");
            mMaxArrets     = param.getNum("strategy 35", "num arrets");
            LogAXIOM.debug("# AXIOM Strategy temperature > 31\n");
        }

        if (mMaxFuel5Laps == 0.0)
            mMaxFuel5Laps = 30.0;

        if (mMaxFuel10Laps == 0.0)
            mMaxFuel10Laps = 50.0;

        if (mMaxFuel80KM == 0.0)
            mMaxFuel80KM = 40.0;

        if (mMaxFuel160KM == 0.0)
            mMaxFuel160KM = 63.0;

        if (mMaxFuel320KM == 0.0)
            mMaxFuel320KM = 70.0;

        if (mMaxFuel330KM == 0.0)
            mMaxFuel330KM = 70.0;

        LogAXIOM.debug("# Strategy 5 Laps     = %.2f\n", mMaxFuel5Laps);
        LogAXIOM.debug("# Strategy 10 Laps    = %.2f\n", mMaxFuel10Laps);
        LogAXIOM.debug("# Strategy 80 KM      = %.2f\n", mMaxFuel80KM);
        LogAXIOM.debug("# Strategy 160 KM     = %.2f\n", mMaxFuel160KM);
        LogAXIOM.debug("# Strategy 320 KM     = %.2f\n", mMaxFuel320KM);
        LogAXIOM.debug("# Strategy 330 KM     = %.2f\n", mMaxFuel330KM);
        LogAXIOM.debug("# Strategy num arrets = %i\n",   mMaxArrets);

        LogAXIOM.debug("# front wheel right pressure = %.2f\n", mPressureFR / 6894.76f);
        LogAXIOM.debug("# front wheel left  pressure = %.2f\n", mPressureFL / 6894.76f);
        LogAXIOM.debug("# rear wheel right pressure  = %.2f\n", mPressureRR / 6894.76f);
        LogAXIOM.debug("# rear wheel left  pressure  = %.2f\n", mPressureRL / 6894.76f);
        LogAXIOM.debug("# front wheel right camber   = %.2f\n", mCamberFR );
        LogAXIOM.debug("# front wheel left  camber   = %.2f\n", mCamberFL);
        LogAXIOM.debug("# rear wheel right camber    = %.2f\n", mCamberRR );
        LogAXIOM.debug("# rear wheel left  camber    = %.2f\n", mCamberRL );
    }
    else
    {
        mCompound = calcCompound(temperature, distance, laps);
        mTireMu = mTireMuC[mCompound];

        if(mCompound == 1)
        {
            LogAXIOM.debug("Compound SOFT !!!\n");
            if (temperature < 8)
            {
                mPressureFR = param.getNum("strategy soft 8", "pressure FR");
                mPressureFL = param.getNum("strategy soft 8", "pressure FL");
                mPressureRR = param.getNum("strategy soft 8", "pressure RR");
                mPressureRL = param.getNum("strategy soft 8", "pressure RL");
                mCamberFR = param.getNum("strategy soft 8", "camber FR");
                mCamberFL = param.getNum("strategy soft 8", "camber FL");
                mCamberRR = param.getNum("strategy soft 8", "camber RR");
                mCamberRL = param.getNum("strategy soft 8", "camber RL");
            }
            else if (temperature < 15)
            {
                mPressureFR = param.getNum("strategy soft 15", "pressure FR");
                mPressureFL = param.getNum("strategy soft 15", "pressure FL");
                mPressureRR = param.getNum("strategy soft 15", "pressure RR");
                mPressureRL = param.getNum("strategy soft 15", "pressure RL");
                mCamberFR = param.getNum("strategy soft 15", "camber FR");
                mCamberFL = param.getNum("strategy soft 15", "camber FL");
                mCamberRR = param.getNum("strategy soft 15", "camber RR");
                mCamberRL = param.getNum("strategy soft 15", "camber RL");
            }
            else if (temperature < 20)
            {
                mPressureFR = param.getNum("strategy soft 20", "pressure FR");
                mPressureFL = param.getNum("strategy soft 20", "pressure FL");
                mPressureRR = param.getNum("strategy soft 20", "pressure RR");
                mPressureRL = param.getNum("strategy soft 20", "pressure RL");
                mCamberFR = param.getNum("strategy soft 20", "camber FR");
                mCamberFL = param.getNum("strategy soft 20", "camber FL");
                mCamberRR = param.getNum("strategy soft 20", "camber RR");
                mCamberRL = param.getNum("strategy soft 20", "camber RL");
            }
            else if (temperature < 25)
            {
                mPressureFR = param.getNum("strategy soft 25", "pressure FR");
                mPressureFL = param.getNum("strategy soft 25", "pressure FL");
                mPressureRR = param.getNum("strategy soft 25", "pressure RR");
                mPressureRL = param.getNum("strategy soft 25", "pressure RL");
                mCamberFR = param.getNum("strategy soft 25", "camber FR");
                mCamberFL = param.getNum("strategy soft 25", "camber FL");
                mCamberRR = param.getNum("strategy soft 25", "camber RR");
                mCamberRL = param.getNum("strategy soft 25", "camber RL");
            }
            else if (temperature < 30)
            {
                mPressureFR = param.getNum("strategy soft 30", "pressure FR");
                mPressureFL = param.getNum("strategy soft 30", "pressure FL");
                mPressureRR = param.getNum("strategy soft 30", "pressure RR");
                mPressureRL = param.getNum("strategy soft 30", "pressure RL");
                mCamberFR = param.getNum("strategy soft 30", "camber FR");
                mCamberFL = param.getNum("strategy soft 30", "camber FL");
                mCamberRR = param.getNum("strategy soft 30", "camber RR");
                mCamberRL = param.getNum("strategy soft 30", "camber RL");
            }
            else
            {
                mPressureFR = param.getNum("strategy soft 35", "pressure FR");
                mPressureFL = param.getNum("strategy soft 35", "pressure FL");
                mPressureRR = param.getNum("strategy soft 35", "pressure RR");
                mPressureRL = param.getNum("strategy soft 35", "pressure RL");
                mCamberFR = param.getNum("strategy soft 35", "camber FR");
                mCamberFL = param.getNum("strategy soft 35", "camber FL");
                mCamberRR = param.getNum("strategy soft 35", "camber RR");
                mCamberRL = param.getNum("strategy soft 35", "camber RL");
            }
        }
        else if (mCompound == 2)
        {
            LogAXIOM.debug("Compound MEDIUM !!!\n");
            if (temperature < 8)
            {
                LogAXIOM.debug("Compound MEDIUM temperature 8 !!!\n");
                mPressureFR = param.getNum("strategy medium 8", "pressure FR");
                mPressureFL = param.getNum("strategy medium 8", "pressure FL");
                mPressureRR = param.getNum("strategy medium 8", "pressure RR");
                mPressureRL = param.getNum("strategy medium 8", "pressure RL");
                mCamberFR = param.getNum("strategy medium 8", "camber FR");
                mCamberFL = param.getNum("strategy medium 8", "camber FL");
                mCamberRR = param.getNum("strategy medium 8", "camber RR");
                mCamberRL = param.getNum("strategy medium 8", "camber RL");
            }
            else if (temperature < 15)
            {
                LogAXIOM.debug("Compound MEDIUM temperature 15 !!!\n");
                mPressureFR = param.getNum("strategy medium 15", "pressure FR");
                mPressureFL = param.getNum("strategy medium 15", "pressure FL");
                mPressureRR = param.getNum("strategy medium 15", "pressure RR");
                mPressureRL = param.getNum("strategy medium 15", "pressure RL");
                mCamberFR = param.getNum("strategy medium 15", "camber FR");
                mCamberFL = param.getNum("strategy medium 15", "camber FL");
                mCamberRR = param.getNum("strategy medium 15", "camber RR");
                mCamberRL = param.getNum("strategy medium 15", "camber RL");
            }
            else if (temperature < 20)
            {
                LogAXIOM.debug("Compound MEDIUM temperature 20 !!!\n");
                mPressureFR = param.getNum("strategy medium 20", "pressure FR");
                mPressureFL = param.getNum("strategy medium 20", "pressure FL");
                mPressureRR = param.getNum("strategy medium 20", "pressure RR");
                mPressureRL = param.getNum("strategy medium 20", "pressure RL");
                mCamberFR = param.getNum("strategy medium 20", "camber FR");
                mCamberFL = param.getNum("strategy medium 20", "camber FL");
                mCamberRR = param.getNum("strategy medium 20", "camber RR");
                mCamberRL = param.getNum("strategy medium 20", "camber RL");
            }
            else if (temperature < 25)
            {
                LogAXIOM.debug("Compound MEDIUM temperature 25 !!!\n");
                mPressureFR = param.getNum("strategy medium 25", "pressure FR");
                mPressureFL = param.getNum("strategy medium 25", "pressure FL");
                mPressureRR = param.getNum("strategy medium 25", "pressure RR");
                mPressureRL = param.getNum("strategy medium 25", "pressure RL");
                mCamberFR = param.getNum("strategy medium 25", "camber FR");
                mCamberFL = param.getNum("strategy medium 25", "camber FL");
                mCamberRR = param.getNum("strategy medium 25", "camber RR");
                mCamberRL = param.getNum("strategy medium 25", "camber RL");
            }
            else if (temperature < 30)
            {
                LogAXIOM.debug("Compound MEDIUM temperature 30 !!!\n");
                mPressureFR = param.getNum("strategy medium 30", "pressure FR");
                mPressureFL = param.getNum("strategy medium 30", "pressure FL");
                mPressureRR = param.getNum("strategy medium 30", "pressure RR");
                mPressureRL = param.getNum("strategy medium 30", "pressure RL");
                mCamberFR = param.getNum("strategy medium 30", "camber FR");
                mCamberFL = param.getNum("strategy medium 30", "camber FL");
                mCamberRR = param.getNum("strategy medium 30", "camber RR");
                mCamberRL = param.getNum("strategy medium 30", "camber RL");
            }
            else
            {
                LogAXIOM.debug("Compound MEDIUM temperature 35 !!!\n");
                mPressureFR = param.getNum("strategy medium 35", "pressure FR");
                mPressureFL = param.getNum("strategy medium 35", "pressure FL");
                mPressureRR = param.getNum("strategy medium 35", "pressure RR");
                mPressureRL = param.getNum("strategy medium 35", "pressure RL");
                mCamberFR = param.getNum("strategy medium 35", "camber FR");
                mCamberFL = param.getNum("strategy medium 35", "camber FL");
                mCamberRR = param.getNum("strategy medium 35", "camber RR");
                mCamberRL = param.getNum("strategy medium 35", "camber RL");
            }
        }
        else if(mCompound == 3)
        {
            LogAXIOM.debug("Compound HARD !!!\n");
            if (temperature < 8)
            {
                mPressureFR = param.getNum("strategy hard 8", "pressure FR");
                mPressureFL = param.getNum("strategy hard 8", "pressure FL");
                mPressureRR = param.getNum("strategy hard 8", "pressure RR");
                mPressureRL = param.getNum("strategy hard 8", "pressure RL");
                mCamberFR = param.getNum("strategy hard 8", "camber FR");
                mCamberFL = param.getNum("strategy hard 8", "camber FL");
                mCamberRR = param.getNum("strategy hard 8", "camber RR");
                mCamberRL = param.getNum("strategy hard 8", "camber RL");
            }
            else if (temperature < 15)
            {
                mPressureFR = param.getNum("strategy hard 15", "pressure FR");
                mPressureFL = param.getNum("strategy hard 15", "pressure FL");
                mPressureRR = param.getNum("strategy hard 15", "pressure RR");
                mPressureRL = param.getNum("strategy hard 15", "pressure RL");
                mCamberFR = param.getNum("strategy hard 15", "camber FR");
                mCamberFL = param.getNum("strategy hard 15", "camber FL");
                mCamberRR = param.getNum("strategy hard 15", "camber RR");
                mCamberRL = param.getNum("strategy hard 15", "camber RL");
            }
            else if (temperature < 20)
            {
                mPressureFR = param.getNum("strategy hard 20", "pressure FR");
                mPressureFL = param.getNum("strategy hard 20", "pressure FL");
                mPressureRR = param.getNum("strategy hard 20", "pressure RR");
                mPressureRL = param.getNum("strategy hard 20", "pressure RL");
                mCamberFR = param.getNum("strategy hard 20", "camber FR");
                mCamberFL = param.getNum("strategy hard 20", "camber FL");
                mCamberRR = param.getNum("strategy hard 20", "camber RR");
                mCamberRL = param.getNum("strategy hard 20", "camber RL");
            }
            else if (temperature < 25)
            {
                mPressureFR = param.getNum("strategy hard 25", "pressure FR");
                mPressureFL = param.getNum("strategy hard 25", "pressure FL");
                mPressureRR = param.getNum("strategy hard 25", "pressure RR");
                mPressureRL = param.getNum("strategy hard 25", "pressure RL");
                mCamberFR = param.getNum("strategy hard 25", "camber FR");
                mCamberFL = param.getNum("strategy hard 25", "camber FL");
                mCamberRR = param.getNum("strategy hard 25", "camber RR");
                mCamberRL = param.getNum("strategy hard 25", "camber RL");
            }
            else if (temperature < 30)
            {
                mPressureFR = param.getNum("strategy hard 30", "pressure FR");
                mPressureFL = param.getNum("strategy hard 30", "pressure FL");
                mPressureRR = param.getNum("strategy hard 30", "pressure RR");
                mPressureRL = param.getNum("strategy hard 30", "pressure RL");
                mCamberFR = param.getNum("strategy hard 30", "camber FR");
                mCamberFL = param.getNum("strategy hard 30", "camber FL");
                mCamberRR = param.getNum("strategy hard 30", "camber RR");
                mCamberRL = param.getNum("strategy hard 30", "camber RL");
            }
            else
            {
                mPressureFR = param.getNum("strategy hard 35", "pressure FR");
                mPressureFL = param.getNum("strategy hard 35", "pressure FL");
                mPressureRR = param.getNum("strategy hard 35", "pressure RR");
                mPressureRL = param.getNum("strategy hard 35", "pressure RL");
                mCamberFR = param.getNum("strategy hard 35", "camber FR");
                mCamberFL = param.getNum("strategy hard 35", "camber FL");
                mCamberRR = param.getNum("strategy hard 35", "camber RR");
                mCamberRL = param.getNum("strategy hard 35", "camber RL");
            }
        }
        else if (mCompound == 4)
        {
            LogAXIOM.debug("Compound WET !!!\n");
            if (temperature < 8)
            {
                mPressureFR = param.getNum("strategy wet 8", "pressure FR");
                mPressureFL = param.getNum("strategy wet 8", "pressure FL");
                mPressureRR = param.getNum("strategy wet 8", "pressure RR");
                mPressureRL = param.getNum("strategy wet 8", "pressure RL");
                mCamberFR = param.getNum("strategy wet 8", "camber FR");
                mCamberFL = param.getNum("strategy wet 8", "camber FL");
                mCamberRR = param.getNum("strategy wet 8", "camber RR");
                mCamberRL = param.getNum("strategy wet 8", "camber RL");
            }
            else if (temperature < 15)
            {
                mPressureFR = param.getNum("strategy wet 15", "pressure FR");
                mPressureFL = param.getNum("strategy wet 15", "pressure FL");
                mPressureRR = param.getNum("strategy wet 15", "pressure RR");
                mPressureRL = param.getNum("strategy wet 15", "pressure RL");
                mCamberFR = param.getNum("strategy wet 15", "camber FR");
                mCamberFL = param.getNum("strategy wet 15", "camber FL");
                mCamberRR = param.getNum("strategy wet 15", "camber RR");
                mCamberRL = param.getNum("strategy wet 15", "camber RL");
            }
            else if (temperature < 20)
            {
                mPressureFR = param.getNum("strategy wet 20", "pressure FR");
                mPressureFL = param.getNum("strategy wet 20", "pressure FL");
                mPressureRR = param.getNum("strategy wet 20", "pressure RR");
                mPressureRL = param.getNum("strategy wet 20", "pressure RL");
                mCamberFR = param.getNum("strategy wet 20", "camber FR");
                mCamberFL = param.getNum("strategy wet 20", "camber FL");
                mCamberRR = param.getNum("strategy wet 20", "camber RR");
                mCamberRL = param.getNum("strategy wet 20", "camber RL");
            }
            else if (temperature < 25)
            {
                mPressureFR = param.getNum("strategy wet 25", "pressure FR");
                mPressureFL = param.getNum("strategy wet 25", "pressure FL");
                mPressureRR = param.getNum("strategy wet 25", "pressure RR");
                mPressureRL = param.getNum("strategy wet 25", "pressure RL");
                mCamberFR = param.getNum("strategy wet 25", "camber FR");
                mCamberFL = param.getNum("strategy wet 25", "camber FL");
                mCamberRR = param.getNum("strategy wet 25", "camber RR");
                mCamberRL = param.getNum("strategy wet 25", "camber RL");
            }
            else if (temperature < 30)
            {
                mPressureFR = param.getNum("strategy wet 30", "pressure FR");
                mPressureFL = param.getNum("strategy wet 30", "pressure FL");
                mPressureRR = param.getNum("strategy wet 30", "pressure RR");
                mPressureRL = param.getNum("strategy wet 30", "pressure RL");
                mCamberFR = param.getNum("strategy wet 30", "camber FR");
                mCamberFL = param.getNum("strategy wet 30", "camber FL");
                mCamberRR = param.getNum("strategy wet 30", "camber RR");
                mCamberRL = param.getNum("strategy wet 30", "camber RL");
            }
            else
            {
                mPressureFR = param.getNum("strategy wet 35", "pressure FR");
                mPressureFL = param.getNum("strategy wet 35", "pressure FL");
                mPressureRR = param.getNum("strategy wet 35", "pressure RR");
                mPressureRL = param.getNum("strategy wet 35", "pressure RL");
                mCamberFR = param.getNum("strategy wet 35", "camber FR");
                mCamberFL = param.getNum("strategy wet 35", "camber FL");
                mCamberRR = param.getNum("strategy wet 35", "camber RR");
                mCamberRL = param.getNum("strategy wet 35", "camber RL");
            }
        }
        else
        {
            LogAXIOM.debug("Compound EXTREM WET !!!\n");
            if (temperature < 8)
            {
                mPressureFR = param.getNum("strategy extrem wet 8", "pressure FR");
                mPressureFL = param.getNum("strategy extrem wet 8", "pressure FL");
                mPressureRR = param.getNum("strategy extrem wet 8", "pressure RR");
                mPressureRL = param.getNum("strategy extrem wet 8", "pressure RL");
                mCamberFR = param.getNum("strategy extrem wet 8", "camber FR");
                mCamberFL = param.getNum("strategy extrem wet 8", "camber FL");
                mCamberRR = param.getNum("strategy extrem wet 8", "camber RR");
                mCamberRL = param.getNum("strategy extrem wet 8", "camber RL");
            }
            else if (temperature < 15)
            {
                mPressureFR = param.getNum("strategy extrem wet 15", "pressure FR");
                mPressureFL = param.getNum("strategy extrem wet 15", "pressure FL");
                mPressureRR = param.getNum("strategy extrem wet 15", "pressure RR");
                mPressureRL = param.getNum("strategy extrem wet 15", "pressure RL");
                mCamberFR = param.getNum("strategy extrem wet 15", "camber FR");
                mCamberFL = param.getNum("strategy extrem wet 15", "camber FL");
                mCamberRR = param.getNum("strategy extrem wet 15", "camber RR");
                mCamberRL = param.getNum("strategy extrem wet 15", "camber RL");
            }
            else if (temperature < 20)
            {
                mPressureFR = param.getNum("strategy extrem wet 20", "pressure FR");
                mPressureFL = param.getNum("strategy extrem wet 20", "pressure FL");
                mPressureRR = param.getNum("strategy extrem wet 20", "pressure RR");
                mPressureRL = param.getNum("strategy extrem wet 20", "pressure RL");
                mCamberFR = param.getNum("strategy extrem wet 20", "camber FR");
                mCamberFL = param.getNum("strategy extrem wet 20", "camber FL");
                mCamberRR = param.getNum("strategy eextrem wetxtrem wet 20", "camber RR");
                mCamberRL = param.getNum("strategy wet 20", "camber RL");
            }
            else if (temperature < 25)
            {
                mPressureFR = param.getNum("strategy extrem wet 25", "pressure FR");
                mPressureFL = param.getNum("strategy extrem wet 25", "pressure FL");
                mPressureRR = param.getNum("strategy extrem wet 25", "pressure RR");
                mPressureRL = param.getNum("strategy extrem wet 25", "pressure RL");
                mCamberFR = param.getNum("strategy extrem wet 25", "camber FR");
                mCamberFL = param.getNum("strategy extrem wet 25", "camber FL");
                mCamberRR = param.getNum("strategy extrem wet 25", "camber RR");
                mCamberRL = param.getNum("strategy extrem wet 25", "camber RL");
            }
            else if (temperature < 30)
            {
                mPressureFR = param.getNum("strategy extrem wet 30", "pressure FR");
                mPressureFL = param.getNum("strategy extrem wet 30", "pressure FL");
                mPressureRR = param.getNum("strategy extrem wet 30", "pressure RR");
                mPressureRL = param.getNum("strategy extrem wet 30", "pressure RL");
                mCamberFR = param.getNum("strategy extrem wet 30", "camber FR");
                mCamberFL = param.getNum("strategy extrem wet 30", "camber FL");
                mCamberRR = param.getNum("strategy extrem wet 30", "camber RR");
                mCamberRL = param.getNum("strategy extrem wet 30", "camber RL");
            }
            else
            {
                mPressureFR = param.getNum("strategy extrem wet 35", "pressure FR");
                mPressureFL = param.getNum("strategy extrem wet 35", "pressure FL");
                mPressureRR = param.getNum("strategy extrem wet 35", "pressure RR");
                mPressureRL = param.getNum("strategy extrem wet 35", "pressure RL");
                mCamberFR = param.getNum("strategy extrem wet 35", "camber FR");
                mCamberFL = param.getNum("strategy extrem wet 35", "camber FL");
                mCamberRR = param.getNum("strategy extrem wet 35", "camber RR");
                mCamberRL = param.getNum("strategy extrem wet 35", "camber RL");
            }
        }

        LogAXIOM.debug("# front wheel right pressure = %.2f\n", mPressureFR / 6894.76f);
        LogAXIOM.debug("# front wheel left  pressure = %.2f\n", mPressureFL / 6894.76f);
        LogAXIOM.debug("# rear wheel right pressure  = %.2f\n", mPressureRR / 6894.76f);
        LogAXIOM.debug("# rear wheel left  pressure  = %.2f\n", mPressureRL / 6894.76f);
        LogAXIOM.debug("# front wheel right camber   = %.2f\n", mCamberFR );
        LogAXIOM.debug("# front wheel left  camber   = %.2f\n", mCamberFL);
        LogAXIOM.debug("# rear wheel right camber    = %.2f\n", mCamberRR );
        LogAXIOM.debug("# rear wheel left  camber    = %.2f\n", mCamberRL );
    }
}

void CarParams::readVarSpecs(const MyParam& param)
{
    if (mOptionWings == 0)
        mRearWingAngle = param.getNum(SECT_REARWING, PRM_WINGANGLE);
    else if (mOptionWings == 1)
        mRearWingAngle = param.getNum(SECT_REARWING1, PRM_WINGANGLE);
    else
        mRearWingAngle = param.getNum(SECT_REARWING2, PRM_WINGANGLE);

    LogAXIOM.debug("# Car Rear Wing angle = %.2f\n", mRearWingAngle);
}

void CarParams::readConstSpecs(void* CarHandle)
{
    const char *enabling;
    double tiremuFL, tiremuFR, tiremuRL, tiremuRR, tiremufront, tiremurear;

    enabling = GfParmGetStr(CarHandle, SECT_FEATURES, PRM_TIRECOMPOUNDS, VAL_NO);

    if (strcmp(enabling, VAL_YES) == 0)
    {
        HASCPD = true;
        LogAXIOM.debug("#Car has Compounds yes\n");
    }
    else
        LogAXIOM.debug("#Car has Compounds no\n");

    enabling = GfParmGetStr(CarHandle, SECT_FEATURES, PRM_ABSINSIMU, VAL_NO);

    if (strcmp(enabling, VAL_YES) == 0)
    {
        HASABS = true;
        LogAXIOM.debug("#Car has ABS yes\n");
    }
    else
        LogAXIOM.debug("#Car has ABS no\n");

    enabling = GfParmGetStr(CarHandle, SECT_FEATURES, PRM_ESPINSIMU, VAL_NO);

    if (strcmp(enabling, VAL_YES) == 0)
    {
        HASESP = true;
        LogAXIOM.debug("#Car has ESP yes\n");
    }
    else
        LogAXIOM.debug("#Car has ESP no\n");

    enabling = GfParmGetStr(CarHandle, SECT_FEATURES, PRM_TCLINSIMU, VAL_NO);

    if (strcmp(enabling, VAL_YES) == 0)
    {
        HASTCL = true;
        LogAXIOM.debug("#Car has TCL yes\n");
    }
    else
        LogAXIOM.debug("#Car has TCL no\n");

    if (mOptionMass == 0)
    {
        mCarMass = GfParmGetNum(CarHandle, SECT_CAR, PRM_MASS, nullptr, 0);
    }
    else if (mOptionMass == 1)
    {
        mCarMass = GfParmGetNum(CarHandle, SECT_CAR, PRM_MASS1, nullptr, 0);
    }
    else if (mOptionMass == 2)
    {
        mCarMass = GfParmGetNum(CarHandle, SECT_CAR, PRM_MASS2, nullptr, 0);
    }
    else
        mCarMass = GfParmGetNum(CarHandle, SECT_CAR, PRM_MASS3, nullptr, 0);

    mTankVol = GfParmGetNum(CarHandle, SECT_CAR, PRM_TANK, (char*)nullptr, 0);

    if(mOptionTires == 0)
    {
        tiremuFL = GfParmGetNum(CarHandle, SECT_FRNTLFTWHEEL, PRM_MU, nullptr, 0);
        tiremuFR = GfParmGetNum(CarHandle, SECT_FRNTRGTWHEEL, PRM_MU, nullptr, 0);
        tiremuRL = GfParmGetNum(CarHandle, SECT_REARLFTWHEEL, PRM_MU, nullptr, 0);
        tiremuRR = GfParmGetNum(CarHandle, SECT_REARRGTWHEEL, PRM_MU, nullptr, 0);
        tiremufront = std::min(tiremuFL, tiremuFR);
        tiremurear = std::min(tiremuRL, tiremuRR);
    }
    else if (mOptionTires == 1)
    {
        tiremuFL = GfParmGetNum(CarHandle, SECT_FRNTLFTWHEEL1, PRM_MU, nullptr, 0);
        tiremuFR = GfParmGetNum(CarHandle, SECT_FRNTRGTWHEEL1, PRM_MU, nullptr, 0);
        tiremuRL = GfParmGetNum(CarHandle, SECT_REARLFTWHEEL1, PRM_MU, nullptr, 0);
        tiremuRR = GfParmGetNum(CarHandle, SECT_REARRGTWHEEL1, PRM_MU, nullptr, 0);
        tiremufront = std::min(tiremuFL, tiremuFR);
        tiremurear = std::min(tiremuRL, tiremuRR);
    }
    else
    {
        tiremuFL = GfParmGetNum(CarHandle, SECT_FRNTLFTWHEEL2, PRM_MU, nullptr, 0);
        tiremuFR = GfParmGetNum(CarHandle, SECT_FRNTRGTWHEEL2, PRM_MU, nullptr, 0);
        tiremuRL = GfParmGetNum(CarHandle, SECT_REARLFTWHEEL2, PRM_MU, nullptr, 0);
        tiremuRR = GfParmGetNum(CarHandle, SECT_REARRGTWHEEL2, PRM_MU, nullptr, 0);
        tiremufront = std::min(tiremuFL, tiremuFR);
        tiremurear = std::min(tiremuRL, tiremuRR);
    }

    mTireMu = std::min(tiremufront, tiremurear);

    if (HASCPD)
    {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s/%s", SECT_FRNTRGTWHEEL, SECT_COMPOUNDS, SECT_SOFT);
        tiremuFR = GfParmGetNum(CarHandle, path, PRM_MU, (char*)NULL, mTireMu);
        snprintf(path, sizeof(path), "%s/%s/%s", SECT_FRNTLFTWHEEL, SECT_COMPOUNDS, SECT_SOFT);
        tiremuFL = GfParmGetNum(CarHandle, path, PRM_MU, (char*)NULL, mTireMu);
        snprintf(path, sizeof(path), "%s/%s/%s", SECT_REARRGTWHEEL, SECT_COMPOUNDS, SECT_SOFT);
        tiremuRR = GfParmGetNum(CarHandle, path, PRM_MU, (char*)NULL, mTireMu);
        snprintf(path, sizeof(path), "%s/%s/%s", SECT_REARLFTWHEEL, SECT_COMPOUNDS, SECT_SOFT);
        tiremuRL = GfParmGetNum(CarHandle, path, PRM_MU, (char*)NULL, mTireMu);
        tiremufront = MIN(tiremuFL, tiremuFR);
        tiremurear = MIN(tiremuRL, tiremuRR);
        mTireMuC[1] = MIN(tiremufront, tiremurear);

        snprintf(path, sizeof(path), "%s/%s/%s", SECT_FRNTRGTWHEEL, SECT_COMPOUNDS, SECT_MEDIUM);
        tiremuFR = GfParmGetNum(CarHandle, path, PRM_MU, (char*)NULL, mTireMu);
        snprintf(path, sizeof(path), "%s/%s/%s", SECT_FRNTLFTWHEEL, SECT_COMPOUNDS, SECT_MEDIUM);
        tiremuFL = GfParmGetNum(CarHandle, path, PRM_MU, (char*)NULL, mTireMu);
        snprintf(path, sizeof(path), "%s/%s/%s", SECT_REARRGTWHEEL, SECT_COMPOUNDS, SECT_MEDIUM);
        tiremuRR = GfParmGetNum(CarHandle, path, PRM_MU, (char*)NULL, mTireMu);
        snprintf(path, sizeof(path), "%s/%s/%s", SECT_REARLFTWHEEL, SECT_COMPOUNDS, SECT_MEDIUM);
        tiremuRL = GfParmGetNum(CarHandle, path, PRM_MU, (char*)NULL, mTireMu);
        tiremufront = MIN(tiremuFL, tiremuFR);
        tiremurear = MIN(tiremuRL, tiremuRR);
        mTireMuC[2] = MIN(tiremufront, tiremurear);

        snprintf(path, sizeof(path), "%s/%s/%s", SECT_FRNTRGTWHEEL, SECT_COMPOUNDS, SECT_HARD);
        tiremuFR = GfParmGetNum(CarHandle, path, PRM_MU, (char*)NULL, mTireMu);
        snprintf(path, sizeof(path), "%s/%s/%s", SECT_FRNTLFTWHEEL, SECT_COMPOUNDS, SECT_HARD);
        tiremuFL = GfParmGetNum(CarHandle, path, PRM_MU, (char*)NULL, mTireMu);
        snprintf(path, sizeof(path), "%s/%s/%s", SECT_REARRGTWHEEL, SECT_COMPOUNDS, SECT_HARD);
        tiremuRR = GfParmGetNum(CarHandle, path, PRM_MU, (char*)NULL, mTireMu);
        snprintf(path, sizeof(path), "%s/%s/%s", SECT_REARLFTWHEEL, SECT_COMPOUNDS, SECT_HARD);
        tiremuRL = GfParmGetNum(CarHandle, path, PRM_MU, (char*)NULL, mTireMu);
        tiremufront = MIN(tiremuFL, tiremuFR);
        tiremurear = MIN(tiremuRL, tiremuRR);
        mTireMuC[3] = MIN(tiremufront, tiremurear);

        snprintf(path, sizeof(path), "%s/%s/%s", SECT_FRNTRGTWHEEL, SECT_COMPOUNDS, SECT_WET);
        tiremuFR = GfParmGetNum(CarHandle, path, PRM_MU, (char*)NULL, mTireMu);
        snprintf(path, sizeof(path), "%s/%s/%s", SECT_FRNTLFTWHEEL, SECT_COMPOUNDS, SECT_WET);
        tiremuFL = GfParmGetNum(CarHandle, path, PRM_MU, (char*)NULL, mTireMu);
        snprintf(path, sizeof(path), "%s/%s/%s", SECT_REARRGTWHEEL, SECT_COMPOUNDS, SECT_WET);
        tiremuRR = GfParmGetNum(CarHandle, path, PRM_MU, (char*)NULL, mTireMu);
        snprintf(path, sizeof(path), "%s/%s/%s", SECT_REARLFTWHEEL, SECT_COMPOUNDS, SECT_WET);
        tiremuRL = GfParmGetNum(CarHandle, path, PRM_MU, (char*)NULL, mTireMu);
        tiremufront = MIN(tiremuFL, tiremuFR);
        tiremurear = MIN(tiremuRL, tiremuRR);
        mTireMuC[4] = MIN(tiremufront, tiremurear);

        snprintf(path, sizeof(path), "%s/%s/%s", SECT_FRNTRGTWHEEL, SECT_COMPOUNDS, SECT_EXTREM_WET);
        tiremuFR = GfParmGetNum(CarHandle, path, PRM_MU, (char*)NULL, mTireMu);
        snprintf(path, sizeof(path), "%s/%s/%s", SECT_FRNTLFTWHEEL, SECT_COMPOUNDS, SECT_EXTREM_WET);
        tiremuFL = GfParmGetNum(CarHandle, path, PRM_MU, (char*)NULL, mTireMu);
        snprintf(path, sizeof(path), "%s/%s/%s", SECT_REARRGTWHEEL, SECT_COMPOUNDS, SECT_EXTREM_WET);
        tiremuRR = GfParmGetNum(CarHandle, path, PRM_MU, (char*)NULL, mTireMu);
        snprintf(path, sizeof(path), "%s/%s/%s", SECT_REARLFTWHEEL, SECT_COMPOUNDS, SECT_EXTREM_WET);
        tiremuRL = GfParmGetNum(CarHandle, path, PRM_MU, (char*)NULL, mTireMu);
        tiremufront = MIN(tiremuFL, tiremuFR);
        tiremurear = MIN(tiremuRL, tiremuRR);
        mTireMuC[5] = MIN(tiremufront, tiremurear);

        int compounds = GfParmGetNum(CarHandle, SECT_TIRESET, PRM_COMPOUNDS_SET, NULL, 1);
        mTireMu = mTireMuC[compounds];
        LogAXIOM.debug("# AXIOM tire mu = %.2f\n", mTireMu);
    }

    mBrakePressMax = GfParmGetNum(CarHandle, SECT_BRKSYST, PRM_BRKPRESS, (char*)nullptr, 0);
    mBrakeRepartition = GfParmGetNum(CarHandle, SECT_BRKSYST, PRM_BRKREP, (char*)nullptr, 0);

    if (mOptionWings == 0)
        mFrontWingAngle = GfParmGetNum(CarHandle, SECT_FRNTWING, PRM_WINGANGLE, (char*)nullptr, 0);
    else if (mOptionWings == 1)
        mFrontWingAngle = GfParmGetNum(CarHandle, SECT_FRNTWING1, PRM_WINGANGLE, (char*)nullptr, 0);
    else
        mFrontWingAngle = GfParmGetNum(CarHandle, SECT_FRNTWING2, PRM_WINGANGLE, (char*)nullptr, 0);

    LogAXIOM.debug("# Car MASS              = %.2f\n", mCarMass);
    LogAXIOM.debug("# Tank Volume           = %.2f\n", mTankVol);
    LogAXIOM.debug("# Tire MU               = %.2f\n", mTireMu);
    LogAXIOM.debug("# Brake Press Max       = %.2f\n", mBrakePressMax);
    LogAXIOM.debug("# Brake repartition     = %.2f\n", mBrakeRepartition);
    LogAXIOM.debug("# Front Wing Angle      = %.2f\n", mFrontWingAngle);
}

void CarParams::initVars()
{
    mShiftTimer = 0;
    mAbsFactor = 1.0;
    mTclFactor = 1.0;
    mAccel = 0.0;
}

void CarParams::initCa()
{
    char* WheelSect[4] = {(char*)SECT_FRNTRGTWHEEL, (char*)SECT_FRNTLFTWHEEL, (char*)SECT_REARRGTWHEEL, (char*)SECT_REARLFTWHEEL};

    double frontwingarea, rearwingarea;

    if (mOptionWings == 0)
    {
        frontwingarea = GfParmGetNum(mCar->_carHandle, SECT_FRNTWING, PRM_WINGAREA, (char*)nullptr, 0);
        rearwingarea = GfParmGetNum(mCar->_carHandle, SECT_REARWING, PRM_WINGAREA, (char*)nullptr, 0);
    }
    else if (mOptionWings == 1)
    {
        frontwingarea = GfParmGetNum(mCar->_carHandle, SECT_FRNTWING1, PRM_WINGAREA, (char*)nullptr, 0);
        rearwingarea = GfParmGetNum(mCar->_carHandle, SECT_REARWING1, PRM_WINGAREA, (char*)nullptr, 0);
    }
    else
    {
        frontwingarea = GfParmGetNum(mCar->_carHandle, SECT_FRNTWING2, PRM_WINGAREA, (char*)nullptr, 0);
        rearwingarea = GfParmGetNum(mCar->_carHandle, SECT_REARWING2, PRM_WINGAREA, (char*)nullptr, 0);
    }

    double frontclift = GfParmGetNum(mCar->_carHandle, SECT_AERODYNAMICS, PRM_FCL, (char*)nullptr, 0);
    double rearclift = GfParmGetNum(mCar->_carHandle, SECT_AERODYNAMICS, PRM_RCL, (char*)nullptr, 0);
    double frntwingca = 1.23 * frontwingarea * sin(mFrontWingAngle);
    double rearwingca = 1.23 * rearwingarea * sin(mRearWingAngle);
    double h = 0.0;

    for (int i = 0; i < 4; i++)
    {
        h += GfParmGetNum(mCar->_carHandle, WheelSect[i], PRM_RIDEHEIGHT, (char*)nullptr, 0);
    }

    h*= 1.5; h = h * h; h = h * h; h = 2.0 * exp(-3.0 * h);
    mFrontCA = h * frontclift + 4.0 * frntwingca;
    mRearCA = h * rearclift + 4.0 * rearwingca;
    mCA = 2.0 * std::min(mFrontCA, mRearCA);

    LogAXIOM.debug("# Front Wing AREA       = %.2f\n", frontwingarea);
    LogAXIOM.debug("# Rear Wing AREA        = %.2f\n", rearwingarea);
    LogAXIOM.debug("# Front Clift           = %.2f\n", frontclift);
    LogAXIOM.debug("# Rear Clift            = %.2f\n", rearclift);
    LogAXIOM.debug("# Front Wing CA         = %.2f\n", frntwingca);
    LogAXIOM.debug("# Rear Wing CA          = %.2f\n", rearwingca);
    LogAXIOM.debug("# Front CA              = %.2f\n", mFrontCA);
    LogAXIOM.debug("# Rear Wing CA          = %.2f\n", mRearCA);
    LogAXIOM.debug("# CA                    = %.2f\n", mCA);
}

void CarParams::initCw()
{
    double frontwingarea, rearwingarea;
    double cx = GfParmGetNum(mCar->_carHandle, SECT_AERODYNAMICS, PRM_CX, (char*)nullptr, 0);
    double frontarea = GfParmGetNum(mCar->_carHandle, SECT_AERODYNAMICS, PRM_FRNTAREA, (char*)nullptr, 0);
    mBodyCW = 0.645 * cx * frontarea;

    if (mOptionWings == 0)
    {
        frontwingarea = GfParmGetNum(mCar->_carHandle, SECT_FRNTWING, PRM_WINGAREA, (char*)nullptr, 0);
        rearwingarea = GfParmGetNum(mCar->_carHandle, SECT_REARWING, PRM_WINGAREA, (char*)nullptr, 0);
    }
    else if (mOptionWings == 1)
    {
        frontwingarea = GfParmGetNum(mCar->_carHandle, SECT_FRNTWING1, PRM_WINGAREA, (char*)nullptr, 0);
        rearwingarea = GfParmGetNum(mCar->_carHandle, SECT_REARWING1, PRM_WINGAREA, (char*)nullptr, 0);
    }
    else
    {
        frontwingarea = GfParmGetNum(mCar->_carHandle, SECT_FRNTWING2, PRM_WINGAREA, (char*)nullptr, 0);
        rearwingarea = GfParmGetNum(mCar->_carHandle, SECT_REARWING2, PRM_WINGAREA, (char*)nullptr, 0);
    }

    double frntwingca = 1.23 * frontwingarea * sin(mFrontWingAngle);
    double rearwingca = 1.23 * rearwingarea * sin(mRearWingAngle);
    mWingCW = frntwingca + rearwingca;

    mCW = mBodyCW + mWingCW;

    LogAXIOM.debug("# CX                    = %.2f\n", cx);
    LogAXIOM.debug("# Front Area            = %.2f\n", frontarea);
    LogAXIOM.debug("# Body CW               = %.2f\n", mBodyCW);
    LogAXIOM.debug("# Wing CW               = %.2f\n", mWingCW);
    LogAXIOM.debug("# CW                    = %.2f\n", mCW);
}

void CarParams::initBrakes()
{
    double frontpistonarea = GfParmGetNum(mCar->_carHandle, SECT_FRNTRGTBRAKE, PRM_BRKAREA, (char*)nullptr, 0);
    double rearpistonarea = GfParmGetNum(mCar->_carHandle, SECT_REARRGTBRAKE, PRM_BRKAREA, (char*)nullptr, 0);
    double frontdiskmu = GfParmGetNum(mCar->_carHandle, SECT_FRNTRGTBRAKE, PRM_MU, (char*)nullptr, 0);
    double reardiskmu = GfParmGetNum(mCar->_carHandle, SECT_REARRGTBRAKE, PRM_MU, (char*)nullptr, 0);
    double maxf = 2.0 * mBrakeRepartition * mBrakePressMax * mCar->_brakeDiskRadius(0) * frontpistonarea * frontdiskmu / mCar->_wheelRadius(0);
    double maxr = 2.0 * (1 - mBrakeRepartition) * mBrakePressMax * mCar->_brakeDiskRadius(2) * rearpistonarea * reardiskmu / mCar->_wheelRadius(2);
    mBrakeForceMax = maxf + maxr;
}

int CarParams::calcCompound(double temp, double distance, int laps)
{
    LogAXIOM.debug("#Calcul compound - temperature = %.2f - Distance = %.2f - Laps = %i!\n", temp, distance, laps);
    if((temp < 8.00 || laps <= 5) && mRain < 2)
    {
        LogAXIOM.debug("Choice 1\n");
        return 1;
    }
    else if ((temp <= 20.00 || laps < 30) && mRain < 2)
    {
        LogAXIOM.debug("Choice 2\n");
        return 2;
    }
    else if (mRain < 2)
    {
        LogAXIOM.debug("Choice 3\n");
        return 3;
    }
    else if (mRain < 3)
    {
        LogAXIOM.debug("Choice 4\n");
        return 4;
    }
    else
    {
        LogAXIOM.debug("Choice 5\n");
        return 5;
    }
}

double CarParams::calcFuel(double dist) const
{
    double tiredist = dist / mTireWearPerMeter;
    LogAXIOM.debug("Tire distance : %.7f\n", tiredist);
    double mindist = MIN(dist, tiredist);
    LogAXIOM.debug("Minimum distance : %.3f\n", mindist);
    double fuel = mindist * mFuelPerMeter;
    LogAXIOM.debug("calcul fuel : %.3f\n", fuel);

    return Utils::clip(fuel, 0.0, mTankVol);
}

double CarParams::calcFuel2(double dist) const
{
    double tiredist = dist / mTireWearPerMeter;
    LogAXIOM.debug("Tire distance : %.7f\n", tiredist);
    double mindist = MIN(dist, tiredist);
    LogAXIOM.debug("Minimum distance : %.3f\n", mindist);
    double fuel = mindist * mFuelPerMeter;
    LogAXIOM.debug("calcul fuel : %.3f\n", fuel);

    return Utils::clip(fuel, 0.0, mTankVol);
}

void CarParams::update(double dt)
{
    mDeltaTime = dt;
    mMass = mCarMass + mFuelWeightFactor * mCar->_fuel;
    mSpeedX = mCar->_speed_x;
    mWheels.update();
    mSegMu = mWheels.gripFactor() * mTireMu * mCar->_trkPos.seg->surface->kFriction;
    LogAXIOM.debug("Tyre Condition = %.8f- Tyre temperature = %.3f\n", mWheels.TyreCondition(), K2C(mCar->priv.wheel[0].temp_mid));
    LogAXIOM.debug("# AXIOM CarParams Tire Mu = %.3f - SegMu = %.3f\n", mTireMu, mSegMu);
    mCW = mBodyCW * (1.0 + mCar->_dammage / 10000.0) + mWingCW;
    mToMiddle = mCar->_trkPos.toMiddle;
    double yawdiff = Utils::normPiPi(mCar->_yaw - mYaw);
    mYaw = mCar->_yaw;
    mHeading = Vec3d(cos(mYaw), sin(mYaw), 0.0);
    Vec3d prevGlobalPos = mGlobalPos;
    mGlobalPos = Vec3d(mCar->_pos_X, mCar->_pos_Y, mCar->_pos_Z);
    mFrontAxleGlobalPos = mGlobalPos + mHeading * mFrontAxleOffset;
    Vec3d speedv = (mGlobalPos - prevGlobalPos) / dt;
    mSpeed = speedv.len();
    mSpeedYaw = speedv.getVec2().angle();
    double distdiff = (mGlobalPos - prevGlobalPos).len();

    if (distdiff > 0.05)
    {
        mYawRate = yawdiff / distdiff;
    }
    else
    {
        mYawRate = 0.0;
    }

    mAngleToTrack = Utils::normPiPi(mTrack->yaw(mCar->_distFromStartLine) - mYaw);
    mBorderDist = mCar->_trkPos.seg->width / 2.0 - fabs(mToMiddle) - mCar->_dimension_y / 2.0;
    mDamageDiff = mCar->_dammage - mLastDamage;
    mLastDamage = mCar->_dammage;
    bool mOnLeftSide = toMid() > 0.0 ? true : false;

    if (mCar->_trkPos.seg->side[mOnLeftSide] != nullptr)
    {
        mBorderFriction = mCar->_trkPos.seg->side[mOnLeftSide]->surface->kFriction;
    }
    else
    {
        mBorderFriction = 1.0;
    }

    bool mAngleToLeft = mAngleToTrack < 0.0 ? true : false;

    if (gear() == -1)
    {
        mPointingToWall = (mAngleToLeft != mOnLeftSide) ? true : false;
    }
    else
    {
        mPointingToWall = (mAngleToLeft == mOnLeftSide) ? true : false;
    }

    mWallToMiddleAbs = mCar->_trkPos.seg->width / 2.0;

    if (mCar->_trkPos.seg->side[mOnLeftSide] != nullptr)
    {
        int style = mCar->_trkPos.seg->side[mOnLeftSide]->style;

        if (style < 2)
        {
            mWallToMiddleAbs += mCar->_trkPos.seg->side[mOnLeftSide]->width;

            if (mCar->_trkPos.seg->side[mOnLeftSide]->side[mOnLeftSide] != nullptr)
            {
                mWallToMiddleAbs += mCar->_trkPos.seg->side[mOnLeftSide]->side[mOnLeftSide]->width;
            }
        }
    }

    mWalldist = mWallToMiddleAbs - fabs(toMid());

    mAccelFilter.sample(20, mAccel);
    mMaxAccelForce = mCar->_gearRatio[mCar->_gear + mCar->_gearOffset] * mCar->_engineMaxTq / mCar->_wheelRadius(0);
    mSideSlip = mCar->_wheelSlipSide(0) + mCar->_wheelSlipSide(1) + mCar->_wheelSlipSide(2) + mCar->_wheelSlipSide(3);
}

void CarParams::setControls(double accel, double brake, double steer)
{
    mAccel = accel;

    mCar->_accelCmd = (tdble)accel;
    mCar->_brakeCmd = (tdble)brake;
    mCar->_steerCmd = (tdble)steer;
    mCar->_gearCmd = calcGear();
    mCar->_clutchCmd = (tdble)calcClutch();  // must be after gear
    mCar->_lightCmd = (RM_LIGHT_HEAD1 | RM_LIGHT_HEAD2);
}

double CarParams::brakeForce(double speed, double curvature, double curv_z, double mu, double pitchAngle, double rollAngle, PathType pathtype) const
{
    double friction = mu * (mMass * 9.81 * (1.0 + sin(pitchAngle) + sin(rollAngle)) + mCA * speed * speed);
    double centrifugal = mMass * speed * speed * fabs(curvature) * (1.0 - sin(rollAngle));
    centrifugal = std::min(centrifugal, friction);
    double brakeforce = std::max(0.03 * maxBrakeForce(), sqrt(friction * friction - centrifugal * centrifugal));

    return std::min(brakeforce, maxBrakeForce());
}

double CarParams::curveSpeed(double curvature, double curv_z, double mu, double rollAngle, PathType pathtype) const
{
    curvature = fabs(curvature);

    if (curv_z < -0.002)
    {
        double factor = 1.8;

        if (pathtype != PATH_O)
        {
            factor = 2.5;
        }

        curvature = -curv_z * factor + curvature;
    }

    double radius = Utils::calcRadius(curvature);

    return sqrt(mu * 9.81 * (1.0 + sin(rollAngle)) * radius / (1.0 - std::min(0.99, radius * mCA * mu / mMass)));
}

double CarParams::bumpSpeed(double curv_z) const
{
    double speed_z = DBL_MAX;

    if (curv_z < -0.002)
    {
        speed_z = mBumpSpeedFactor * sqrt(9.81 / -(curv_z));
    }

    return speed_z;
}

int CarParams::calcGear()
{
    double SHIFT_UP = 0.98; // [-] (% of rpmredline)
    double SHIFT_DOWN_MARGIN = 130.0; // [rad/s] down from rpmredline
    int shifttime = 25;
    int MAX_GEAR = mCar->_gearNb - 1;

    if (v() < 1.0)
    {
        // For the start
        shifttime = 0;
    }

    if (mShiftTimer < shifttime)
    {
        mShiftTimer++;
    }

    if (mShiftTimer < shifttime)
    {
        return gear();
    }
    if (mGearDirection == -1)
    {
        return -1;
    }

    if (gear() <= 0)
    {
        return 1;
    }

    if (gear() < MAX_GEAR && mCar->_enginerpm / mCar->_enginerpmRedLine > SHIFT_UP)
    {
        mShiftTimer = 0;

        return gear() + 1;
    }
    else
    {
        double ratiodown = mCar->_gearRatio[gear() + mCar->_gearOffset - 1] / mCar->_gearRatio[gear() + mCar->_gearOffset];

        if (gear() > 1 && (mCar->_enginerpmRedLine - SHIFT_DOWN_MARGIN) / mCar->_enginerpm > ratiodown)
        {
            mShiftTimer = 0;

            return gear() - 1;
        }
    }

    return gear();
}

double CarParams::calcClutch()
{
    if (gear() > 1 || v() > 15.0)
    {
        if (gear() > mPrevGear)
        {
            mClutch = 0.3;
        }

        if (mCar->_enginerpm / mCar->_enginerpmRedLine > 0.7)
        {
            mClutch -= 0.04;
        }
        else
        {
            mClutch += 0.04;
        }

        if (gear() < mPrevGear)
        {
            mClutch = 0.0;
        }
    }
    else if (gear() == 1)
    {
        if (mCar->_enginerpm / mCar->_enginerpmRedLine > 0.7)
        {
            mClutch -= 0.04;
        }
        else
        {
            mClutch += 0.04;
        }

        if (fabs(mAngleToTrack) > 1.0 || mBorderDist < -2.0)
        {
            mClutch = 0.0;
        }
    }
    else if (gear() == 0)
    {
        // For a good start
        mClutch = 0.7;
    }
    else if (gear() == -1)
    {
        // For the reverse gear.
        if (mCar->_enginerpm > 500.0)
        {
            mClutch -= 0.01;
        }
        else
        {
            mClutch += 0.01;
        }
    }

    mPrevGear = gear();
    mClutch = Utils::clip(mClutch, 0.0, 1.0);

    return mClutch;
}

double CarParams::filterABS(double brake)
{
    double ABS_MINSPEED = 3.0;      // [m/s]
    if (mSpeed < ABS_MINSPEED)
    {
        return brake;
    }

    mSlip = slipFront() + slipRear();

    if (mAccel > 0.0)
    {
        mAbsFactor = 0.8;
    }

    if (fabs(mSideSlip) > 30.0)
    {
        mAbsFactor -= 0.1;
    }
    else if (mSlip < -mAbsSlip)
    { // TODO: use a ABS controller
        mAbsFactor -= 0.1;
    }
    else
    {
        mAbsFactor += 0.1;
    }

    mAbsFactor = Utils::clip(mAbsFactor, 0.1, 1.0);

    return brake *= mAbsFactor;
}

double CarParams::filterTCL(double accel)
{
    double factor = 3.1;

    if (fabs(mCar->_steerCmd) > 0.2)
    {
        factor = 2.5 * mWheels.gripFactor();
    }

    double TCL_SLIP = factor * mWheels.gripFactor();

    double diff = std::max(slipFront() - TCL_SLIP, slipRear() - TCL_SLIP);
    mTclController.mP = 0.19;
    mTclController.mD = 0.002;
    mTclFactor -= mTclController.sample(diff, mDeltaTime);
    mTclFactor = Utils::clip(mTclFactor, 0.0, 1.0);

    return accel *= mTclFactor;
}

double CarParams::slipFront() const
{
    return (mCar->_wheelSpinVel(FRNT_RGT) + mCar->_wheelSpinVel(FRNT_LFT)) * mCar->_wheelRadius(FRNT_LFT) / 2.0 - mSpeedX;
}

double CarParams::slipRear() const
{
    return (mCar->_wheelSpinVel(REAR_RGT) + mCar->_wheelSpinVel(REAR_LFT)) * mCar->_wheelRadius(REAR_LFT) / 2.0 - mSpeedX;
}

double CarParams::filterTCLSideSlip(double accel) const
{
    if (mCar->_remainingLaps < 5 && mWheels.wear() < 0.8)
    {
        double slipQualy = MAX(mSideSlipTCL, mSideSlipTCLQualy);

        if (fabs(mSideSlip) > slipQualy * 2)
            return 0.0;
        else if (fabs(mSideSlip) > slipQualy)
            return accel * ((1.0 - MIN(0.7, (fabs(mSideSlip) - slipQualy) * mSideSlipTCLFactor)));
    }
    else
    {
        if (fabs(mSideSlip) > mSideSlipTCL * 2)
            return 0.0;
        else if (fabs(mSideSlip) > mSideSlipTCL)
            return accel * ((1.0 - MIN(0.7, (fabs(mSideSlip) - mSideSlipTCL) * mSideSlipTCLFactor)));
    }

    return accel;
}

bool CarParams::learningOfftrack()
{
    // Offtrack situations
    double offtrackmargin = 1.0;

    if (borderDist() < -offtrackmargin)
    {
        //std::cout << "offtrack "<< borderDist() << std::endl;
        return true;
    }

    // Barrier collisions
    if (damageDiff() > 0 && wallDist() - mCar->_dimension_y / 2.0 < 0.5)
    {
        std::cout << "barrier coll (damage diff): " << damageDiff() << std::endl;
        return true;
    }

    return false;
}
