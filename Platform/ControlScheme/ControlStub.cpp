//
// ControlStub.cpp
//	Instantiation sans implementation
//
// See header file for main comment.  This class is empty and simply serves as an example.
//
// Derive your custom class from iControlScheme, or use this file as a template and fill-in your own
//	implementation.  On construction, pass in a reference to an object you want this controller to act upon.
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
		downX = atX;  prevY = atY;// - camera.getCurrentScreenHeight() / 2.0f;
		handlePrimaryPressAndDrag(atX, atY);
	#endif
}

void ControlStub::handlePrimaryPressUp(int atX, int atY)
{
	#ifdef TRY_IT_OUT
		isPrimaryPressed = false;
	#endif
}
