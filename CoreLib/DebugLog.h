#pragma once

enum LogColor
{
	WHITE = 0,
	RED = 31,
	GREEN = 32,
	YELLOW = 33,
	BLUE = 34,
};

class DebugLog
{
public:
	static void PrintColorText(const LogColor& color, const string& text, const string& variable, const bool& isDisplayTime, const bool& isEnter);
};

