#include "tcp_sender.hh"
#include "tcp_config.hh"

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
    , _stream(capacity)
    , mytimer(retx_timeout) {}

uint64_t TCPSender::bytes_in_flight() const { return bif; }

void TCPSender::fill_window() 
{ 
   if(!next_seqno_absolute())
   {
       TCPSegment out;
       out.header().syn=true;
       out.header().seqno=next_seqno();
       _segments_out.push(out);

       bif+=out.length_in_sequence_space();
       _next_seqno+=out.length_in_sequence_space();

       track.insert(tracker(0,out));
       mytimer.start();
       return;
   }
     if(_stream.buffer_empty()) 
  { if(_stream.eof() && !finsent && wsize>0)
      {
      TCPSegment out;
      out.header().seqno=next_seqno();
      out.header().fin=true;
      _segments_out.push(out);
      track.insert(tracker(_next_seqno,out));
      bif+=out.length_in_sequence_space();
      _next_seqno+=out.length_in_sequence_space();
      mytimer.start();
      finsent=true;
      wsize=-1;
      }
    if(wsize || !_stream.eof())
     return;
  }
   if(wsize && wsize!=-1)
{
    while(wsize >= long(TCPConfig::MAX_PAYLOAD_SIZE))
    {
      TCPSegment out;
      out.payload()=Buffer(move(_stream.read(TCPConfig::MAX_PAYLOAD_SIZE)));
      out.header().seqno=next_seqno();
      if(_stream.eof() && !finsent && out.length_in_sequence_space() < uint64_t(wsize))
        {out.header().fin=true;finsent=true;}
      _segments_out.push(out);
      track.insert(tracker(_next_seqno,out));
      bif+=out.length_in_sequence_space();
      _next_seqno+=out.length_in_sequence_space();

      wsize-=out.length_in_sequence_space();
      if(_stream.buffer_empty())
       {
         if(!wsize)
         wsize=-1;
        mytimer.start();
        return;
       }
    }
      if(wsize)
    {
      TCPSegment out;
      out.payload()=Buffer(move(_stream.read(wsize)));
      out.header().seqno=next_seqno();
      if(_stream.eof() && !finsent && out.length_in_sequence_space() < uint64_t(wsize))
        {out.header().fin=true;finsent=true;}
      _segments_out.push(out);
      track.insert(tracker(_next_seqno,out));
      bif+=out.length_in_sequence_space();
      _next_seqno+=out.length_in_sequence_space();
      wsize-=out.length_in_sequence_space();
      if(!wsize)
        wsize=-1;
    }
     else
      wsize=-1;
    mytimer.start();
}
 if(!wsize && firstsent)
 { 
   TCPSegment out;
   out.payload()=Buffer(move(_stream.read(1)));
   out.header().seqno=next_seqno();
   if(!out.length_in_sequence_space() && _stream.eof() && !finsent)
      {out.header().fin=true;finsent=true;}
   _segments_out.push(out);
   track.insert(tracker(_next_seqno,out));
   bif+=out.length_in_sequence_space();
   _next_seqno+=out.length_in_sequence_space();
   mytimer.start();
   firstsent=false;
 }
   
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) 
{  uint64_t absackno=unwrap(ackno,_isn,_next_seqno);
   //impossible case
    if(absackno > _next_seqno)
      return;
    if(absackno<lastack)
      return;
    
    bif-=(absackno-lastack);
    

    //wsize=window_size;
    if(absackno+window_size == _next_seqno  && window_size)
       wsize=-1;
    else 
      wsize=window_size-(_next_seqno - absackno);
    
   //if acked new bytes  -->next
   if(absackno>lastack)
   { mytimer.current_rto=_initial_retransmission_timeout;
     mytimer.stop();
   //  next--> and if outstanding remains
    if(!track.empty())
     for(set<tracker>::reverse_iterator i=track.rbegin();i!=track.rend();i++)
           if(   absackno> ( i->absseqno + i->tcp.length_in_sequence_space() - 1 )  )
              {track.erase(track.begin(),i.base()); break;}
    if(!track.empty())
       mytimer.start(); 
    firstsent=true;
   }
   lastack=absackno;
   fill_window();
}
//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) 
{  if(mytimer.run)
    {   
        mytimer.accu_tick+=ms_since_last_tick; 
        if(mytimer.expire())
       {   
         if(mytimer.rt_times>TCPConfig::MAX_RETX_ATTEMPTS)
             return ;
           //! retransmit
           _segments_out.push(track.begin()->tcp);
           //if wsize if nonzero    {double rto & increment rt times}
           if(wsize)
          {
            mytimer.rt_times++;
            mytimer.current_rto*=2;
          }
            //restart 
            mytimer.accu_tick=0;
        }
    }


}

unsigned int TCPSender::consecutive_retransmissions() const { return mytimer.rt_times; }

void TCPSender::send_empty_segment() 
{
      TCPSegment out;
      out.header().seqno=wrap(lastack? lastack-1 : lastack,_isn);
      out.header().ack=true;
      _segments_out.push(out);
}
