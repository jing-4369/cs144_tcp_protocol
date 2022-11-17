#include "stream_reassembler.hh"

#include <cstddef>
#include <cstdint>
#include <iterator>
//#include <algorithm>
// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {
    cur_index = 0;
    end_index = INT32_MAX;
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if (eof) {
        end_index = index + data.size();
    }
    if (index > cur_index) {
        for (size_t i = 0; i < data.size() && buffer.size() <= _capacity; i++) {
            buffer.insert({index + i, data[i]});
        }
    } else {
        string str;
        size_t remain_capacity = stream_out().remaining_capacity();
        for (size_t i = cur_index - index; i < data.size() && str.size() < remain_capacity; i++) {
            str += data[i];
            cur_index++;
        }
		auto iter = buffer.begin();
		while (!buffer.empty() && iter->first < cur_index) {
			buffer.erase(buffer.begin());
			iter = buffer.begin();
		}
		while (!buffer.empty() && iter->first == cur_index && str.size() < remain_capacity) {
			str += iter->second;
			cur_index++;
			buffer.erase(buffer.begin());
			iter = buffer.begin();
		}
        stream_out().write(str);
        if (cur_index == end_index) {
            stream_out().end_input();
        }
    }
}

size_t StreamReassembler::unassembled_bytes() const { return buffer.size(); }

bool StreamReassembler::empty() const { return buffer.empty(); }
