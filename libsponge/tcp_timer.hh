#ifndef SPONGE_LIBSPONGE_TCP_TIMER_HH
#define SPONGE_LIBSPONGE_TCP_TIMER_HH



class TCPTimer{
  private:
	unsigned int _retransmission_timeout{};
	unsigned int _start_time{}, _cur_time{};
	unsigned int _consecutive_number{};
	bool _is_run{};
  public:
	TCPTimer(const unsigned int timeout);
	void run_timer(const unsigned int start_time);
	void stop_timer();
	void double_timeout();
	void reset_timeout(const unsigned int timeout);
	bool is_expired(const unsigned int ms_since_last_call) ;
	bool is_running() const;
	unsigned int consecutive_number() const;
};

#endif
