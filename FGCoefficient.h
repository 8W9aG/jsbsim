/*******************************************************************************

 Header:       FGCoefficient.h
 Author:       Jon Berndt
 Date started: 12/28/98

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
12/28/98   JSB   Created

********************************************************************************
SENTRY
*******************************************************************************/

#ifndef FGCOEFFICIENT_H
#define FGCOEFFICIENT_H

/*******************************************************************************
INCLUDES
*******************************************************************************/

#ifdef FGFS
#  include <simgear/compiler.h>
#endif

#include <vector>
#include <string>
#include "FGConfigFile.h"
#include "FGDefs.h"

/*******************************************************************************
DEFINES
*******************************************************************************/

/*******************************************************************************
FORWARD DECLARATIONS
*******************************************************************************/

class FGFDMExec;
class FGState;
class FGAtmosphere;
class FGFCS;
class FGAircraft;
class FGTranslation;
class FGRotation;
class FGPosition;
class FGAuxiliary;
class FGOutput;

/*******************************************************************************
COMMENTS, REFERENCES,  and NOTES
********************************************************************************

This class models the stability derivative coefficient lookup tables or 
equations. Note that the coefficients need not be calculated each delta-t.

********************************************************************************
CLASS DECLARATION
*******************************************************************************/

using std::vector;

class FGCoefficient
{
  typedef vector <eParam> MultVec;
  enum Type {UNKNOWN, VALUE, VECTOR, TABLE, EQUATION};

  int numInstances;
  string filename;
  string description;
  string name;
  string method;
  float StaticValue;
  float **Table;
  eParam LookupR, LookupC;
  MultVec multipliers;
  int rows, columns;
  Type type;
  float SD; // Actual stability derivative (or other coefficient) value

  FGFDMExec*      FDMExec;
  FGState*        State;
  FGAtmosphere*   Atmosphere;
  FGFCS*          FCS;
  FGAircraft*     Aircraft;
  FGTranslation*  Translation;
  FGRotation*     Rotation;
  FGPosition*     Position;
  FGAuxiliary*    Auxiliary;
  FGOutput*       Output;
  
  bool DeAllocate(void);
  bool Allocate(int, int);

public:
  FGCoefficient(FGFDMExec*, FGConfigFile*);
  ~FGCoefficient(void);
  
  float Value(float, float);
  float Value(float);
  float Value(void);
  float TotalValue(void);
  inline string Getname(void) {return name;}
  inline float GetSD(void) {return SD;}
  inline MultVec Getmultipliers(void) {return multipliers;}
  void DumpSD(void);

};

/******************************************************************************/
#endif
