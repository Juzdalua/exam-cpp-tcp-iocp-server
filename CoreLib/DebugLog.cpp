#include "pch.h"
#include "DebugLog.h"
#include <iomanip>
#include <sstream>

string getCurrentTime() {
    // ���� �ð��� ����
    auto now = chrono::system_clock::now();
    time_t currentTime = chrono::system_clock::to_time_t(now);

    // ������ ������� struct tm�� ä��
    struct tm localTime;
    localtime_s(&localTime, &currentTime); // localtime_s ���

    // �ð��� ���ڿ��� ��ȯ (YYYY-MM-DD HH:MM:SS ����)
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
