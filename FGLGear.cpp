/*******************************************************************************

 Module:       FGLGear.cpp
 Author:       Jon S. Berndt
 Date started: 11/18/99
 Purpose:      Encapsulates the landing gear elements
 Called by:    FGAircraft

 ------------- Copyright (C) 1999  Jon S. Berndt (jsb@hal-pc.org) -------------

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free Software
 Foundation; either version 2 of the License, or (at your option) any later
 version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 details.

 You should have received a copy of the GNU General Public License along with
 this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 Place - Suite 330, Boston, MA  02111-1307, USA.

 Further information about the GNU General Public License can also be found on
 the world wide web at http://www.gnu.org.

FUNCTIONAL DESCRIPTION
--------------------------------------------------------------------------------

HISTORY
--------------------------------------------------------------------------------
11/18/99   JSB   Created

********************************************************************************
INCLUDES
*******************************************************************************/

#include "FGLGear.h"
#include <algorithm>

/*******************************************************************************
************************************ CODE **************************************
*******************************************************************************/


FGLGear::FGLGear(FGConfigFile* AC_cfg, FGFDMExec* fdmex) : vXYZ(3),
                                                           vMoment(3),
                                                           vWhlBodyVec(3),
                                                           Exec(fdmex)
{
  string tmp;
  *AC_cfg >> tmp >> name >> vXYZ(1) >> vXYZ(2) >> vXYZ(3)  
            >> kSpring >> bDamp >> statFCoeff >> brakeCoeff;
  
  
  cout << "    Name: " << name << endl;
  cout << "      Location: " << vXYZ << endl;
  cout << "      Spring Constant: " << kSpring << endl;
  cout << "      Damping Constant: " << bDamp << endl;
  cout << "      Rolling Resistance: " << statFCoeff << endl;
  cout << "      Braking Coeff: " << brakeCoeff << endl;
  
  State       = Exec->GetState();
  Aircraft    = Exec->GetAircraft();
  Position    = Exec->GetPosition();
  Rotation    = Exec->GetRotation();
  
  
  WOW = false;
  ReportEnable=true;
  FirstContact = false;
  Reported = false;
  DistanceTraveled = 0.0;
  MaximumStrutForce = MaximumStrutTravel = 0.0;
}


/******************************************************************************/

FGLGear::~FGLGear(void)
{
}

/******************************************************************************/

FGColumnVector FGLGear::Force(void)
{
  FGColumnVector vForce(3);
  FGColumnVector vLocalForce(3);
  FGColumnVector vLocalGear(3);     // Vector: CG to this wheel (Local)
  FGColumnVector vWhlVelVec(3);     // Velocity of this wheel (Local)
  
  vWhlBodyVec     = (vXYZ - Aircraft->GetXYZcg()) / 12.0;
  vWhlBodyVec(eX) = -vWhlBodyVec(eX);
  vWhlBodyVec(eZ) = -vWhlBodyVec(eZ);

  vLocalGear = State->GetTb2l() * vWhlBodyVec;
  
  compressLength = vLocalGear(eZ) - Position->GetDistanceAGL();

  if (compressLength > 0.00) {
     
    WOW = true;
    vWhlVelVec      =  State->GetTb2l() * (Rotation->GetPQR() * vWhlBodyVec);
    vWhlVelVec     +=  Position->GetVel();

    compressSpeed   =  vWhlVelVec(eZ);

    if (!FirstContact) {
      FirstContact  = true;
      SinkRate      =  compressSpeed;
      GroundSpeed   =  Position->GetVel().Magnitude();
    }

    vWhlVelVec      = -1.0 * vWhlVelVec.Normalize();
    vWhlVelVec(eZ)  =  0.00;

    vLocalForce(eZ) =  min(-compressLength * kSpring - compressSpeed * bDamp, (float)0.0);
    vLocalForce(eX) =  fabs(vLocalForce(eZ) * statFCoeff) * vWhlVelVec(eX);
    vLocalForce(eY) =  fabs(vLocalForce(eZ) * statFCoeff) * vWhlVelVec(eY);

    MaximumStrutForce = max(MaximumStrutForce, fabs(vLocalForce(eZ)));
    MaximumStrutTravel = max(MaximumStrutTravel, fabs(compressLength));

    vForce  = State->GetTl2b() * vLocalForce ;
    vMoment = vWhlBodyVec * vForce;
    cout << "      Force: " << vForce << endl;
    cout << "      Moment: " << vMoment << endl;


  } else {

    WOW = false;

    if (Position->GetDistanceAGL() > 200.0) {
      FirstContact = false;
      Reported = false;
      DistanceTraveled = 0.0;
      MaximumStrutForce = MaximumStrutTravel = 0.0;
    }

    vForce.InitMatrix();
    vMoment.InitMatrix();
  }

  if (FirstContact) {
    DistanceTraveled += Position->GetVel().Magnitude()*State->Getdt()*Aircraft->GetRate();
  }

  if (ReportEnable && Position->GetVel().Magnitude() <= 0.05 && !Reported) {
    Report();
  }
  return vForce;
}

/******************************************************************************/

void FGLGear::Report(void)
{
  cout << endl << "Touchdown report for " << name << endl;
  cout << "  Sink rate at contact:  " << SinkRate                << " fps,    "
                              << SinkRate*0.3408          << " mps"     << endl;
  cout << "  Contact ground speed:  " << GroundSpeed*.5925       << " knots,  "
                              << GroundSpeed*0.3408       << " mps"     << endl;
  cout << "  Maximum contact force: " << MaximumStrutForce       << " lbs,    "
                              << MaximumStrutForce*4.448  << " Newtons" << endl;
  cout << "  Maximum strut travel:  " << MaximumStrutTravel*12.0 << " inches, "
                              << MaximumStrutTravel*30.48 << " cm"      << endl;
  cout << "  Distance traveled:     " << DistanceTraveled        << " ft,     "
                              << DistanceTraveled*0.3408  << " meters"  << endl;
  Reported = true;
}

