#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return currenttime-lastrectime; }

void TCPConnection::segment_received(const TCPSegment &seg) 
{   
    lastrectime=currenttime;
    if(seg.header().rst)
  {  
    if(!_receiver.ackno().has_value()  &&  !_sender.next_seqno_absolute())
      return;
     _sender.stream_in().set_error();
     _receiver.stream_out().set_error();
     _linger_after_streams_finish=false;
  }
  //reive seg from remote tcp
  //size_t inlastack= _receiver.getasmbler().getabsackno();
  _receiver.segment_received(seg);
  //size_t inthisack=_receiver.getasmbler().getabsackno();
  if( _receiver.getasmbler().empty()  &&  !_sender.finsent )
    _linger_after_streams_finish=false;


  //sender react to remote ack info
    if(seg.header().ack)
        _sender.ack_received(seg.header().ackno,seg.header().win);
    else
      _sender.fill_window();
    

    if( _receiver.ackno().has_value() &&   _sender.segments_out().empty() && seg.length_in_sequence_space() )
    { 
      TCPSegment out;
      out.header().seqno=_sender.next_seqno();
      out.header().ack=true;
      out.header().ackno=_receiver.ackno().value();
      out.header().win=_receiver.window_size()> 0xffff ? 0xffff : _receiver.window_size();
      _segments_out.push(out);
    }
    else
      output();
   

    //keep alive
    if(_receiver.ackno().has_value() && !seg.length_in_sequence_space() && seg.header().seqno == _receiver.ackno().value()-1 )
     {
       if(active() && _receiver.ackno().has_value())
      {TCPSegment out;
      out.header().seqno=_sender.next_seqno();
      out.header().ack=true;
      out.header().ackno=_receiver.ackno().value();
      out.header().win=_receiver.window_size()> 0xffff ? 0xffff : _receiver.window_size();
      _segments_out.push(out);
      }

     }

}

//check if the connection is alive.
bool TCPConnection::active() const 
{ if(_sender.stream_in().error() && _receiver.stream_out().error()) 
     return false;
  if(_receiver.getasmbler().empty() &&  !_sender.bytes_in_flight() &&  _sender.finsent  && !_linger_after_streams_finish )
     return false; 
  return true;
}


//write to sender stream.
size_t TCPConnection::write(const string &data) 
{    size_t i=_sender.stream_in().write(data);   
    _sender.fill_window();
    output();
    return i;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) 
{  currenttime+=ms_since_last_tick;
   if(_receiver.getasmbler().empty() &&  !_sender.bytes_in_flight() &&  _sender.finsent && !_sender.wsize )
      _sender.wsize=-1;
     
   if(_receiver.getasmbler().empty() &&  !_sender.bytes_in_flight() &&  _sender.finsent  && _linger_after_streams_finish  && time_since_last_segment_received() >= 10*_cfg.rt_timeout)
        _linger_after_streams_finish=false;
   _sender.tick(ms_since_last_tick); 
    if(_sender.consecutive_retransmissions() > _cfg.MAX_RETX_ATTEMPTS)
    {
         _sender.stream_in().set_error();
         _receiver.stream_out().set_error();
         _linger_after_streams_finish=false;
         TCPSegment out;
         out.header().seqno=_sender.next_seqno();
         out.header().rst=true;
         _segments_out.push(out);
    }
    else
      output();
}


//end sender stream. no more bytes.
void TCPConnection::end_input_stream() 
{
    _sender.stream_in().end_input();
    _sender.fill_window();
    output();
}


//client send syn
void TCPConnection::connect() 
{ 
  _sender.fill_window();
  _segments_out.push(_sender.segments_out().front());
  _sender.segments_out().pop();
}


//pop sender queue to the connection queue. for output
void TCPConnection::output()
{
  
    if(_receiver.ackno().has_value())  
    while(!_sender.segments_out().empty())
      {
            _sender.segments_out().front().header().ack=true;
            _sender.segments_out().front().header().ackno=_receiver.ackno().value();
            _sender.segments_out().front().header().win=_receiver.window_size()> 0xffff ? 0xffff : _receiver.window_size();
            _segments_out.push(_sender.segments_out().front());
            _sender.segments_out().pop();
      }
    else  
    while(!_sender.segments_out().empty())
        {  
              _segments_out.push(_sender.segments_out().front());
              _sender.segments_out().pop();
        }   

    
}
TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
            TCPSegment out;
            out.header().seqno=_sender.next_seqno();
            out.header().rst=true;
            _segments_out.push(out);
            // Your code here: need to send a RST segment to the peer
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
