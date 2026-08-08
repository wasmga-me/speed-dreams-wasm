INCLUDE(../../../cmake/macros.cmake)

SET(ROBOT_NAME "axiom")

# Official-only AXIOM interface.
SET(ROBOT_INTERFACE LEGACY WELCOME)

SET(ROBOT_SOURCES
    src/axiom.cpp
    src/CarParams.cpp
    src/CarParams.h
    src/cubic.cpp
    src/cubic.h
    src/datalog.cpp
    src/datalog.h
    src/dataloghead.h
    src/driver.cpp
    src/driver.h
    src/filter.cpp
    src/filter.h
    src/MuFactors.cpp
    src/MuFactors.h
    src/MuFactorsSect.h
    src/MyParam.cpp
    src/MyParam.h
    src/MyTrack.cpp
    src/MyTrack.h
    src/MyTrackSeg.h
    src/opponent.cpp
    src/opponent.h
    src/opponents.cpp
    src/opponents.h
    src/Path.cpp
    src/Path.h
    src/PathMargins.cpp
    src/PathMargins.h
    src/PathMarginsSect.h
    src/PathSeg.h
    src/PathState.cpp
    src/PathState.h
    src/PathType.h
    src/pidcontroller.cpp
    src/pidcontroller.h
    src/pit.cpp
    src/pit.h
    src/spline.cpp
    src/spline.h
    src/Timer.h
    src/Utils.cpp
    src/Utils.h
    src/Vec2.h
    src/Vec2d.h
    src/Vec3.h
    src/Vec3d.h
    src/Wheels.cpp
    src/Wheels.h )

# Official-only AXIOM instances.
SET(ROBOT_CLONES axiom_01GPA axiom_09GP5 axiom_10GP4 axiom_91GP axiom_11GPA axiom_12GPA axiom_36GP axiom_67GP axiom_sca axiom_scb axiom_scc
				 axiom_ls2 axiom_rs axiom_ls1 axiom_lp2)

# The ubiquitous robot module and its clones.
ROBOT_MODULE(NAME ${ROBOT_NAME} VERSION 3.3.0 SOVERSION 1.0.0
             INTERFACE ${ROBOT_INTERFACE}
             SOURCES ${ROBOT_SOURCES}
             CLONENAMES ${ROBOT_CLONES})
