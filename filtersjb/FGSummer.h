
/*******************************************************************************

 Header:       FGSummer.h
 Author:       
 Date started: 

 ------------- Copyright (C)  -------------

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

********************************************************************************
COMMENTS, REFERENCES,  and NOTES
********************************************************************************

********************************************************************************
SENTRY
*******************************************************************************/

#ifndef FGSUMMER_H
#define FGSUMMER_H

/*******************************************************************************
INCLUDES
*******************************************************************************/

#ifdef FGFS
#  include <simgear/compiler.h>
#  include STL_STRING
   FG_USING_STD(string);
#  ifdef FG_HAVE_STD_INCLUDES
#    include <vector>
#  else
#    include <vector.h>
#  endif
#else
#  include <vector>
#  include <string>
#endif

#include "FGFCSComponent.h"
#include "../FGConfigFile.h"

/*******************************************************************************
DEFINES
*******************************************************************************/

/*******************************************************************************
CLASS DECLARATION
*******************************************************************************/

class FGSummer  : public FGFCSComponent
{
  FGConfigFile* AC_cfg;
  vector<int> InputIndices;
  vector<int> InputTypes;

public:
  FGSummer(FGFCS* fcs, FGConfigFile* AC_cfg);
  ~FGSummer ( ) { }       //Destructor

  bool Run (void );
};

#endif
