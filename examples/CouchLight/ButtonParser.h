#ifndef _BUTTONPARSER_h
#define _BUTTONPARSER_h

template<uint32_t LongPressThresholdMicros = 1000000, uint32_t ShortPressThresholdMicros = 5000>
class ButtonParser
{
private:
	uint32_t PressedTimestamp = 0;
	bool LastPressed = false;
	bool LastLong = false;

public:
	bool HasShortEvent = false;
	bool HasLongEvent = false;

	void Clear()
	{
		PressedTimestamp = 0;
		LastPressed = false;
		HasShortEvent = false;
		HasLongEvent = false;
	}

	bool Parse(const uint32_t timestamp, const bool pressed)
	{
		HasShortEvent = false;
		HasLongEvent = false;

		if (pressed)
		{
			if (!LastPressed)
			{
				PressedTimestamp = timestamp;
			}
			else
			{
				const uint32_t elapsed = timestamp - PressedTimestamp;

				if (elapsed >= LongPressThresholdMicros && !LastLong)
				{
					HasLongEvent = true;
					HasShortEvent = false;
					LastLong = true;
				}
			}
		}
		else
		{
			if (LastPressed)
			{
				const uint32_t elapsed = timestamp - PressedTimestamp;
				if (elapsed > ShortPressThresholdMicros)
				{
					HasLongEvent = false;
					HasShortEvent = !LastLong;
				}
				else
				{
					HasLongEvent = false;
					HasShortEvent = false;
				}
			}

			LastLong = false;
		}

		LastPressed = pressed;

		return HasShortEvent || HasLongEvent;
	}
};
#endif

