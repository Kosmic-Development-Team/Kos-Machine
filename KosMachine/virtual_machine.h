#pragma once

#include "stdafx.h"
#include <fstream>
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
	short flags; // 0-true 1-zero, 2-negative, 3-carry;
	short running;

	void initStates(string fileName, string fileReg);

	void binInitReg(string fileReg);
	void hexInitReg(string fileReg);
	void binInitMem(string file);
	void hexInitMem(string file);

	string executeNext();

	void setFlags(short result, short carry);

	short jumps(short opcode, short operand);
	short moves(short opcode, short operand);
	short arith(short opcode, short operand);
	short logic(short opcode, short operand);
};


short toWord(string hex);
string toHex(short word);
void initBlank(short * block, int length);

