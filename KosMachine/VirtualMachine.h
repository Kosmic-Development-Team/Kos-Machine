#pragma once

#include "stdafx.h"
#include <fstream>
#include <sstream>
#include <iomanip>
using namespace std;

class VirtualMachine {
public:
	VirtualMachine(string file, string fileReg);
	~VirtualMachine();
	short run();
	void saveState(string path);
private:
	static const int MEM_SIZE = 65536;
	static const int NUM_REG = 256;
	short * mem;
	short * reg;
	void executeNext();
	short toWord(string hex);
	string toHex(short word);
	void binInitReg(string fileReg);
	void hexInitReg(string fileReg);
	void binInitMem(string file);
	void hexInitMem(string file);
	void initStates(string fileName, string fileReg);
};

void initBlank(short * block, int length);

