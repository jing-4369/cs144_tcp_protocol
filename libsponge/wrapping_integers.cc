#include "wrapping_integers.hh"
#include <bits/stdint-uintn.h>
#include <cmath>
#include <cstdint>

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
	uint32_t res = isn.raw_value() + static_cast<uint32_t>(n);
    return WrappingInt32{res};
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
	uint32_t biggest_offset = (1ull << 31) - 1;
	uint32_t abs_n =  n - isn;
	if(checkpoint < biggest_offset){
		return static_cast<uint64_t>(abs_n);
	}
	uint64_t res = (checkpoint & ~((1ull << 32) - 1)) + abs_n;
	uint32_t check = static_cast<uint32_t>(checkpoint);
	if(check > abs_n && check - abs_n > biggest_offset)
		res += 1ul << 32;
	else if(check < abs_n && abs_n - check > biggest_offset ) 
		res -= 1ul << 32;
    return res;
}
