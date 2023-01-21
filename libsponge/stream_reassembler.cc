#include "stream_reassembler.hh"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iterator>
#include <numeric>
// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if (eof) {
        eof_index = index + data.size();
        if (cur_index == eof_index) {
            stream_out().end_input();
        }
    }
    if (index > cur_index) {
        if (buffer.empty()) {
            buffer.insert({index, data});
            _unassembled_bytes += data.size();
            return;
        }
        auto iter = buffer.upper_bound(index);
        --iter;
        string new_data(data);
        size_t new_index(index);
        while (iter != buffer.end()) {
            size_t begin_index = new_index, end_index = new_index + new_data.size();
            size_t tmp_begin_index = iter->first, tmp_end_index = iter->first + iter->second.size();
            if (end_index < tmp_begin_index || tmp_end_index < begin_index) {
                break;
            }
            if (begin_index < tmp_begin_index) {
                if (end_index <= tmp_end_index) {
                    new_data = new_data + iter->second.substr(end_index - tmp_begin_index);
                }
            } else {
                new_index = tmp_begin_index;
                if (tmp_end_index <= end_index) {
                    new_data = iter->second + new_data.substr(tmp_end_index - begin_index);
                } else {
                    new_data = iter->second;
                }
            }
            _unassembled_bytes -= iter->second.size();
            buffer.erase(iter++);
        }
        buffer.insert({new_index, new_data});
        _unassembled_bytes += new_data.size();
        return;
    }
    if (index + data.size() <= cur_index) {
        return;
    }
    size_t write_lens = min(_output.remaining_capacity(), data.size() - cur_index + index);
    if (write_lens <= 0) {
        return;
    }
    _output.write(data.substr(cur_index - index, write_lens));
    cur_index = cur_index + write_lens;
    auto iter = buffer.begin();
    while (!buffer.empty() && iter->first <= cur_index) {
        if (iter->first + iter->second.size() > cur_index) {
            write_lens = min(_output.remaining_capacity(), iter->second.size() - cur_index + iter->first);
            _output.write(iter->second.substr(cur_index - iter->first, write_lens));
            cur_index = cur_index + write_lens;
        }
        buffer.erase(iter);
        _unassembled_bytes -= iter->second.size();
        iter = buffer.begin();
    }
    if (cur_index == eof_index) {
        stream_out().end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return buffer.empty(); }
