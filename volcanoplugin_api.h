
/*
  Created by: Stuart Mead
  Creation date: 2014-02-03
   
  Revision:       $Revision: $
  Last changed:   $Date: $
  Last changed by: Stuart Mead

  Copyright Risk Frontiers 2014, Faculty of Science, Macquarie University, NSW 2109, Australia.

  For further information, contact:
          Stuart Mead
          Building E7A
          Dept. of Environment & Geography
          Macquarie University
          North Ryde NSW 2109

  This copyright notice must be included with all copies of the source code.

*/


/**
 * \file
 */

#ifndef RF_API_H
#define RF_API_H

#include "Workspace/api_workspace.h"


#ifdef RF_EXPORT
    #define RF_API CSIRO_EXPORTSPEC
#else
    #define RF_API CSIRO_IMPORTSPEC
#endif

#endif


