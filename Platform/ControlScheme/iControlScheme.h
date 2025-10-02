//
// iControlScheme.h
//	Interface/Abstraction
//
// Ultimately a "control scheme" takes input and acts on a camera/Object3D/matrix/etc.
//	Encapsulating how that's done in a specific way is the purpose of each subclass.
//	"Input" is typically from a Human Interface Device, but could take other forms.
//	The resultant matrix may affect 6DOF, 3DOF, 2+1DOF (X,Y+rotation) etc. depending
//	on scheme and can multiply to directly translate/rotate a camera object...  OR
//	a more complex object, whether 2D (sprite) or 3D (model), acting on such objects'
//	custom fields, for instance a velocity vector, or calling a FireShot() method.
//
// As for Mobile vs. Desktop platforms, a ControlScheme can either handle them
//	separately, or try to "merge" their handling as possible; there's much overlap
//	between the two.  However for some things, there's no equivalent - for instance
//	"mouse hover" is impossible to mimic on Mobile.  Also, keyboard input will never
//	be equal on Mobile, which of course requires the pop-up virtual keyboard, so can
//	never emulate, for example, directional/strafing controls like the WASD keys.
//	Standalone Game Controller support: a whole other story + its own ControlScheme.
//
// Created 15-Sep-2018 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef iControlScheme_h
#define iControlScheme_h


class iControlScheme
{
public:
	iControlScheme() { }

	virtual void handlePrimaryPressAndDrag(int toX, int toY)	{ }		// "Press" means either mouse
	virtual void handlePrimaryPressDown(int atX, int atY)		{ }		//	 button (click) or touch (tap).
	virtual void handlePrimaryPressUp(int atX, int atY)			{ }
	virtual void handleSecondaryPressAndDrag(int toX, int toY)	{ }		// "Secondary" == right mouse
	virtual void handleSecondaryPressDown(int atX, int atY)		{ }		//	 button or 2-finger touch.
	virtual void handleSecondaryPressUp(int atX, int atY)		{ }
	virtual void handleTertiaryPressAndDrag(int toX, int toY)	{ }		// "Tertiary" == mouse wheel
	virtual void handleTertiaryPressDown(int atX, int atY)		{ }		//	 click or 3-finger touch.
	virtual void handleTertiaryPressUp(int atX, int atY)		{ }

	virtual void handleMouseWheel(int spunX, int spunY)			{ }		// "can" assume zooming via mouse.
	virtual void handlePinchSpread(float amount)				{ }		// Always means zoom via touch.
	virtual void handleTwoFingerTwist(float angle)				{ }
};

#endif	// iControlScheme_h
