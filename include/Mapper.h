#ifndef MAPPER_H
#define MAPPER_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

class Mapper
{
public:
	virtual void load(const char * pgrRom, const size_t pgrLen, const char * chrRom, const size_t chrLen) = 0;
	virtual uint8_t read(uint16_t a) = 0;
	virtual void write(uint16_t a, uint8_t v) = 0;
	virtual uint8_t readCHR(uint16_t a) = 0;
	virtual void writeCHR(uint16_t a, uint8_t v) = 0;
};

#endif // MAPPER_H

