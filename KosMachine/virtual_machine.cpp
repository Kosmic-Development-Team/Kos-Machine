#include "stdafx.h"
#include "virtual_machine.h"


VirtualMachine::VirtualMachine(string file, string fileReg = "") {
	mem = new short[MEM_SIZE];
	reg = new short[NUM_REG];
	flags = 0x1;
	running = 0x1;
	initStates(file, fileReg);
}


VirtualMachine::~VirtualMachine() {
	delete mem;
	delete reg;
}

short VirtualMachine::run() {
	while (running) {
		cout << executeNext() << '\n';
	}
	return reg[0x07];
}

string VirtualMachine::executeNext() {
	short opcode = mem[reg[0x00]];
	short operand = mem[reg[0x00] + 1];
	switch (opcode) {
	case 0x0000:
		reg[0x00]++;
		return "0000 ----";
	case 0x000F:
		setFlags(reg[0x02], 0x0);
		reg[0x00]++;
		return "000F ----";
	}
	if ((!(opcode & 0xF00) && (opcode & 0xF0)) ||
		(((opcode & 0xF000) == 0x8000) && (!(opcode & 0xF) || (opcode & 0xF) > 0x8)) ||
		(((opcode & 0xF000) == 0xC000) && (!(opcode & 0xF00) || (opcode & 0xF00) > 0x8))){
		reg[0x00]++;
		return opcode + " ---- not matching any current. No operation replace.";
	}

	switch (opcode & 0xFF00) {
	case 0xFF00:
		reg[0x07] = reg[opcode & 0xFF];
		running = 0x0;
		reg[0x00]++;
		return toHex(opcode) + " ----";
	case 0xC700:
		reg[opcode & 0xFF] = mem[--reg[0x01]];
		reg[0x00]++;
		return toHex(opcode) + " ----";
	case 0xC800:
		mem[reg[0x01]++] = reg[opcode & 0xFF];
		reg[0x00]++;
		return toHex(opcode) + " ----";
	}

	short ppAdd = 0x0;

	switch (opcode & 0xC000) {
	case 0x0000:
		ppAdd = logic(opcode, operand);
		break;
	case 0x4000:
		ppAdd = arith(opcode, operand);
		break;
	case 0x8000:
		ppAdd = jumps(opcode, operand);
		break;
	case 0xC000:
		ppAdd = moves(opcode, operand);
		break;
	default:
		reg[0x00]++;
		return opcode + " ---- not matching any current. No operation replace.";
	}
	reg[0x00] += ppAdd;
	if (!(opcode & 0x0F00)) {
		return toHex(opcode) + " " + toHex(operand);
	}
	return toHex(opcode) + " ----";
}

void VirtualMachine::setFlags(short result, short carry) {
	flags = 0x1;
	if (result == 0x0) {
		flags = 0x2;
	}
	if (result & 0x8000) {
		flags |= 0x4;
	}
	if (carry) {
		flags |= 0x8;
	}
}

short VirtualMachine::jumps(short opcode, short operand) {
	short subOp, shortened = opcode & 0xF00;
	if (!shortened) {
		subOp = opcode & 0xF;
	}
	switch (subOp & 0x3) {
	case 0x0:
		if (!(flags & 0x8)) {
			return 0x0;
		}
	case 0x1:
		break;
	case 0x2:
		if (!(flags & 0x2)) {
			return 0x0;
		}
	case 0x3:
		if (!(flags & 0x4)) {
			return 0x0;
		}
	}
	if (subOp <= 0x4) {
		if (shortened) {
			reg[0x00] = reg[opcode & 0xFF];
		} else {
			reg[0x00] = operand;
		}
		return 0x0;
	}
	if (shortened) {
		return reg[opcode & 0xFF];
	}
	return operand;
}

short VirtualMachine::moves(short opcode, short operand) {
	short rg = opcode & 0xFF;
	switch (opcode & 0xF00) {
	case 0x100:
		reg[rg] = mem[operand];
		return 0x2;
	case 0x200:
		reg[rg] = reg[operand & 0xFF];
		return 0x2;
	case 0x300:
		reg[rg] = mem[reg[operand & 0xFF]];
		return 0x2;
	case 0x400:
		reg[rg] = operand;
		return 0x2;
	case 0x500:
		mem[operand] = reg[rg];
		return 0x2;
	case 0x600:
		mem[reg[operand & 0xFF]] = reg[rg];
		return 0x2;
	}
}

short VirtualMachine::arith(short opcode, short operand) {
	short subOp, shortened = opcode & 0xF00;
	if (!shortened) {
		subOp = opcode & 0xF;
	}
	short result = 0x0;
	switch (subOp) {
	case 0x1:
		int r = (int)reg[0x02] + (int)reg[0x03];
		result = (short)r;
		setFlags(result, r != ((int)result));
		break;
	case 0x2:
		int r = (int)reg[0x02] + (int)reg[0x03] + (int)(flags >> 3);
		result = (short)r;
		setFlags(result, r != ((int)result));
		break;
	case 0x3:
		int r = (int)reg[0x02] + (int)(~reg[0x03]) + 1;
		result = (short)r;
		setFlags(result, r != ((int)result));
		break;
	case 0x4:
		int r = (int)reg[0x02] + (int)(~reg[0x03]) + (int)(flags >> 3);
		result = (short)r;
		setFlags(result, r != ((int)result));
		break;
	case 0x5:
		result = ~reg[0x02];
		setFlags(result, 0x0);
		break;
	}
	if (shortened) {
		reg[opcode & 0xFF] = result;
		return 0x1;
	}
	mem[operand] = result;
	return 0x2;
}

short VirtualMachine::logic(short opcode, short operand) {
	short subOp, shortened = opcode & 0xF00;
	if (!shortened) {
		subOp = opcode & 0xF;
	}
	short result = 0x0;
	switch (subOp) {
	case 0x1:
		result = ~reg[0x02];
		setFlags(result, 0x0);
		break;
	case 0x2:
		result = reg[0x02] & reg[0x03];
		setFlags(result, 0x0);
		break;
	case 0x3:
		result = reg[0x02] | reg[0x03];
		setFlags(result, 0x0);
		break;
	case 0x4:
		result = reg[0x02] ^ reg[0x03];
		setFlags(result, 0x0);
		break;
	case 0x5:
		result = ~(reg[0x02] & reg[0x03]);
		setFlags(result, 0x0);
		break;
	case 0x6:
		result = ~(reg[0x02] | reg[0x03]);
		setFlags(result, 0x0);
		break;
	case 0x7:
		result = ~(reg[0x02] ^ reg[0x03]);
		setFlags(result, 0x0);
		break;
	case 0x8:
		result = reg[0x02] << 1;
		setFlags(result, 0x0);
		break;
	case 0x9:
		result = (reg[0x02] << 1) | ((reg[0x02] >> 15) & 0x1);
		setFlags(result, 0x0);
		break;
	case 0xA:
		short c = reg[0x02] & 0x8000;
		result = (reg[0x02] << 1) | (flags >> 3);
		setFlags(result, c);
		break;
	case 0xB:
		result = reg[0x02] >> 1;
		setFlags(result, 0x0);
		break;
	case 0xC:
		result = (reg[0x02] >> 1) & 0x7FFF;
		setFlags(result, 0x0);
		break;
	case 0xD:
		result = (reg[0x02] >> 1) | (reg[0x02] << 15);
		setFlags(result, 0x0);
		break;
	case 0xE:
		short c = reg[0x02] & 0x1;
		result = (reg[0x02] >> 1) | ((flags << 12) & 0x8000);
		setFlags(result, c);
		break;
	}
	if (shortened) {
		reg[opcode & 0xFF] = result;
		return 0x1;
	}
	mem[operand] = result;
	return 0x2;
}

void VirtualMachine::saveState(string path) {
	ofstream regOut(path + ".kmbrg", ofstream::out | ofstream::binary | ofstream::trunc);
	if (regOut.is_open()) {
		regOut.write((char *) reg, NUM_REG * 2);
		regOut.close();
	}
	else {
		cout << "Unable to write register states to " + path + ".kmbrg";
	}
	ofstream memOut(path + ".kmbrm", ofstream::out | ofstream::binary | ofstream::trunc);
	if (memOut.is_open()) {
		memOut.write((char *) mem, MEM_SIZE * 2);
		memOut.close();
	}
	else {
		cout << "Unable to write memory state to " + path + ".kmbrm";
	}
}

void VirtualMachine::binInitReg(string fileReg) {
	ifstream stream(fileReg);
	char * charReg = (char *) reg;
	char c = stream.get();
	int i = 0;
	for (i; i < (NUM_REG * 2) && stream.good(); ++i) {
		charReg[i] = c;
		c = stream.get();
	}
	for (i; i < (NUM_REG * 2); ++i) {
		charReg[i] = 0x00;
	}
	stream.close();
}

void VirtualMachine::hexInitReg(string fileReg) {
	ifstream stream(fileReg, ifstream::in);
	char c;
	do {
		c = (char) stream.get();
	} while ((c == ' ' || c == '\n') && stream.good());
	string hex = "";
	int i = 1;
	int index = 0;
	for (i; i <= (NUM_REG * 4) && stream.good(); ++i) {
		hex += c;
		if (!(i & 3)) {
			mem[index] = toWord(hex);
			index++;
			hex = "";
		}
		do {
			c = (char) stream.get();
		} while ((c == ' ' || c == '\n') && stream.good());
		
	}
	for (index; index < NUM_REG; ++index) {
		reg[index] = 0x0000;
	}
	stream.close();
}

void VirtualMachine::binInitMem(string file) {
	ifstream stream(file);
	char * charMem = (char *) mem;
	char c = stream.get();
	int i = 0;
	for (i; i < (MEM_SIZE * 2) && stream.good(); ++i) {
		charMem[i] = c;
		c = stream.get();
	}
	for (i; i < (MEM_SIZE * 2); ++i) {
		charMem[i] = 0x00;
	}
	stream.close();
}

void VirtualMachine::hexInitMem(string file) {
	ifstream stream(file, ifstream::in);
	char c;
	do {
		c = (char) stream.get();
	} while ((c == ' ' || c == '\n') && stream.good());
	string hex = "";
	int i = 1;
	int index = 0;
	for (i; i <= (MEM_SIZE * 4) && stream.good(); ++i) {
		hex += c;
		if (!(i & 3)) {
			mem[index] = toWord(hex);
			index++;
			hex = "";
		}
		do {
			c = (char) stream.get();
		} while ((c == ' ' || c == '\n') && stream.good());
	}
	for (index; index < MEM_SIZE; ++index) {
		mem[index] = 0x0000;
	}
	stream.close();
}

void VirtualMachine::initStates(string file, string fileReg = "") {
	if (fileReg.length() >= 6) {
		string suffix = fileReg.substr(fileReg.length() - 6);
		if (!suffix.compare(".kmbrg")) {
			binInitReg(fileReg);
		} else if (!suffix.compare(".kmhrg")) {
			hexInitReg(fileReg);
		} else {
			initBlank(reg, NUM_REG);
		}
	} else {
		initBlank(reg, NUM_REG);
	}
	if (file.length() >= 6) {
		string suffix = file.substr(file.length() - 6);
		if (!suffix.compare(".kmbrm")) {
			binInitMem(file);
		} else if(!suffix.compare(".kmhrm")) {
			hexInitMem(file);
		} else {
			initBlank(mem, MEM_SIZE);
		}
	} else {
		initBlank(mem, MEM_SIZE);
	}
}

short toWord(string hex) {
	short w = 0x0000;
	for (int i = 0; i < 4; ++i) {
		char c = hex[i];
		if (c >= '0' && c <= '9') {
			w = (w << 4) | (c & 0xF);
		}
		else if ((c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
			w = (w << 4) | ((c & 0xF) + 0x9);
		}
	}
	return w;
}

string toHex(short word) {
	return string();
}

void initBlank(short * block, int length) {
	for (int i = 0; i < length; i++) {
		block[i] = 0x0000;
	}
}

int main() {
	VirtualMachine vm("../VM State Files/test1.kmhrm");
	vm.saveState("../VM State Files/saveTest1");
}
