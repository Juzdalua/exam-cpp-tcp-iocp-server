#include "pch.h"
#include "ENV.h"

#ifdef _WIN32
#define setenv(name, value, overwrite) _putenv_s(name, value)
#endif

void ENV::loadEnvFile(const string& filename)
{
	ifstream envFile(filename);
	string line;

	if (!envFile.is_open()) {
		cerr << "----- Failed to open .env file. -----" << endl;
		return;
	}

	while (getline(envFile, line)) {
		// 주석이나 빈 라인은 무시
		if (line.empty() || line[0] == '#') continue;

		istringstream lineStream(line);
		string key;
		if (getline(lineStream, key, '=')) {
			string value;
			if (getline(lineStream, value)) {
				// 환경 변수 설정
				_putenv_s(key.c_str(), value.c_str());
			}
		}
	}

	envFile.close();
}

string ENV::getEnvVariable(const string& varName)
{
	char* buffer = nullptr;
	size_t size = 0;

	// _dupenv_s를 사용하여 환경 변수 값 가져오기
	if (_dupenv_s(&buffer, &size, varName.c_str()) == 0 && buffer != nullptr) {
		string value(buffer);
		free(buffer);  // 동적으로 할당된 메모리 해제
		return value;
	}
	else {
		cerr << "----- Failed to retrieve the environment variable or variable is not set. -----" << endl;
		return "";
	}
}
