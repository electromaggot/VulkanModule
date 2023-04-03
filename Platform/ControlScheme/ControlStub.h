//
// ControlStub.h
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
#include "iControlScheme.h"
#include "gxCamera.h"

//#define TRY_IT_OUT


class ControlStub : public iControlScheme
{
public:
	ControlStub(gxCamera& camera)
		:	camera(camera),
			isPrimaryPressed(false)
	{ }

	void handlePrimaryPressAndDrag(int toX, int toY);
	void handlePrimaryPressDown(int atX, int atY);
	void handlePrimaryPressUp(int atX, int atY);

private:
	gxCamera&	camera;

	bool		isPrimaryPressed;
	#ifdef TRY_IT_OUT
		float		downX, prevY;
	#endif
};
