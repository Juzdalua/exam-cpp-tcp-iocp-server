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
		// �ּ��̳� �� ������ ����
		if (line.empty() || line[0] == '#') continue;

		istringstream lineStream(line);
		string key;
		if (getline(lineStream, key, '=')) {
			string value;
			if (getline(lineStream, value)) {
				// ȯ�� ���� ����
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

	// _dupenv_s�� ����Ͽ� ȯ�� ���� �� ��������
	if (_dupenv_s(&buffer, &size, varName.c_str()) == 0 && buffer != nullptr) {
		string value(buffer);
		free(buffer);  // �������� �Ҵ�� �޸� ����
		return value;
	}
	else {
		cerr << "----- Failed to retrieve the environment variable or variable is not set. -----" << endl;
		return "";
	}
}
