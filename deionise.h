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

#ifndef RF_DEIONISE_H
#define RF_DEIONISE_H

#include "Workspace/DataExecution/Operations/operation.h"
#include "Workspace/DataExecution/Operations/operationfactorytraits.h"

#include "volcanoplugin.h"


namespace RF
{
	class DeioniseImpl;

	/**
	* \brief Deionises terrain models using OpenCV algos.
	*
	*/
	class RF_API Deionise : public CSIRO::DataExecution::Operation
	{
		// Allow string translation to work properly
		Q_DECLARE_TR_FUNCTIONS(RF::Deionise)

			DeioniseImpl*  pImpl_;

		// Prevent copy and assignment - these should not be implemented
		Deionise(const Deionise&);
		Deionise& operator=(const Deionise&);

	protected:
		virtual bool  execute();

	public:
		Deionise();
		virtual ~Deionise();
	};
}

DECLARE_WORKSPACE_OPERATION_FACTORY(RF::Deionise, RF_API)

#endif