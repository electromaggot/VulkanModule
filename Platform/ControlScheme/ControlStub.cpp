//
// ControlStub.cpp
//	Instantiation sans implementation
//
// See header file for main comment.  This class is empty and simply serves as an example.
//
// Created 15-Sep-2018 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#include "ControlStub.h"


void ControlStub::handlePrimaryPressAndDrag(int toX, int toY)
{
	#ifdef TRY_IT_OUT
		if (isPrimaryPressed)
		{
			const float foreAftSensitivity = 0.5f;
			const float lateralSensitivity = 0.3f;

			camera.z += (toY - prevY) * foreAftSensitivity;
			camera.x = (toX - downX) * lateralSensitivity;
			camera.updateViewMatrix();
			prevY = toY;
		}
	#endif
}

void ControlStub::handlePrimaryPressDown(int atX, int atY)
{
	#ifdef TRY_IT_OUT
		isPrimaryPressed = true;
		downX = atX;  prevY = atY;
		handlePrimaryPressAndDrag(atX, atY);
	#endif
}

void ControlStub::handlePrimaryPressUp(int atX, int atY)
{
	#ifdef TRY_IT_OUT
		isPrimaryPressed = false;
	#endif
}
