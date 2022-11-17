#include "stream_reassembler.hh"
#include <cstddef>
//#include <algorithm>
// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity){
	cur = 0;
	cur_size = 0;
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
	if(eof){
		end = data.size() + index;
		if(cur == end){
			stream_out().end_input();
			return;
		}
	}
	if (index <= cur){
		if(cur - index < data.size()){
			stream_out().write(data.substr(cur - index));
			cur = min(_capacity, index + data.size());
			if(cur == end){
				stream_out().end_input();
				return;
			}
			while(!buffer.empty() && cur >= buffer.begin()->first){
				auto iter = buffer.begin();
				size_t t_index = iter->first;
				string t_data = iter->second;
				cur_size -= t_data.size();
				if(cur - t_index < t_data.size()){
					stream_out().write(t_data.substr(cur - t_index));
					cur = min(_capacity, t_index + t_data.size());
				}
				if(cur == end){
					stream_out().end_input();
					return;
				}
				buffer.erase(buffer.begin());
			}
		}
	}
	else if(data.size() <= _capacity - cur_size) {
		buffer.insert({index, data});
		cur_size += data.size();
	}
}

size_t StreamReassembler::unassembled_bytes() const { 
	int res = 0;
	size_t pre_index = 0;
	for (auto iter = buffer.begin(); iter != buffer.end(); iter++) {
		size_t index = iter->first;
		string data = iter->second;
		if(index >= pre_index){
			res += data.size();
			pre_index = index + data.size();
		}
		else if(pre_index < index + data.size()){
			res += data.size() + index - pre_index;
			pre_index = index + data.size();
		}
	}
	return res;
}

bool StreamReassembler::empty() const { 
	return buffer.empty(); 
}
