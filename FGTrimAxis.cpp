/*******************************************************************************
 
 Header:       FGTrimAxis.cpp
 Author:       Tony Peden
 Date started: 7/3/00
 
 --------- Copyright (C) 1999  Anthony K. Peden (apeden@earthlink.net) ---------
 
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
 
 
 HISTORY
--------------------------------------------------------------------------------
7/3/00   TP   Created
 
 
*/
/*******************************************************************************
INCLUDES
*******************************************************************************/

#include <string>
#include <stdlib.h>

#include "FGFDMExec.h"
#include "FGAtmosphere.h"
#include "FGInitialCondition.h"
#include "FGTrimAxis.h"
#include "FGAircraft.h"

/*****************************************************************************/

FGTrimAxis::FGTrimAxis(FGFDMExec* fdex, FGInitialCondition* ic, Accel acc,
                       Control ctrl, float ff) {

  fdmex=fdex;
  fgic=ic;
  accel=acc;
  control=ctrl;
  tolerance=ff;
  solver_eps=tolerance;
  max_iterations=10;
  control_value=0;
  its_to_stable_value=0;
  total_iterations=0;
  total_stability_iterations=0;
  accel_convert=1.0;
  control_convert=1.0;
  accel_value=0;
  switch(control) {
  case tThrottle:
    control_min=0;
    control_max=1;
    control_value=0.5;
    break;
  case tBeta:
    control_min=-30*DEGTORAD;
    control_max=30*DEGTORAD;
    control_convert=RADTODEG;
    break;
  case tAlpha:
    control_min=fdmex->GetAircraft()->GetAlphaCLMin();
    control_max=fdmex->GetAircraft()->GetAlphaCLMax();
    if(control_max <= control_min) {
      control_max=20*DEGTORAD;
      control_min=-5*DEGTORAD;
    }
    control_value= (control_min+control_max)/2;
    control_convert=RADTODEG;
    solver_eps=tolerance/100;
    break;
  case tPitchTrim:
  case tElevator:
  case tRollTrim:
  case tAileron:
  case tYawTrim:
  case tRudder:
    control_min=-1;
    control_max=1;
    accel_convert=RADTODEG;
    solver_eps=tolerance/100;
    break;
  case tAltAGL:
    control_min=0;
    control_max=30;
    control_value=fdmex->GetPosition()->GetDistanceAGL();
    solver_eps=tolerance/100;
    break;
  case tTheta:
    control_min=-10*DEGTORAD;
    control_max=10*DEGTORAD;
    accel_convert=RADTODEG;
    break;
  case tPhi:
    control_min=-30*DEGTORAD;
    control_max=30*DEGTORAD;
    accel_convert=RADTODEG;
    break;
  case tGamma:
    solver_eps=tolerance/100;
    control_min=-80*DEGTORAD;
    control_max=80*DEGTORAD;
    control_convert=RADTODEG;
    break;
  }
  
}

/*****************************************************************************/

FGTrimAxis::~FGTrimAxis() {}

/*****************************************************************************/

void FGTrimAxis::getAccel(void) {
  switch(accel) {
  case tUdot: accel_value=fdmex -> GetTranslation()->GetUVWdot()(1); break;
  case tVdot: accel_value=fdmex -> GetTranslation()->GetUVWdot()(2); break;
  case tWdot: accel_value=fdmex -> GetTranslation()->GetUVWdot()(3); break;
  case tQdot: accel_value=fdmex -> GetRotation()->GetPQRdot()(2);break;
  case tPdot: accel_value=fdmex -> GetRotation()->GetPQRdot()(1); break;
  case tRdot: accel_value=fdmex -> GetRotation()->GetPQRdot()(3); break;
  }
}

/*****************************************************************************/

//Accels are not settable

void FGTrimAxis::getControl(void) {
  switch(control) {
  case tThrottle:  control_value=fdmex->GetFCS()->GetThrottleCmd(0); break;
  case tBeta:      control_value=fdmex->GetTranslation()->Getalpha(); break;
  case tAlpha:     control_value=fdmex->GetTranslation()->Getbeta();  break;
  case tPitchTrim: control_value=fdmex->GetFCS() -> GetPitchTrimCmd(); break;
  case tElevator:  control_value=fdmex->GetFCS() -> GetDeCmd(); break;
  case tRollTrim:
  case tAileron:   control_value=fdmex->GetFCS() -> GetDaCmd(); break;
  case tYawTrim:
  case tRudder:    control_value=fdmex->GetFCS() -> GetDrCmd(); break;
  case tAltAGL:    control_value=fdmex->GetPosition()->GetDistanceAGL();break;
  case tTheta:     control_value=fdmex->GetRotation()->Gettht(); break;
  case tPhi:       control_value=fdmex->GetRotation()->Getphi(); break;
  case tGamma:     control_value=fdmex->GetPosition()->GetGamma();break;
  }
}

/*****************************************************************************/


void FGTrimAxis::setControl(void) {
  switch(control) {
  case tThrottle:  setThrottlesPct(); break;
  case tBeta:      fgic->SetBetaRadIC(control_value); break;
  case tAlpha:     fgic->SetAlphaRadIC(control_value);  break;
  case tPitchTrim: fdmex->GetFCS() -> SetPitchTrimCmd(control_value); break;
  case tElevator:  fdmex-> GetFCS() -> SetDeCmd(control_value); break;
  case tRollTrim:
  case tAileron:   fdmex-> GetFCS() -> SetDaCmd(control_value); break;
  case tYawTrim:
  case tRudder:    fdmex-> GetFCS() -> SetDrCmd(control_value); break;
  case tAltAGL:    fgic->SetAltitudeAGLFtIC(control_value); break;
  case tTheta:     fgic->SetPitchAngleRadIC(control_value); break;
  case tPhi:       fgic->SetRollAngleRadIC(control_value); break;
  case tGamma:     fgic->SetFlightPathAngleRadIC(control_value); break;
  }
}

/*****************************************************************************/

// the aircraft center of rotation is no longer the cg once the gear
// contact the ground so the altitude needs to be changed when pitch 
// and roll angle are adjusted.  Instead of attempting to calculate the 
// new center of rotation, pick a gear unit as a reference and use its
// location vector to calculate the new height change. i.e. new altitude =
// earth z component of that vector (which is in body axes )  
void FGTrimAxis::SetThetaOnGround(float ff) {
  int center,i,ref;

  // favor an off-center unit so that the same one can be used for both
  // pitch and roll.  An on-center unit is used (for pitch)if that's all 
  // that's in contact with the ground.
  i=0; ref=-1; center=-1;
  while( (ref < 0) && (i < fdmex->GetAircraft()->GetNumGearUnits()) ) {
    if(fdmex->GetAircraft()->GetGearUnit(i)->GetWOW()) {
      if(fabs(fdmex->GetAircraft()->GetGearUnit(i)->GetBodyLocation()(2)) > 0.01)
        ref=i;
      else
        center=i;
    } 
    i++; 
  }
  if((ref < 0) && (center >= 0)) {
    ref=center;
  }
  cout << "SetThetaOnGround ref gear: " << ref << endl;
  if(ref >= 0) {
    float sp=fdmex->GetRotation()->GetSinphi();
    float cp=fdmex->GetRotation()->GetCosphi();
    float lx=fdmex->GetAircraft()->GetGearUnit(ref)->GetBodyLocation()(1);
    float ly=fdmex->GetAircraft()->GetGearUnit(ref)->GetBodyLocation()(2);
    float lz=fdmex->GetAircraft()->GetGearUnit(ref)->GetBodyLocation()(3);
    float hagl = -1*lx*sin(ff) +
                    ly*sp*cos(ff) +
                    lz*cp*cos(ff);
   
    fgic->SetAltitudeAGLFtIC(hagl);
    cout << "SetThetaOnGround new alt: " << hagl << endl;
  }                   
  fgic->SetPitchAngleRadIC(ff);  
  cout << "SetThetaOnGround new theta: " << ff << endl;      
}      

/*****************************************************************************/

void FGTrimAxis::SetPhiOnGround(float ff) {
  int i,ref;

  i=0; ref=-1;
  //must have an off-center unit here 
  while( (ref < 0) && (i < fdmex->GetAircraft()->GetNumGearUnits()) ) {
    if( (fdmex->GetAircraft()->GetGearUnit(i)->GetWOW()) && 
      (fabs(fdmex->GetAircraft()->GetGearUnit(i)->GetBodyLocation()(2)) > 0.01))
        ref=i;
    i++; 
  }
  if(ref >= 0) {
    float st=fdmex->GetRotation()->GetSintht();
    float ct=fdmex->GetRotation()->GetCostht();
    float lx=fdmex->GetAircraft()->GetGearUnit(ref)->GetBodyLocation()(1);
    float ly=fdmex->GetAircraft()->GetGearUnit(ref)->GetBodyLocation()(2);
    float lz=fdmex->GetAircraft()->GetGearUnit(ref)->GetBodyLocation()(3);
    float hagl = -1*lx*st +
                    ly*sin(ff)*ct +
                    lz*cos(ff)*ct;
   
    fgic->SetAltitudeAGLFtIC(hagl);
  }                   
  fgic->SetRollAngleRadIC(ff);
           
}      

/*****************************************************************************/

void FGTrimAxis::Run(void) {

  float last_accel_value;
  int i;
  setControl();
  //cout << "FGTrimAxis::Run: " << control_value << endl;
  i=0;
  bool stable=false;
  while(!stable) {
    i++;
    last_accel_value=accel_value;
    fdmex->RunIC(fgic);
    getAccel();
    if(i > 1) {
      if((fabs(last_accel_value - accel_value) < tolerance) || (i >= 100) )
        stable=true;
    }
  }

  its_to_stable_value=i;
  total_stability_iterations+=its_to_stable_value;
  total_iterations++;
}

/*****************************************************************************/

void FGTrimAxis::setThrottlesPct(void) {
  float tMin,tMax;
  for(unsigned i=0;i<fdmex->GetAircraft()->GetNumEngines();i++) {
      tMin=fdmex->GetAircraft()->GetEngine(i)->GetThrottleMin();
      tMax=fdmex->GetAircraft()->GetEngine(i)->GetThrottleMax();
      //cout << "setThrottlespct: " << i << ", " << control_min << ", " << control_max << ", " << control_value;
      fdmex -> GetFCS() -> SetThrottleCmd(i,tMin+control_value*(tMax-tMin));
  }
}


/*****************************************************************************/


void FGTrimAxis::AxisReport(void) {
  
  char out[80];
  sprintf(out,"  %20s: %6.2f %5s: %9.2e Tolerance: %3.0e\n",
           GetControlName().c_str(), GetControl()*control_convert,
           GetAccelName().c_str(), GetAccel(), GetTolerance()); 
  cout << out;

}


/*****************************************************************************/

float FGTrimAxis::GetAvgStability( void ) {
  if(total_iterations > 0) {
    return float(total_stability_iterations)/float(total_iterations);
  }
  return 0;
}

