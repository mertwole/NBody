#include "LoadFile.h"

#include <iostream>
#include <fstream>

void load_file(const char* filename, std::string* content) {
	std::fstream f(filename, (std::fstream::in | std::fstream::binary));
	if (!f.is_open()) {
		std::cout << "failed to open file\n:" << filename << std::endl;
		exit(1);
	}
	size_t file_size;
	f.seekg(0, std::fstream::end);
	file_size = (size_t)f.tellg();
	f.seekg(0, std::fstream::beg);
	char* str = new char[file_size + 1];
	f.read(str, file_size);
	f.close();
	str[file_size] = '\0';
	*content = str;
	delete[] str;
}