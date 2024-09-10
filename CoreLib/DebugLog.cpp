#include "pch.h"
#include "DebugLog.h"
#include <iomanip>
#include <sstream>

string getCurrentTime() {
    // 현재 시간을 얻음
    auto now = chrono::system_clock::now();
    time_t currentTime = chrono::system_clock::to_time_t(now);

    // 안전한 방식으로 struct tm을 채움
    struct tm localTime;
    localtime_s(&localTime, &currentTime); // localtime_s 사용

    // 시간을 문자열로 변환 (YYYY-MM-DD HH:MM:SS 형식)
    stringstream timeStream;
    timeStream << put_time(&localTime, "%Y-%m-%d %H:%M:%S");

    return timeStream.str();
}

void DebugLog::PrintColorText(const LogColor& color, const string& text, const string& variable, const bool& isDisplayTime, const bool& isEnter)
{
    cout << "\033[" << color << "m";
    
    if (isDisplayTime) {
    string currentTime = getCurrentTime();
        cout << "[" << currentTime << "]";
    
    }
	cout << text << variable << "\033[0m";

	if (isEnter)
		cout << endl;
}
