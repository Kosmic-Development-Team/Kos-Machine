#include "stdafx.h"
#include "VirtualMachine.h"


VirtualMachine::VirtualMachine(string file, string fileReg = "") {
	mem = new short[MEM_SIZE];
	reg = new short[NUM_REG];
	initStates(file, fileReg);
}


VirtualMachine::~VirtualMachine() {
	delete mem;
	delete reg;
}

short VirtualMachine::run() {
	return 0;
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

void VirtualMachine::executeNext() {
}

short VirtualMachine::toWord(string hex) { //TODO: This is broken right now
	istringstream hexConv(hex);
	short w;
	hexConv >> std::hex >> w;
	return w;
}

string VirtualMachine::toHex(short word) {
	return string();
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
		if ((i & 3) == 0) {
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
		if ((i & 3) == 0) {
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
		if (suffix.compare(".kmbrg") == 0) {
			binInitReg(fileReg);
		} else if (suffix.compare(".kmhrg") == 0) {
			hexInitReg(fileReg);
		} else {
			initBlank(reg, NUM_REG);
		}
	} else {
		initBlank(reg, NUM_REG);
	}
	if (file.length() >= 6) {
		string suffix = file.substr(file.length() - 6);
		if (suffix.compare(".kmbrm") == 0) {
			binInitMem(file);
		} else if(suffix.compare(".kmhrm") == 0) {
			hexInitMem(file);
		} else {
			initBlank(mem, MEM_SIZE);
		}
	} else {
		initBlank(mem, MEM_SIZE);
	}
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
