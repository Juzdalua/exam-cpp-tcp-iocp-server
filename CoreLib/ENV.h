#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>  // _dupenv_s, free
#include <vector>

class ENV {
public:
	// .env ������ �о� ȯ�� ������ �����ϴ� �Լ�
	static void loadEnvFile(const string& filename);

	// ȯ�� ������ �����ϰ� �о���� �Լ�
	static string getEnvVariable(const string& varName);


};
