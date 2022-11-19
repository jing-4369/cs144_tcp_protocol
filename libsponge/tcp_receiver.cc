#include "tcp_receiver.hh"

#include "wrapping_integers.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if (seg.header().syn) {
        SYN = true;
        isn = seg.header().seqno;
        if (seg.header().fin) {
            FIN = true;
            stream_out().end_input();
        }
        if (seg.payload().size()) {
            _reassembler.push_substring(seg.payload().copy(),
                                        unwrap(seg.header().seqno + 1, isn, stream_out().bytes_written() + 1) - 1,
                                        seg.header().fin);
        }
        return;
    }
    if (SYN && seg.header().fin) {
        FIN = true;
    }
    if (SYN) {
        _reassembler.push_substring(seg.payload().copy(),
                                    unwrap(seg.header().seqno, isn, stream_out().bytes_written() + 1) - 1,
                                    seg.header().fin);
    }
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!SYN) {
        return {};
    } else if (FIN && _reassembler.empty()) {
        return wrap(stream_out().bytes_written() + 2, isn);
    } else {
        return wrap(stream_out().bytes_written() + 1, isn);
    }
}

size_t TCPReceiver::window_size() const { return stream_out().remaining_capacity(); }
