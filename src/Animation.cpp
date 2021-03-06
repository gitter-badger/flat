#include <cassert>
#include "Animation.h"

namespace flat2d
{
	void Animation::start()
	{
		animationTimer.start();
	}

	void Animation::stop()
	{
		animationTimer.stop();
	}

	bool Animation::isRunning() const
	{
		return animationTimer.isStarted();
	}

	const SDL_Rect* Animation::run()
	{
		assert (!clips.empty());

		if (!animationTimer.isStarted()) {
			return &clips[0];
		}

		if (animationTimer.getTicks() >= timestep) {
			clipIndex++;
			animationTimer.stop();
			animationTimer.start();
		}

		clipIndex = clipIndex % clips.size();

		return &clips[clipIndex];
	}
} // namespace flat2d
