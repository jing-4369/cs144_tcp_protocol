#include "tcp_timer.hh"

TCPTimer::TCPTimer(const unsigned int timeout): _retransmission_timeout(timeout){
	_start_time = 0;
	_cur_time = 0;
	_consecutive_number = 0;
	_is_run = false;
}

void TCPTimer::run_timer(const unsigned int start_time){
	_start_time = start_time;
	_cur_time = _start_time;
	_is_run = true;
}

void TCPTimer::stop_timer(){
	_is_run = false;
}

void TCPTimer::double_timeout(){
	_retransmission_timeout *= 2;
	_consecutive_number++;
}

void TCPTimer::reset_timeout(const unsigned int timeout){
	_retransmission_timeout = timeout;
	_consecutive_number = 0;
}

bool TCPTimer::is_expired(const unsigned int ms_since_last_call) {
	_cur_time += ms_since_last_call;
	if (_cur_time - _start_time>= _retransmission_timeout) {
		return true;
	}
	return false;
}

bool TCPTimer::is_running() const{
	return _is_run;
}

unsigned int TCPTimer::consecutive_number() const{
	return _consecutive_number;
}
