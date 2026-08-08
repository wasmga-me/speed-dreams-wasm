/***************************************************************************

    file                 : Wheels.cpp
    created              : 2011-03-21 07:39:00 UTC
    copyright            : (C) 2017 Daniel Schellhammer
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

#include "Wheels.h"

#include <algorithm>

Wheels::Wheels() :
    mCar(nullptr),
    mAvgWearPerMeter(0.00001)
{
}

Wheels::~Wheels()
{
}

void Wheels::init(const tCarElt* car, double muFactor, double length, int rain)
{
    mCar = car;
    mDistWhenNew = 0;
    mWear = 0.0;
    mMuScale = muFactor;

    if (rain == 0)
        mMuScaleCold = GfParmGetNum(mCar->_carHandle, "private", "mu scale cold", (char*)NULL, mMuScale);
    else if (rain == 1)
        mMuScaleCold = GfParmGetNum(mCar->_carHandle, "private", "mu scale cold rain1", (char*)NULL, mMuScale);
    else if (rain == 2)
        mMuScaleCold = GfParmGetNum(mCar->_carHandle, "private", "mu scale cold rain2", (char*)NULL, mMuScale);
    else
        mMuScaleCold = GfParmGetNum(mCar->_carHandle, "private", "mu scale cold rain3", (char*)NULL, mMuScale);

    mTrackLength = length;
    GfLogDebug("# Wheel MUSCALE = %.3f - MUSCALE COLD = %.3f\n", mMuScale, mMuScaleCold);
}

void Wheels::update()
{
    double tireWear = 1.0;

    for (int i = 0; i < 4; i++)
    {
        if (mCar->priv.wheel[i].treadDepth < mWear)
        {
            tireWear = mCar->priv.wheel[i].treadDepth;
        }
    }

    mWear = tireWear;

    // Calc dist to tire change
    mAvgWearPerMeter = (1.0 - mWear) / mCar->_distRaced;
    GfLogDebug("Avg Wear Per Meter = %.7f\n", mAvgWearPerMeter);

    double wearuntilchange = mWear / mAvgWearPerMeter;
    mDistLeft = wearuntilchange;
    GfLogDebug("# Distance Left for Tyre = %.2f\n", mDistLeft);
    mGripFactor = updateGripFactor();
}

double Wheels::updateGripFactor() const
{
    double gripFactor = 1.0;

    if(TyreCondition() < 0.86)
        gripFactor = 1.0 * mMuScaleCold;
    else
        gripFactor = 1.0 * mMuScale;

    gripFactor *= TyreCondition();
    GfLogDebug("# Tyre Condition = %.2f\n", TyreCondition());

    return gripFactor;
}

double Wheels::frictionBalanceLR() const
{
    double left = mCar->_wheelSeg(FRNT_LFT)->surface->kFriction + mCar->_wheelSeg(REAR_LFT)->surface->kFriction;
    double right = mCar->_wheelSeg(FRNT_RGT)->surface->kFriction + mCar->_wheelSeg(REAR_RGT)->surface->kFriction;

    return 1.0 - left / right;
}

double Wheels::TyreConditionFront() const
{
    return MIN(mCar->_tyreCondition(0), mCar->_tyreCondition(1));
}

double Wheels::TyreConditionRear() const
{
    return MIN(mCar->_tyreCondition(2), mCar->_tyreCondition(3));
}

double Wheels::TyreCondition() const
{
    return MIN(TyreConditionFront(), TyreConditionRear());
}

double Wheels::TyreTreadDepthFront() const
{
    double Right = mCar->_tyreTreadDepth(0);
    double Left = mCar->_tyreTreadDepth(1);

    return 100 * MIN(Right,Left);
}

double Wheels::TyreTreadDepthRear() const
{
    double Right = mCar->_tyreTreadDepth(2);
    double Left = mCar->_tyreTreadDepth(3);

    return 100 * MIN(Right,Left);
}

double Wheels::TyreTreadDepth() const
{
    return MIN(TyreTreadDepthFront(), TyreTreadDepthRear());
}
