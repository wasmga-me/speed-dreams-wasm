/***************************************************************************

    file        : MyParam.cpp
    created     : 18 Mar 2019
    copyright   : (C) 2019 D.Schellhammer

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "MyParam.h"

#include <cassert>
#include <iostream>
#include <robot.h>
#include <tgf.h>

MyParam::MyParam(void** carparmhandle, const std::string& datadir, const std::string& trackname)
{
  std::string trackfile = datadir + trackname + ".xml";
  std::string cardefaultfile = datadir + "default.xml";

  *carparmhandle = GfParmReadFile(trackfile.c_str(), GFPARM_RMODE_STD);

  if (*carparmhandle == nullptr)
  {
    *carparmhandle = GfParmReadFile(cardefaultfile.c_str(), GFPARM_RMODE_STD);

    if (*carparmhandle == nullptr)
    {
      std::string defaultfile("drivers/axiom/default" PARAMEXT);

      *carparmhandle = GfParmReadFile(defaultfile.c_str(), GFPARM_RMODE_STD);
      assert(*carparmhandle != nullptr && "default.xml file not found");
    }
  }

  mCarParmHandle = *carparmhandle;
}

double MyParam::getNum(const std::string& sect, const std::string& att) const
{
  double value = GfParmGetNum(mCarParmHandle, sect.c_str(), att.c_str(), (char*)nullptr, 0.0);

  if (value == 0.0)
  {
    std::cout << "Get " << sect << " " << att << " " << 0 << " <<<<< Check if this parameter should be 0" << std::endl;
  }
  else
  {
    std::cout << "Get " << sect << " " << att << " " << value << std::endl;
  }

  return value;
}

void MyParam::setNum(const std::string& sect, const std::string& att, double value) const
{
  GfParmSetNum(mCarParmHandle, sect.c_str(), att.c_str(), (char*)nullptr, (tdble)value); // (tdble) for VS 2013 compatibility
  std::cout << "Set " << sect << " " << att << " " << value << std::endl;
}
