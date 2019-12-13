#ifndef _INC_STREAMREADER_
#define _INC_STREAMREADER_

#include <stdint.h>

class StreamReader {
	public:
	StreamReader(uint8_t *data) {
		data_ = data;
		offset_ = 0;
	}

	StreamReader &Read(uint8_t &val) {
		val = *(uint8_t*)(data_ + offset_);
		offset_++;
	}
	StreamReader &Read(uint16_t &val) {
		val = *(uint16_t*)(data_ + offset_);
		offset_ += sizeof(val);
	}
	StreamReader &Read(uint32_t &val) {
		val = *(uint32_t*)(data_ + offset_);
		offset_ += sizeof(val);
	}
	StreamReader &Skip(int len) {
		offset_ += len;
	}
	StreamReader &Offset(int off) {
		offset_ = off;
	}

	private:
	uint8_t *data_;
	int offset_;
};

#endif
