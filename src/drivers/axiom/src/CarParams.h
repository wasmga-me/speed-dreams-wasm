/***************************************************************************

    file        : CarParams.h
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

#ifndef _MYCAR_H_
#define _MYCAR_H_

#include <string>
#include <car.h>

#include "filter.h"
#include "MyParam.h"
#include "MyTrack.h"
#include "PathType.h"
#include "pidcontroller.h"
#include "Wheels.h"
#include "Vec3d.h"

#include "raceman.h"

// The "AXIOM" logger instance.
extern GfLogger* PLogAXIOM;
#define LogAXIOM (*PLogAXIOM)

class CarParams
{
public:
    CarParams();

    void      init(tCarElt* car, MyTrack* track, int rain);
    void      readPrivateSection(const MyParam& param, bool qualif, int rain);
    void      readStrategySection(const MyParam& param, double temperature, int rain, double distance, int laps);
    void      readVarSpecs(const MyParam& param);
    void      readConstSpecs(void* CarHandle);
    int       calcCompound(double temp, double distance, int laps);
    double    calcFuel(double dist) const;
    double    calcFuel2(double dist) const;
    void      update(double dt);
    void      setControls(double accel, double brake, double steer);
    double    brakeForce(double speed, double curvature, double curv_z, double mu, double pitchAngle, double rollAngle, PathType pathtype) const;
    double    curveSpeed(double curvature, double curv_z, double mu, double rollAngle, PathType pathtype) const;
    double    bumpSpeed(double curv_z) const;
    void      driveForward() { mGearDirection = 1; }
    void      driveBackwards() { mGearDirection = -1; }
    void      setTireMu(int c) { mTireMu = mTireMuC[c]; }
    double    v() const { return mSpeed; }
    double    vX() const { return mSpeedX; }
    double    accelFiltered() const { return mAccelFilter.get(); }
    double    maxAccelForce() const { return mMaxAccelForce; }
    double    maxBrakeForce() const { return mBrakeForceMax; }
    double    cw() const { return mCW; }
    double    segMu() const { return mSegMu; }
    double    tireMu() const { return mTireMu; }
    double    brakeMuFactor() const { return mBrakeMuFactor; };
    double    brakeMuFactorCold() const { return mBrakeMuFactorCold; }
    double    brakeMuPitFactor() const { return mBrakeMuPitFactor; }
    double    muScaleLR() const { return mMuScaleLR; }
    double    mass() const { return mMass; }
    double    toMid() const { return mToMiddle; }
    double    speedYaw() const { return mSpeedYaw; }
    double    yaw() const { return mYaw; }
    double    yawRate() const { return mYawRate; } // yawdiff / dist (the same as path curvature when driving slow)
    double    angToTrack() const { return mAngleToTrack; }
    double    borderDist() const { return mBorderDist; }
    double    borderFriction() const { return mBorderFriction; }
    double    wallDist() const { return mWalldist; }
    double    wallToMiddleAbs() const { return mWallToMiddleAbs; }
    bool      pointingToWall() const { return mPointingToWall; }
    int       damageDiff() const { return mDamageDiff; }
    double    sideSlip() const { return mSideSlip; }
    double    avgFuelPerMeter() const { return mFuelPerMeter; }
    bool      learningOfftrack();
    double    filterABS(double brake);
    double    filterTCL(double accel);
    double    slipFront() const;
    double    slipRear() const;
    double    filterTCLSideSlip(double accel) const ;
    const tCarElt* car()const { return mCar; }
    const Wheels* wheels() const { return &mWheels; }
    int       gear() const { return mCar->_gear; }
    double    pathL() const { return mPathL; }
    double    pathR() const { return mPathR; }
    double    MaxFuel() const { return mMaxFuel; }
    double    Fuel5Laps() const { return mMaxFuel5Laps; }
    double    Fuel10Laps() const { return mMaxFuel10Laps; }
    double    Fuel80KM() const { return mMaxFuel80KM; }
    double    Fuel160KM() const { return mMaxFuel160KM; }
    double    Fuel320KM() const { return mMaxFuel320KM; }
    double    Fuel330KM() const { return mMaxFuel330KM; }
    double    getPressureFR() const { return mPressureFR; }
    double    getPressureFL() const { return mPressureFL; }
    double    getPressureRR() const { return mPressureRR; }
    double    getPressureRL() const { return mPressureRL; }
    double    getCamberFR() const { return mCamberFR; }
    double    getCamberFL() const { return mCamberFL; }
    double    getCamberRR() const { return mCamberRR; }
    double    getCamberRL() const { return mCamberRL; }
    int       GetArrets() const { return mMaxArrets; }
    int       GetCompound() const { return mCompound; }


    Vec3d mFrontAxleGlobalPos;
    double mSpeed; // COG speed
    double mSpeedX; // speed in cars yaw direction
    double mSpeedYaw; // yaw of COG speed vector
    double mSlip;
    double mSideSlip;
    double mToMiddle;

    bool    HASCPD;
    bool    HASABS;
    bool    HASESP;
    bool    HASTCL;

    virtual ~CarParams();

private:
    void      initVars();
    void      initCa();
    void      initCw();
    void      initBrakes();
    int       calcGear();
    double    calcClutch();

    double    mDeltaTime;
    tCarElt*  mCar;
    const MyTrack* mTrack;
    Wheels    mWheels;
    Vec3d     mGlobalPos;
    double    mFrontAxleOffset;
    int       mGearDirection;
    int       mShiftTimer;
    int       mPrevGear;
    double    mClutch;
    double    mAbsFactor;
    double    mTclFactor;
    PidController mTclController;
    double    mAccel;
    Filter    mAccelFilter;
    double    mMaxAccelForce;
    double    mSegMu;
    double    mMass;
    Vec3d     mHeading;
    double    mYaw;
    double    mYawRate;
    double    mAngleToTrack;
    double    mBorderDist;
    double    mBorderFriction;
    int       mDamageDiff;
    int       mLastDamage;
    double    mWalldist;
    double    mWallToMiddleAbs;
    bool      mPointingToWall;

    // Private configuration file constants
    double    mAbsSlip;
    double    mBrakeMuFactor;
    double    mBrakeMuFactorCold;
    double    mBrakeMuPitFactor;
    double    mMuScaleLR;
    double    mMuScale;
    double    mBumpSpeedFactor;
    double    mFuelPerMeter;
    double    mFuelWeightFactor;
    double    mTireWearPerMeter;
    double    mPathL;
    double    mPathR;

    // Strategy configuration file constants
    double    mMaxFuel;
    double    mMaxFuel5Laps;
    double    mMaxFuel10Laps;
    double    mMaxFuel80KM;
    double    mMaxFuel160KM;
    double    mMaxFuel320KM;
    double    mMaxFuel330KM;
    double    mPressureFR;
    double    mPressureFL;
    double    mPressureRR;
    double    mPressureRL;
    double    mCamberFR;
    double    mCamberFL;
    double    mCamberRR;
    double    mCamberRL;
    int       mMaxArrets;
    int       mCompound;
    int       mRain;

    // Configuration file constants
    double    mBrakePressMax;
    double    mBrakeRepartition;
    double    mFrontWingAngle;
    double    mRearWingAngle;
    double    mSideSlipTCL;
    double    mSideSlipTCLQualy;
    double    mSideSlipTCLFactor;

    // Constants
    double    mCarMass;
    double    mTankVol;
    double    mTireMu;
    double    mTireMuC[6];

    // Aerodynamic constants
    double    mFrontCA;
    double    mRearCA;
    double    mCA;      // aerodynamic downforce coefficient
    double    mBodyCW;  // body aerodynamic drag coefficient
    double    mWingCW;  // wing aerodynamic drag coefficient
    double    mCW;

    // Brake constant
    double    mBrakeForceMax;
};

#endif // _CARPARAMS_H_
