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
// Created 15-Sep-2018 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef iControlScheme_h
#define iControlScheme_h


class iControlScheme
{
public:
	iControlScheme() { }

	virtual void handlePrimaryPressAndDrag(int toX, int toY)	{ }
	virtual void handlePrimaryPressDown(int atX, int atY)		{ }
	virtual void handlePrimaryPressUp(int atX, int atY)			{ }
};

#endif	// iControlScheme_h
