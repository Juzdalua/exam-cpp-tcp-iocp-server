#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>  // _dupenv_s, free
#include <vector>

class ENV {
public:
	// .env 파일을 읽어 환경 변수로 설정하는 함수
	static void loadEnvFile(const string& filename);

	// 환경 변수를 안전하게 읽어오는 함수
	static string getEnvVariable(const string& varName);


};
