#include "byte_stream.hh"

#include <functional>
#include <iostream>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) : _capacity(capacity), buffer(capacity) {}

size_t ByteStream::write(const string &data) {
    int size = 0;
    for (auto ch : data) {
        if (_bytes_write > _bytes_read && write_ptr == read_ptr) {
            set_error();
            break;
        }
        buffer[write_ptr] = ch;
        write_ptr = (write_ptr + 1) % _capacity;
        _bytes_write++;
        size++;
    }
    return size;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    string bytes;
    int tmp_ptr = read_ptr;
    for (size_t i = 0; i < len; i++) {
        bytes += buffer[tmp_ptr];
        tmp_ptr = (tmp_ptr + 1) % _capacity;
    }
    return bytes;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (_bytes_read == _bytes_write && read_ptr == write_ptr) {
            set_error();
            break;
        }
        read_ptr = (read_ptr + 1) % _capacity;
        _bytes_read++;
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string bytes = peek_output(len);
    pop_output(len);
    return bytes;
}

void ByteStream::end_input() { _input_ended = 1; }

bool ByteStream::input_ended() const { return _input_ended; }

size_t ByteStream::buffer_size() const { return _bytes_write - _bytes_read; }

bool ByteStream::buffer_empty() const { return buffer_size() == 0; }

bool ByteStream::eof() const { return _input_ended && buffer_size() == 0; }

size_t ByteStream::bytes_written() const { return _bytes_write; }

size_t ByteStream::bytes_read() const { return _bytes_read; }

size_t ByteStream::remaining_capacity() const { return _capacity - buffer_size(); }
