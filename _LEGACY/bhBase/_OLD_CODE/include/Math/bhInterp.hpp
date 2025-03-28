#ifndef BH_INTERP_HPP
#define BH_INTERP_HPP

// REFERENCE:
// http://sol.gfxile.net/interpolation/index.html

template<class InterpType, class TimeType>
inline InterpType bhLinear(InterpType startVal, InterpType endVal, TimeType t) // Normalized Elapsed Time
{
	return (startVal * (TimeType(1) - t)) + (endVal * t);
}

template<class InterpType, class TimeType>
inline InterpType bhSmoothstep1(InterpType startVal, InterpType endVal, TimeType t) // Normalized Elapsed Time
{
	auto x = t * t * (TimeType(3) - TimeType(2) * t);
	return (startVal * (TimeType(1) - x)) + (endVal * x);
}

template<class InterpType, class TimeType> inline InterpType
bhSmoothstep2(InterpType startVal, InterpType endVal, TimeType t) // Normalized Elapsed Time
{
	auto x = t * t * t * (t * (TimeType(6) * t - TimeType(15)));
	return (startVal * (TimeType(1) - x)) + (endVal * x);
}

template<class InterpType, class TimeType, InterpType(*TimeFunc)(InterpType, InterpType, TimeType)>
class bhInterp
{
public:
	bhInterp()
	{}

	bhInterp(InterpType _start, InterpType _end)
		: startVal(_start)
		, endVal(_end)
	{}

	void Start(TimeType _duration)
	{
		assert(_duration != TimeType(0));
		elapsedTime = TimeType(0);
		duration = _duration;
		playStatus = PlayStatus::PLAY;
	}
	
	void Tick(TimeType deltaTime)
	{
		if (playStatus != PlayStatus::PLAY) return;
		elapsedTime += deltaTime;
		if (elapsedTime > duration)
		{
			elapsedTime = duration;
			playStatus = PlayStatus::STOP;
		}
		currVal = TimeFunc(startVal, endVal, elapsedTime / duration);
	}

	inline void SetStart(const InterpType* s)
	{
		startVal = *s;
	}

	inline void SetEnd(const InterpType* e)
	{
		endVal = *e;
	}

	inline InterpType GetValue() const
	{
		return currVal;
	}

	inline TimeType GetTime() const
	{
		return elapsedTime / duration;
	}

	inline bool IsPlaying() const
	{
		return playStatus == PlayStatus::PLAY;
	}

protected:
	enum class PlayStatus
	{
		STOP,
		PAUSE,
		PLAY
	};

	InterpType startVal, endVal, currVal;
	TimeType duration = TimeType(0), elapsedTime = TimeType(0);
	PlayStatus playStatus = PlayStatus::STOP;

private:
};

#endif //BH_INTERP_H
