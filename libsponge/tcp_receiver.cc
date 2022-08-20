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
        _reassembler.increackno();
    }
    if(seg.header().fin)
    {   
        maxseq.changevalue( (seg.header().seqno).raw_value());
        maxlength=seg.length_in_sequence_space()-1;
        eof=true;
    } 
    if(init)
    {   
        _reassembler.push_substring(seg.payload().str(),unwrap(seg.header().seqno+(seg.header().syn? 1: 0),isn,_reassembler.getabsackno()-1),false);
        if(eof)
        _reassembler.push_substring("",unwrap(maxseq,isn,_reassembler.getabsackno()-1)+maxlength,eof);
    }
    if(_reassembler.empty())
      _reassembler.increackno();
}

optional<WrappingInt32> TCPReceiver::ackno() const 
{ 
  if(init) return wrap(_reassembler.getabsackno(),isn);
  else return {}; 
}

size_t TCPReceiver::window_size() const { return _capacity-_reassembler.stream_out().buffer_size(); }
