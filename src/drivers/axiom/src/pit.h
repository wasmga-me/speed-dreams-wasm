/***************************************************************************

    file                 : pit.h
    created              : Thu Aug 31 01:21:49 UTC 2006
    copyright            : (C) 2006 Daniel Schellhammer
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

#ifndef _PIT_H_
#define _PIT_H_

#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robottools.h>

#include "spline.h"
#include "CarParams.h"

#define PITPOINTS 7
#define DTPOINTS 4 // drive through

class Pit
{
public:
    Pit();
    void   init(const tTrack* track, tSituation *situation, CarParams* car,
              int pitdamage, double pitgripfactor, double pitentrymargin, int rain, double pitleft, double pitright);
    void      update();
    double    dist();
    void      pitCommand();
    double    pathToMiddle(double fromstart) const;
    bool      isBetween(double fromstart) const;
    bool      isPitLimit(double fromstart);
    bool      pitOnLeftSide() const { return mPitOnLeftSide; }
    double    limitEntry() const { return mLimitEntry; }
    double    speedLimit() const { return mSpeedLimit; }
    double    avgFuelPerLap() const { return mAvgFuelPerLap; }
    bool      pitstop() const { return mPitstop; }
    bool      drivethrough() const { return mDriveThrough; }
    int       penalty() const { return mPenalty; };
    void      updateDriveThrough();

    virtual   ~Pit();

private:
    void    updateInPitLane(double fromstart);
    void    updateFuel(double fromstart);
    bool    pitBeforeTeammate(int remaininglaps) const;
    bool    pitForPenalty();
    void    setPitstop(bool pitstop);
    double  toSplineCoord(double x) const;
    int     calcRepair() const;
    double  calcRefuel();

    const tTrack*         mTrack;
    tTrackSeg             *mSeg;
    CarParams*            mMyCar;
    tCarElt*              mCar;
    tSituation*           mSituation;
    const tCarElt*        mTeamCar;
    const tTrackOwnPit*   mPit; // pointer to my pit
    const tTrackPitInfo*  mPitInfo; // general pit info
    bool                  mPitOnLeftSide;
    SplinePoint           mPitp[PITPOINTS]; // pit spline points
    SplinePoint           mDtp[DTPOINTS]; // drive through spline points
    Spline                mPitSpline;
    Spline                mDtSpline;
    int                   mPenalty;
    bool                  mPitstop;
    bool                  mInPitLane; // we are still in the pit lane
    bool                  mDriveThrough;
    double                mEntryMargin;
    double                mPitEntry;
    double                mPitExit;
    double                mLimitEntry;
    double                mLimitExit;
    double                mSpeedLimit;
    bool                  mFuelChecked;
    bool                  mStopChecked;
    double                mLastFuel;
    double                mLastPitFuel;
    double                mTotalFuel;
    double                mMaxFuel;
    int                   mFuelLapsCounted;
    double                mAvgFuelPerLap;
    double                mAvgFuelPerMeter;
    bool                  mTireChange;
    double                mTiresChangeTime;
    int                   mPitDamage;
    double                mPitGripFactor;
    double                mPitLeft;
    double                mPitRight;
    int                   mRain;

    int                   mMaxDamage;
    const int             mMaxDamageDist {30000};
    const double          mPreEntryMargin {100.0};
};

#endif // _PIT_H_
