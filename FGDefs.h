/*******************************************************************************
 
 Header:       FGDefs.h
 Author:       Jon S. Berndt
 Date started: 02/01/99
 
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
 
HISTORY
--------------------------------------------------------------------------------
02/01/99  JSB   Created
 
********************************************************************************
SENTRY
*******************************************************************************/

#ifndef FGDEFS_H
#define FGDEFS_H

#define MAX_ENGINES     10
#define MAX_TANKS       30
#define GRAVITY         32.174
#define INVGRAVITY      0.031081
#define EARTHRAD        20925650.00       // feet, equatorial
#define OMEGAEARTH      7.2685E-3         // rad/sec
#define EARTHRADSQRD    437882827922500.0
#define ONESECOND       4.848136811E-6
#define ECCENT          0.996647186
#define ECCENTSQRD      0.99330561
#define INVECCENTSQRD   1.0067395
#define INVECCENTSQRDM1 0.0067395
#define Reng            1716             //Specific Gas Constant,ft^2/(sec^2*R)
#define SHRATIO         1.4              //Specific Heat Ratio
#define RADTODEG        57.29578
#define DEGTORAD        1.745329E-2
#define KTSTOFPS        1.68781
#define FPSTOKTS        0.592484
#define INCHTOFT        0.08333333
#define OMEGA_EARTH .00007272205217  
#define NEEDED_CFG_VERSION "1.35"

#define HPTOFTLBSSEC 550
#define METERS_TO_FEET 3.2808

enum eParam {
  FG_UNDEF = 0,
  FG_QBAR,
  FG_WINGAREA,
  FG_WINGSPAN,
  FG_CBAR,
  FG_ALPHA,
  FG_ALPHADOT,
  FG_BETA,
  FG_BETADOT,
  FG_PITCHRATE,
  FG_ROLLRATE,
  FG_YAWRATE,
  FG_MACH,
  FG_ALTITUDE,
  FG_BI2VEL,
  FG_CI2VEL,
  FG_ELEVATOR_POS,
  FG_AILERON_POS,
  FG_RUDDER_POS,
  FG_SPDBRAKE_POS,
  FG_SPOILERS_POS,
  FG_FLAPS_POS,
  FG_ELEVATOR_CMD,
  FG_AILERON_CMD,
  FG_RUDDER_CMD,
  FG_SPDBRAKE_CMD,
  FG_SPOILERS_CMD,
  FG_FLAPS_CMD,
  FG_THROTTLE_CMD,
  FG_THROTTLE_POS,
  FG_HOVERB,
  FG_PITCH_TRIM_CMD
};

/******************************************************************************/
#endif

