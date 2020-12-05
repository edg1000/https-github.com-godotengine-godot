/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2013 Erwin Coumans  http://bulletphysics.org

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
///original version written by Erwin Coumans, October 2013

#ifndef BT_MLCP_SOLVER_INTERFACE_H
#define BT_MLCP_SOLVER_INTERFACE_H

#include "LinearMath/btMatrixX.h"

class btMLCPSolverInterface
{
public:
	virtual ~btMLCPSolverInterface()
	{
	}

	//return true is it solves the problem successfully
	virtual bool solveMLCP(const btMatrixXu& A, const btVectorXu& b, btVectorXu& x, const btVectorXu& lo, const btVectorXu& hi, const btAlignedObjectArray<int>& limitDependency, int numIterations, bool useSparsity = true) = 0;
};

#endif  //BT_MLCP_SOLVER_INTERFACE_H
