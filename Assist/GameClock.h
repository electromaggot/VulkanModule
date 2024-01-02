//
// GameClock.h
//	Vulkan Renderables Assist
//
// Game Loop Timing, encapsulate things like: per-frame delta-time for
//	Update() calls, elapsed time since game started, etc.
//
// Intended to support Renderables called to Update() in a time-dependent manner.
//	Previously passed simply a 'float deltaTime' but now passing a reference to this
//	entire object, allowing more flexible access to timing-related values.
//
// Implementation uses clock resource that's built-into the standard library... but could be
//	extracted or overridden per other platforms' availability or depending on accuracy, overhead, etc.
//
// Created 3/31/23 by Tadd Jensen
//	© 0000 (uncopyrighted; use at will)
//
#ifndef GameClock_h
#define GameClock_h

#include <chrono>
using std::chrono::high_resolution_clock; using std::chrono::duration; using std::chrono::seconds;


class GameClock
{
public:
	GameClock()
		:	startTime(high_resolution_clock::now()),
			durationLastFrame(0.0f),
			elapsedToFrameStart(0.0f)
	{ }

		// MEMBERS
protected:
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	float durationLastFrame;
	float elapsedToFrameStart;

		// METHODS
public:
	// Returns a realtime float, serves as our overall running "time clock" for the game.
	//
	inline float secondsSinceGameStart()
	{
		auto currentTime = high_resolution_clock::now();
		return duration<float, seconds::period>(currentTime - startTime).count();
	}

	inline void BeginNewFrame()
	{
		float elapsedToNow = secondsSinceGameStart();
		durationLastFrame = elapsedToNow - elapsedToFrameStart;
		elapsedToFrameStart = elapsedToNow;
	}

		// getters
	inline float deltaSeconds() {	// since previous frame
		return durationLastFrame;
	}
	inline float secondsElapsed() {	// since game start
		return elapsedToFrameStart;
	}
};

#endif	// GameClock_h



/* IMPORTANT USAGE NOTE (and when to be careful using this class)
	Times are stored in seconds, represented by 'float's -- of course including
	  fractions-of-second for tiny durations like that of a rendered frame.
	"Duration" is simply a number of seconds (or again, fractional seconds), but
	  does not directly imply a start or finish time.
	"Elapsed" means duration of seconds since the Game Started (startTime) up until
	  either: the start of a frame (or current frame), or "right now"
	Heed the distinction between REAL-TIME and FRAME TIME.
		- realtime - is independent of frames or anything frame-related, for example:
				- exact time of an event, like precisely when user hit a button.
				- right now at this instant.
		- frametime - regards the:
				- exact time that current frame started (or frame previous to it).
				- duration of previous frame.
	Frametime is vital, especially that previous frame duration, here called: deltaSeconds
	  In other words:													  or: deltaTime
	  Upon BeginNewFrame() the duration that has passed since we drew the previous frame.
	All game elements must be Updat()ed using that same value (not realtime, which changes
	  as the program runs line-to-line).  This step updates the positions of all elements
	  equivalently.
	  (Done correctly, this is critical to, for example, not allowing two colliding objects
	   to pass through each other.)  Once all those positions are updated, rendering of the
	  new frame can begin.
*/
/* TO-DOs:
	- Provide/manage FPS frames-per-second counter.
	- Slo-mo mode (by scaling deltaTime and returning that) or fixed timing.
	- If deltaTime exceeds threshold, break into series of smaller dT's -- keeping
	  iterative "calculations" from blowing up, above-mentioned physics collision, etc.
*/
