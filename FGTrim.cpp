/*******************************************************************************
 
 Header:       FGTrim.cpp
 Author:       Tony Peden
 Date started: 9/8/99
 
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
9/8/99   TP   Created
 
 
FUNCTIONAL DESCRIPTION
--------------------------------------------------------------------------------
 
This class takes the given set of IC's and finds the angle of attack, elevator,
and throttle setting required to fly steady level. This is currently for in-air
conditions only.  It is implemented using an iterative, one-axis-at-a-time
scheme. */

//  !!!!!!! BEWARE ALL YE WHO ENTER HERE !!!!!!!


/*******************************************************************************
INCLUDES
*******************************************************************************/

#include <stdlib.h>

#include "FGFDMExec.h"
#include "FGAtmosphere.h"
#include "FGInitialCondition.h"
#include "FGTrim.h"
#include "FGAircraft.h"

/*******************************************************************************/

FGTrim::FGTrim(FGFDMExec *FDMExec,FGInitialCondition *FGIC, TrimMode tt ) {

  N=Nsub=0;
  max_iterations=60;
  max_sub_iterations=100;
  Tolerance=1E-3;
  A_Tolerance = Tolerance / 10;
  
  Debug=0;
  fdmex=FDMExec;
  fgic=FGIC;
  total_its=0;
  trimudot=true;
  gamma_fallback=true;
  axis_count=0;
  mode=tt;
  xlo=xhi=alo=ahi;
  switch(mode) {
  case tFull:
    cout << "  Full 6-DOF Trim" << endl;
    TrimAxes.push_back(new FGTrimAxis(fdmex,fgic,tWdot,tAlpha,Tolerance));
    TrimAxes.push_back(new FGTrimAxis(fdmex,fgic,tUdot,tThrottle,Tolerance));
    TrimAxes.push_back(new FGTrimAxis(fdmex,fgic,tQdot,tPitchTrim,A_Tolerance));
    TrimAxes.push_back(new FGTrimAxis(fdmex,fgic,tVdot,tPhi,Tolerance));
    TrimAxes.push_back(new FGTrimAxis(fdmex,fgic,tPdot,tAileron,A_Tolerance));
    TrimAxes.push_back(new FGTrimAxis(fdmex,fgic,tRdot,tRudder,A_Tolerance));
    break;
  case tLongitudinal:
    cout << "  Longitudinal Trim" << endl;
    TrimAxes.push_back(new FGTrimAxis(fdmex,fgic,tWdot,tAlpha,Tolerance));
    TrimAxes.push_back(new FGTrimAxis(fdmex,fgic,tUdot,tThrottle,Tolerance));
    TrimAxes.push_back(new FGTrimAxis(fdmex,fgic,tQdot,tPitchTrim,A_Tolerance));
    break;
  case tGround:
    cout << "  Ground Trim" << endl;
    TrimAxes.push_back(new FGTrimAxis(fdmex,fgic,tWdot,tAltAGL,Tolerance));
    TrimAxes.push_back(new FGTrimAxis(fdmex,fgic,tQdot,tTheta,A_Tolerance));
    TrimAxes.push_back(new FGTrimAxis(fdmex,fgic,tPdot,tPhi,A_Tolerance));
    break;
  }
  //cout << "NumAxes: " << TrimAxes.size() << endl;
  NumAxes=TrimAxes.size();
  sub_iterations=new float[NumAxes];
  successful=new float[NumAxes];
  solution=new bool[NumAxes];
  current_axis=0;
}

/******************************************************************************/

FGTrim::~FGTrim(void) {
  for(current_axis=0; current_axis<NumAxes; current_axis++) {
    delete TrimAxes[current_axis];
  }
  delete[] sub_iterations;
  delete[] successful;
  delete[] solution;
}

/******************************************************************************/

void FGTrim::TrimStats() {
  char out[80];
  int run_sum=0;
  cout << endl << "  Trim Statistics: " << endl;
  cout << "    Total Iterations: " << total_its << endl;
  if(total_its > 0) {
    cout << "    Sub-iterations:" << endl;
    for(current_axis=0; current_axis<NumAxes; current_axis++) {
      run_sum+=TrimAxes[current_axis]->GetRunCount();
      sprintf(out,"   %5s: %3.0f average: %5.2f  successful: %3.0f  stability: %5.2f\n",
                  TrimAxes[current_axis]->GetAccelName().c_str(),
                  sub_iterations[current_axis],
                  sub_iterations[current_axis]/float(total_its),
                  successful[current_axis],
                  TrimAxes[current_axis]->GetAvgStability() );
      cout << out;
    }
    cout << "    Run Count: " << run_sum << endl;
  }
}

/******************************************************************************/

void FGTrim::Report(void) {
  cout << "  Trim Results: " << endl;
  for(current_axis=0; current_axis<NumAxes; current_axis++)
    TrimAxes[current_axis]->AxisReport();

}

/******************************************************************************/

void FGTrim::ReportState(void) {
  char out[80], flap[10], gear[10];
  
  cout << endl << "  JSBSim State" << endl;
  sprintf(out,"    Weight: %7.0f lbs.  CG: %5.1f, %5.1f, %5.1f inches\n",
                   fdmex->GetAircraft()->GetWeight(),
                   fdmex->GetAircraft()->GetXYZcg()(1),
                   fdmex->GetAircraft()->GetXYZcg()(2),
                   fdmex->GetAircraft()->GetXYZcg()(3) );
  cout << out;             
  if( fdmex->GetFCS()->GetDfPos() <= 0.01)
    sprintf(flap,"Up");
  else
    sprintf(flap,"%2.0f",fdmex->GetFCS()->GetDfPos());
  if(fdmex->GetAircraft()->GetGearUp() == true)
    sprintf(gear,"Up");
  else
    sprintf(gear,"Down");
  sprintf(out, "    Flaps: %3s  Gear: %4s\n",flap,gear);
  cout << out;
  sprintf(out, "    Speed: %4.0f KCAS  Mach: %5.2f\n",
                    fdmex->GetAuxiliary()->GetVcalibratedKTS(),
                    fdmex->GetState()->GetParameter(FG_MACH),
                    fdmex->GetPosition()->Geth() );
  cout << out;
  sprintf(out, "    Altitude: %7.0f ft.  AGL Altitude: %7.0f ft.\n",
                    fdmex->GetPosition()->Geth(),
                    fdmex->GetPosition()->GetDistanceAGL() );
  cout << out;
  sprintf(out, "    Angle of Attack: %6.2f deg  Pitch Angle: %6.2f deg\n",
                    fdmex->GetState()->GetParameter(FG_ALPHA)*RADTODEG,
                    fdmex->GetRotation()->Gettht()*RADTODEG );
  cout << out;
  sprintf(out, "    Flight Path Angle: %6.2f deg  Climb Rate: %5.0f ft/min\n",
                    fdmex->GetPosition()->GetGamma()*RADTODEG,
                    fdmex->GetPosition()->Gethdot()*60 );
  cout << out;                  
  sprintf(out, "    Normal Load Factor: %4.2f g's  Pitch Rate: %5.2f deg/s\n",
                    fdmex->GetAircraft()->GetNlf(),
                    fdmex->GetState()->GetParameter(FG_PITCHRATE)*RADTODEG );
  cout << out;
  sprintf(out, "    Heading: %3.0f deg true  Sideslip: %5.2f deg\n",
                    fdmex->GetRotation()->Getpsi()*RADTODEG,
                    fdmex->GetState()->GetParameter(FG_BETA)*RADTODEG );                  
  cout << out;
  sprintf(out, "    Bank Angle: %3.0f deg\n",
                    fdmex->GetRotation()->Getphi()*RADTODEG );
  cout << out;
  sprintf(out, "    Elevator: %5.2f deg  Left Aileron: %5.2f deg  Rudder: %5.2f deg\n",
                    fdmex->GetState()->GetParameter(FG_ELEVATOR_POS)*RADTODEG,
                    fdmex->GetState()->GetParameter(FG_AILERON_POS)*RADTODEG,
                    fdmex->GetState()->GetParameter(FG_RUDDER_POS)*RADTODEG );
  cout << out;                  
  sprintf(out, "    Throttle: %5.2f%c\n",
                    fdmex->GetFCS()->GetThrottlePos(0),'%' );
  cout << out;                                  
}

/******************************************************************************/

bool FGTrim::DoTrim(void) {
  
  trim_failed=false;


  for(int i=0;i < fdmex->GetAircraft()->GetNumGearUnits();i++){
    fdmex->GetAircraft()->GetGearUnit(i)->SetReport(false);
  }

  fdmex->GetOutput()->Disable();

  //clear the sub iterations counts & zero out the controls
  for(current_axis=0;current_axis<NumAxes;current_axis++) {
    //cout << current_axis << "  " << TrimAxes[current_axis]->GetAccelName()
    //<< "  " << TrimAxes[current_axis]->GetControlName()<< endl;
    xlo=TrimAxes[current_axis]->GetControlMin();
    xhi=TrimAxes[current_axis]->GetControlMax();
    TrimAxes[current_axis]->SetControl((xlo+xhi)/2);
    TrimAxes[current_axis]->Run();
    //TrimAxes[current_axis]->AxisReport();
    sub_iterations[current_axis]=0;
    successful[current_axis]=0;
    solution[current_axis]=false;
  }
  do {
    axis_count=0;
    for(current_axis=0;current_axis<NumAxes;current_axis++) {
      Nsub=0;
      if(!solution[current_axis]) {
        if(checkLimits()) { 
          solution[current_axis]=true;
          solve();
        }  
      } else if(findInterval()) {
        solve();
      } else {
        solution[current_axis]=false;
      }  
      sub_iterations[current_axis]+=Nsub;
    } 
    for(current_axis=0;current_axis<NumAxes;current_axis++) {
      //these checks need to be done after all the axes have run
      if(Debug > 0) TrimAxes[current_axis]->AxisReport();
      if(TrimAxes[current_axis]->InTolerance()) {
        axis_count++;
        successful[current_axis]++;
      } 
    }
    

    if((axis_count == NumAxes-1) && (NumAxes > 1)) {
      //cout << NumAxes-1 << " out of " << NumAxes << "!" << endl;
      //At this point we can check the input limits of the failed axis
      //and declare the trim failed if there is no sign change. If there
      //is, keep going until success or max iteration count

      //Oh, well: two out of three ain't bad
      for(current_axis=0;current_axis<NumAxes;current_axis++) {
        //these checks need to be done after all the axes have run
        if(!TrimAxes[current_axis]->InTolerance()) {
          if(!checkLimits()) {
            // special case this for now -- if other cases arise proper
            // support can be added to FGTrimAxis
            if( (gamma_fallback) &&
                (TrimAxes[current_axis]->GetAccelType() == tUdot) &&
                (TrimAxes[current_axis]->GetControlType() == tThrottle)) {
              cout << "  Can't trim udot with throttle, trying flight"
              << " path angle. (" << N << ")" << endl;
              if(TrimAxes[current_axis]->GetAccel() > 0)
                TrimAxes[current_axis]->SetControlToMin();
              else
                TrimAxes[current_axis]->SetControlToMax();
              TrimAxes[current_axis]->Run();
              delete TrimAxes[current_axis];
              TrimAxes[current_axis]=new FGTrimAxis(fdmex,fgic,tUdot,
                                                    tGamma,Tolerance);
            } else {
              cout << "  Sorry, " << TrimAxes[current_axis]->GetAccelName()
              << " doesn't appear to be trimmable" << endl;
              //total_its=k;
              trim_failed=true; //force the trim to fail
            } //gamma_fallback
          }
        } //solution check
      } //for loop
    } //all-but-one check
    N++;
    if(N > max_iterations)
      trim_failed=true;
  } while((axis_count < NumAxes) && (!trim_failed));
  if((!trim_failed) && (axis_count >= NumAxes)) {
    total_its=N;
    cout << endl << "  Trim successful" << endl;
  } else {
    total_its=N;
    cout << endl << "  Trim failed" << endl;
  }
  for(int i=0;i < fdmex->GetAircraft()->GetNumGearUnits();i++){
    fdmex->GetAircraft()->GetGearUnit(i)->SetReport(true);
  }
  fdmex->GetOutput()->Enable();
  return !trim_failed;
}

/******************************************************************************/

bool FGTrim::solve(void) {

  float x1,x2,x3,f1,f2,f3,d,d0;
  const float relax =0.9;
  float eps=TrimAxes[current_axis]->GetSolverEps();

  x1=x2=x3=0;
  d=1;
  bool success=false;
  //initializations
  if( solutionDomain != 0) {
   /* if(ahi > alo) { */
      x1=xlo;f1=alo;
      x3=xhi;f3=ahi;
   /* } else {
      x1=xhi;f1=ahi;
      x3=xlo;f3=alo;
    }   */

    d0=fabs(x3-x1);
    //iterations
    //max_sub_iterations=TrimAxes[current_axis]->GetIterationLimit();
    while (!TrimAxes[current_axis]->InTolerance() && (fabs(d) > eps) 
              && (Nsub < max_sub_iterations)) {
      Nsub++;
      d=(x3-x1)/d0;
      x2=x1-d*d0*f1/(f3-f1);
      TrimAxes[current_axis]->SetControl(x2);
      TrimAxes[current_axis]->Run();
      f2=TrimAxes[current_axis]->GetAccel();
      if(Debug > 1) {
        cout << "FGTrim::solve Nsub,x1,x2,x3: " << Nsub << ", " << x1
        << ", " << x2 << ", " << x3 << endl;
        cout << "                             " << f1 << ", " << f2 << ", " << f3 << endl;
      }
      if(f1*f2 <= 0.0) {
        x3=x2;
        f3=f2;
        f1=relax*f1;
        //cout << "Solution is between x1 and x2" << endl;
      }
      else if(f2*f3 <= 0.0) {
        x1=x2;
        f1=f2;
        f3=relax*f3;
        //cout << "Solution is between x2 and x3" << endl;

      }
      //cout << i << endl;

      
    }//end while
    if(Nsub < max_sub_iterations) success=true;
  }  
  return success;
}

/******************************************************************************/
/*
 produces an interval (xlo..xhi) on one side or the other of the current 
 control value in which a solution exists.  This domain is, hopefully, 
 smaller than xmin..0 or 0..xmax and the solver will require fewer iterations 
 to find the solution. This is, hopefully, more efficient than having the 
 solver start from scratch every time. Maybe it isn't though...
 This tries to take advantage of the idea that the changes from iteration to
 iteration will be small after the first one or two top-level iterations.

 assumes that changing the control will a produce significant change in the
 accel i.e. checkLimits() has already been called.

 if a solution is found above the current control, the function returns true 
 and xlo is set to the current control, xhi to the interval max it found, and 
 solutionDomain is set to 1.
 if the solution lies below the current control, then the function returns 
 true and xlo is set to the interval min it found and xmax to the current 
 control. if no solution is found, then the function returns false.
 
 
 in all cases, alo=accel(xlo) and ahi=accel(xhi) after the function exits.
 no assumptions about the state of the sim after this function has run 
 can be made.
*/
bool FGTrim::findInterval(void) {
  bool found=false;
  float step;
  float current_control=TrimAxes[current_axis]->GetControl();
  float current_accel=TrimAxes[current_axis]->GetAccel();;
  float xmin=TrimAxes[current_axis]->GetControlMin();
  float xmax=TrimAxes[current_axis]->GetControlMax();
  float lastxlo,lastxhi,lastalo,lastahi;
  
  step=0.025*fabs(xmax);
  xlo=xhi=current_control;
  alo=ahi=current_accel;
  lastxlo=xlo;lastxhi=xhi;
  lastalo=alo;lastahi=ahi;
  do {
    
    Nsub++;
    step*=2;
    xlo-=step;
    if(xlo < xmin) xlo=xmin;
    xhi+=step;
    if(xhi > xmax) xhi=xmax;
    TrimAxes[current_axis]->SetControl(xlo);
    TrimAxes[current_axis]->Run();
    alo=TrimAxes[current_axis]->GetAccel();
    TrimAxes[current_axis]->SetControl(xhi);
    TrimAxes[current_axis]->Run();
    ahi=TrimAxes[current_axis]->GetAccel();
    if(fabs(ahi-alo) <= TrimAxes[current_axis]->GetTolerance()) continue;
    if(alo*ahi <=0) {  //found interval with root
      found=true;
      if(alo*current_accel <= 0) { //narrow interval down a bit
        solutionDomain=-1;
        xhi=lastxlo;
        ahi=lastalo;
        //xhi=current_control;
        //ahi=current_accel;
      } else {
        solutionDomain=1;
        xlo=lastxhi;
        alo=lastahi;
        //xlo=current_control;
        //alo=current_accel;
      }     
    }
    lastxlo=xlo;lastxhi=xhi;
    lastalo=alo;lastahi=ahi;
    if( !found && xlo==xmin && xhi==xmax ) continue;
    if(Debug > 1)
      cout << "FGTrim::findInterval: Nsub=" << Nsub << " Lo= " << xlo
                           << " Hi= " << xhi << " alo*ahi: " << alo*ahi << endl;
  } while(!found && (Nsub <= max_sub_iterations) );
  return found;
}

/******************************************************************************/
//checks to see which side of the current control value the solution is on
//and sets solutionDomain accordingly:
//  1 if solution is between the current and max
// -1 if solution is between the min and current
//  0 if there is no solution
// 
//if changing the control produces no significant change in the accel then
//solutionDomain is set to zero and the function returns false
//if a solution is found, then xlo and xhi are set so that they bracket
//the solution, alo is set to accel(xlo), and ahi is set to accel(xhi)
//if there is no change or no solution then xlo=xmin, alo=accel(xmin) and
//xhi=xmax and ahi=accel(xmax) 
//in all cases the sim is left such that the control=xmax and accel=ahi

bool FGTrim::checkLimits(void) {
  bool solutionExists;
  float current_control=TrimAxes[current_axis]->GetControl();
  float current_accel=TrimAxes[current_axis]->GetAccel();
  xlo=TrimAxes[current_axis]->GetControlMin();
  xhi=TrimAxes[current_axis]->GetControlMax();

  TrimAxes[current_axis]->SetControl(xlo);
  TrimAxes[current_axis]->Run();
  alo=TrimAxes[current_axis]->GetAccel();
  TrimAxes[current_axis]->SetControl(xhi);
  TrimAxes[current_axis]->Run();
  ahi=TrimAxes[current_axis]->GetAccel();
  if(Debug > 1)
    cout << "checkLimits() xlo,xhi,alo,ahi: " << xlo << ", " << xhi << ", "
                                              << alo << ", " << ahi << endl;
  solutionDomain=0;
  solutionExists=false;
  if(fabs(ahi-alo) > TrimAxes[current_axis]->GetTolerance()) {
    if(alo*current_accel < 0) {
      solutionExists=true;
      solutionDomain=-1;
      xhi=current_control;
      ahi=current_accel;
    } else if(current_accel*ahi < 0){
      solutionExists=true;
      solutionDomain=1;
      xlo=current_control;
      alo=current_accel;  
    }
  } 
  TrimAxes[current_axis]->SetControl(current_control);
  TrimAxes[current_axis]->Run();
  return solutionExists;
}




//YOU WERE WARNED, BUT YOU DID IT ANYWAY.

