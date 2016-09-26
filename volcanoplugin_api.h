
/*
  Created by: Stuart Mead
  Creation date: 2014-02-03
  
  Released under BSD 3 clause.
  Use it however you want, but I cannot guarantee it is right.
  Also don't use my name, the name of collaborators and my/their affiliations
  as endorsement.

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


