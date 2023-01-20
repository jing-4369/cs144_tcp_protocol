#include "tcp_connection.hh"

#include "tcp_config.hh"
#include "tcp_segment.hh"

#include <bits/stdint-intn.h>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <limits>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _cur_time - _receive_seg_time; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    _receive_seg_time = _cur_time;
    if (seg.header().rst) {
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _active = false;
        return;
    }

    if (!_receiver.ackno().has_value() ||
        ((seg.header().seqno - _receiver.ackno().value() +
              static_cast<int32_t>(_receiver.stream_out().bytes_written()) >=
          0) &&
         (seg.header().seqno - _receiver.ackno().value() - static_cast<int32_t>(_receiver.window_size()) < 0))) {
        _receiver.segment_received(seg);
    }
    if (inbound_stream().input_ended() && !_sender.stream_in().input_ended()) {
        _linger_after_streams_finish = false;
    }

    if (seg.header().syn && _sender.next_seqno_absolute() == 0) {
        connect();
        return;
    }
    if (seg.header().ack) {
        if (_sender.next_seqno_absolute() != 0) {
            _sender.ack_received(seg.header().ackno, seg.header().win);
            _sender.fill_window();
        }
    }
    bool need_send_empty = _receiver.ackno().has_value() && (seg.length_in_sequence_space() == 0) &&
                           seg.header().seqno == _receiver.ackno().value() - 1 && _sender.segments_out().empty();
    need_send_empty |= seg.length_in_sequence_space();
    if (need_send_empty) {
        _sender.send_empty_segment();
    }
    set_seg_state_and_send();
}

bool TCPConnection::active() const { return _active; }

size_t TCPConnection::write(const string &data) {
    size_t write_size = _sender.stream_in().write(data);
    _sender.fill_window();
    set_seg_state_and_send();
    return write_size;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    _cur_time += ms_since_last_tick;
    _sender.tick(ms_since_last_tick);
    if (_sender.consecutive_retransmissions() > _cfg.MAX_RETX_ATTEMPTS) {
        // send RST
        if (_sender.segments_out().empty()) {
            _sender.send_empty_segment();
        }
        TCPSegment seg = _sender.segments_out().front();
        seg.header().rst = true;
        _segments_out.push(seg);
        _sender.segments_out().pop();
        _active = false;
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        // _linger_after_streams_finish = false;
        return;
    }
    set_seg_state_and_send();
    if ((_sender.bytes_in_flight() == 0) && inbound_stream().input_ended() && _sender.stream_in().input_ended() &&
        _receiver.unassembled_bytes() == 0 &&
        (_sender.next_seqno_absolute() == _sender.stream_in().bytes_written() + 2)) {
        if (_linger_after_streams_finish) {
            if (time_since_last_segment_received() >= 10 * _cfg.rt_timeout) {
                _active = false;
            }
        } else {
            _active = false;
        }
    }
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    // send FIN
    _sender.fill_window();
    set_seg_state_and_send();
}

void TCPConnection::connect() {
    _sender.fill_window();
    set_seg_state_and_send();
}

void TCPConnection::set_seg_state_and_send() {
    while (!_sender.segments_out().empty()) {
        TCPSegment seg = _sender.segments_out().front();
        _sender.segments_out().pop();
        if (_receiver.ackno().has_value()) {
            seg.header().ack = true;
            seg.header().ackno = _receiver.ackno().value();
        }
        seg.header().win = min(static_cast<size_t>(UINT16_MAX), _receiver.window_size());
        _segments_out.push(seg);
    }
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
            // Your code here: need to send a RST segment to the peer
            TCPSegment seg;
            _sender.send_empty_segment();
            seg = _sender.segments_out().front();
            _sender.segments_out().pop();
            seg.header().rst = true;
            _segments_out.push(seg);
            _sender.stream_in().set_error();
            _receiver.stream_out().set_error();
            _active = false;
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
