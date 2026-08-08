/***************************************************************************

    file                 : opponent.h
    created              : Thu Aug 31 01:21:49 UTC 2006
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

#ifndef _OPPONENT_H_
#define _OPPONENT_H_

#include "CarParams.h"
#include "Path.h"

#include <track.h>
#include <car.h>

class Opponent
{
public:
    Opponent(const tTrack* track, const CarElt* car, const CarParams* mycar, const Path* mypath);
    void update();
    bool racing() const { return mRacing; };
    double fromStart() { return mOppCar->_distFromStartLine; };
    double v() const { return mSpeed; };
    double dist() const { return mDist; };
    bool aside() const { return mAside; };
    double sideDist() const { return mSideDist; };
    double toMiddle() const { return mToMiddle; };
    double angle() const { return mAngle; };
    double distToStraight() const { return mDistToStraight; };
    bool inDrivingDirection() const { return mInDrivingDirection; };
    double catchTime() const { return mCatchTime; };
    bool behind() const { return mBehind; };
    bool fastBehind() const { return mFastBehind; };
    double borderDist() const { return mBorderDist; };
    bool teamMate() const { return mTeamMate; };
    bool backMarker() const { return mBackMarker; };
    bool letPass() const { return mLetpass; };
    bool damaged() const { return mDamaged; };
    bool leftOfMe() const { return mLeftOfMe; };

    virtual ~Opponent();

private:
    void updateBasics();
    void updateSpeed();
    double speedInYawDir(double yaw) const;
    void updateDist();
    double cornerDist() const;
    double calcDistToStraight() const; // dist from front to back (x) line
    bool calcBehind() const;
    double calcAngle() const; // angle relative to my car
    bool calcInDrivingDirection() const;
    double calcCatchTime() const;
    bool calcFastBehind() const;

    double mSpeed; // speed in path or my yaw direction
    double mDist; // approximation of the real distance
    double mSideDist; // side distance of center of gravity of the cars
    double mToMiddle;
    double mAngle;
    double mDistToStraight;
    double mCatchTime;
    double mBorderDist;

    bool mRacing;
    bool mBackMarker;
    bool mLetpass;
    bool mDamaged;
    bool mLeftOfMe;
    bool mBehind;
    bool mFastBehind;
    bool mAside;
    bool mInDrivingDirection;

    const CarElt*   mOppCar; // pointer to the opponents car
    const CarElt*   mCar; // pointer to my car
    const tTrack*   mTrack;
    const Path*     mMyPath; // ideal race path of my car
    const bool      mTeamMate;
    const double    mCarsDim; // dist between the cars centers when they are bumper to bumper
    const double    mFrontRange {200.0};
    const double    mBackRange {100.0};
};



#endif // _OPPONENT_H_

