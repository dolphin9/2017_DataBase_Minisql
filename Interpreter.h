

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <string>
#include <vector>
#include "API.h"
using namespace std;
class Interpreter{
public:

    API* ap;
	string fileName ;
	int interpreter(string s);

	string getWord(string s, int *st);
	bool getCon(string s);

	Interpreter(string s) { fileName = s; }
	~Interpreter(){}
};

#endif
