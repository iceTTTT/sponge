#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {  return WrappingInt32(     isn.raw_value() + n- ( (n>>32) << 32 )   ) ;}
//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint)
{   
         int32_t dif= n-isn;
         uint64_t x;
         if(dif<0)
           x=0xffffffff+dif+1;
         else
           x=dif;
        uint64_t times=checkpoint>>32;
        x=(times<<32)+x;
        if(x<checkpoint)
        {
            if(checkpoint -x > x+(1ul<<32) -checkpoint)
               return x+(1ul<<32);
            else
               return x;
        }
        else if(x>checkpoint)
        {
             if(times==0)
               return x;
             if(x- checkpoint > checkpoint - (x -(1ul<<32)))
               return x-(1ul<<32);
             else
               return x;
        }
        return x;
}
