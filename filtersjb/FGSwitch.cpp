
/*******************************************************************************

 Module:       FGSwitch.cpp
 Author:       
 Date started: 
 
 ------------- Copyright (C) 2000 -------------

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

********************************************************************************
COMMENTS, REFERENCES,  and NOTES
********************************************************************************

********************************************************************************
INCLUDES
*******************************************************************************/

#include "FGSwitch.h"    				

/*******************************************************************************
************************************ CODE **************************************
*******************************************************************************/

// *****************************************************************************
//  Function:   constructor
//  Purpose:
//  Parameters: void
//  Comments:

FGSwitch::FGSwitch(FGFCS* fcs, FGConfigFile* AC_cfg) : FGFCSComponent(fcs),
                                                       AC_cfg(AC_cfg)
{
  Type = AC_cfg->GetValue("TYPE");
  Name = AC_cfg->GetValue("NAME");
}

// *****************************************************************************
//  Function:   Run
//  Purpose:    
//  Parameters: void 
//  Comments:   

bool FGSwitch::Run(void )
{
  FGFCSComponent::Run(); // call the base class for initialization of Input

  return true;
}

