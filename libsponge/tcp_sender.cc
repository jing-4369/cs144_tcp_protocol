#include "tcp_sender.hh"

#include "buffer.hh"
#include "tcp_config.hh"
#include "tcp_segment.hh"
#include "wrapping_integers.hh"
#include "tcp_timer.hh"

#include <algorithm>
#include <bits/stdint-uintn.h>
#include <cstddef>
#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity) {}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_flight; }

void TCPSender::fill_window() {
    TCPSegment seg;
	if (_next_seqno == 0) {
		seg.header().syn = true;
		_next_seqno++;
	}
	else {
		size_t seg_size = min(TCPConfig::MAX_PAYLOAD_SIZE, min(_stream.buffer_size(), _window_size));
		seg.parse(Buffer(_stream.read(seg_size)));
	}
	seg.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(seg);
    _segments_flying.push_back(seg);
    _bytes_flight += seg.length_in_sequence_space();
    if (!_timer.is_running()) {
        _timer.run_timer(_cur_time);
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    uint64_t abs_ackno = unwrap(ackno, _isn, next_seqno_absolute());
    while (!_segments_flying.empty()) {
        WrappingInt32 cur_no =
            _segments_flying.front().header().seqno + _segments_flying.front().length_in_sequence_space();
        uint64_t abs_curno = unwrap(cur_no, _isn, next_seqno_absolute());
        if (abs_ackno > abs_curno) {
            _bytes_flight -= _segments_flying.front().length_in_sequence_space();
            _segments_flying.pop_front();
        }
    }
    _next_seqno = abs_ackno;
    _window_size = window_size;
    fill_window();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    _cur_time += ms_since_last_tick;
    if (_timer.is_running() && _timer.is_expired(ms_since_last_tick)) {
        _timer.double_timeout();
        _timer.run_timer(_cur_time);
        consecutive_retransmissions();
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _timer.consecutive_number(); }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = next_seqno();
    _segments_out.push(seg);
}
