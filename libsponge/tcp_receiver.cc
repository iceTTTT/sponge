#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) 
{   if(seg.header().syn)
    {
        isn=seg.header().seqno;
        init=true;
    }
    if(init)
    {

    }
    else
     
    DUMMY_CODE(seg);
}

optional<WrappingInt32> TCPReceiver::ackno() const 
{ 
  if(init) return wrap(_reassembler.getabsackno(),isn);
  else return {}; 
}

size_t TCPReceiver::window_size() const { return _capacity-_reassembler.stream_out().buffer_size(); }