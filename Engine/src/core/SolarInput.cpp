#include "SolarInput.h"

namespace sol
{
	static Input current = {};
	static Input previous = {};

	inline bool8 Input::Initailize()
	{
		current.oldInput = &previous;
		return true;
	}

	void Input::Shutdown()
	{

	}

	void Input::Flip()
	{
		previous = current;
	}

	inline Input* Input::Get()
	{
		return &current;
	}
}