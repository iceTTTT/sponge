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

uint64_t TCPSender::bytes_in_flight() const { return {}; }

void TCPSender::fill_window() 
{ 
   //after sent nonempty segment
  

  if(!mytimer.run)
    mytimer.start();

  

}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) 
{  
   //if acked new bytes  -->next
    mytimer.current_rto=_initial_retransmission_timeout;
    mytimer.stop();

   //  next--> and if outstanding remains
    mytimer.start();
    
    //if remain space fill window
   
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) 
{  if(mytimer.run)
    {   
        mytimer.accu_tick+=ms_since_last_tick; 
        if(mytimer.expire())
       {   
           
           //! retransmit.
           
           

           //if wsize if nonzero    {double rto & increment rt times}
            mytimer.rt_times++;
            mytimer.current_rto*=2;


            
            //restart 
            mytimer.accu_tick=0;
        }
    }


}

unsigned int TCPSender::consecutive_retransmissions() const { return mytimer.rt_times; }

void TCPSender::send_empty_segment() {}
