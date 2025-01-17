/**
 * @file
 */

#include "BufferedReadWriteStream.h"

namespace io {

BufferedReadWriteStream::BufferedReadWriteStream(int64_t size) {
	resizeBuffer(size);
}

BufferedReadWriteStream::~BufferedReadWriteStream() {
	core_free(_buffer);
}

void BufferedReadWriteStream::resizeBuffer(int64_t size) {
	if (size == 0u) {
		return;
	}
	if (_capacity >= size) {
		return;
	}
	_capacity = align(size);
	if (!_buffer) {
		_buffer = (uint8_t*)core_malloc(_capacity);
	} else {
		_buffer = (uint8_t*)core_realloc(_buffer, _capacity);
	}
}

int BufferedReadWriteStream::write(const void *buf, size_t size) {
	if (size == 0) {
		return 0;
	}
	resizeBuffer(_pos + (int64_t)size);
	core_memcpy(&_buffer[_pos], buf, size);
	_pos += (int64_t)size;
	_size = core_max(_pos, _size);
	return 0;
}

int BufferedReadWriteStream::read(void *buf, size_t size) {
	if (_pos + (int64_t)size > _size) {
		return -1;
	}
	core_memcpy(buf, &_buffer[_pos], size);
	_pos += (int64_t)size;
	return 0;
}

int64_t BufferedReadWriteStream::seek(int64_t position, int whence) {
	const int64_t s = size();
	int64_t newPos = -1;
	switch (whence) {
	case SEEK_SET:
		newPos = position;
		break;
	case SEEK_CUR:
		newPos = _pos + position;
		break;
	case SEEK_END:
		newPos = s + position;
		break;
	default:
		return -1;
	}
	if (newPos < 0) {
		newPos = 0;
	} else if (newPos > s) {
		newPos = s;
	}
	_pos = newPos;
	return 0;
}

int64_t BufferedReadWriteStream::pos() const {
	return _pos;
}

int64_t BufferedReadWriteStream::size() const {
	return _size;
}

} // namespace io
