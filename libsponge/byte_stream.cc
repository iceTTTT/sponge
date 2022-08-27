#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;
ByteStream::ByteStream(const size_t capacity):buffer(),_capacity(capacity) {}

size_t ByteStream::write(string data) 
{
    size_t written=remaining_capacity();
    if(data.size()<=written)
      written=data.size();
    writecount+=written;
    data.erase(data.begin()+written,data.end());
    buffer.append(move(data));
    return written;
}
//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const 
{
    size_t n=buffer_size();
    if(len<=n)
       n=len;
    string ret("");
     deque<Buffer>::const_iterator i= buffer._buffers.begin();
     while(n>0){
        if(n<i->str().size())
        {
            ret+=string_view(i++->str().begin(),n );
            n=0;
        }
        else
        {
            n-=i->str().size();
            ret+=i++->str();
        }
     }
     return ret;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) 
{ 
    size_t n=buffer_size();
    if(len<n)
      n=len;
    readcount+=n;
    buffer.remove_prefix(n);
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    size_t n=buffer_size();
    if(len<n)
      n=len;
    readcount+=n;
    string ret("");
    deque<Buffer>::iterator i= buffer._buffers.begin();
    while(n>0){
        if(n<i->str().size())
        {
            ret+=string_view(i->str().begin(),n );
            i++->remove_prefix(n);
            n=0;
        }
        else
        {
            n-=i->str().size();
            ret+=i->str();
            buffer._buffers.erase(i++);
        }
     }
    return ret;
}

void ByteStream::end_input() {isend=true;}

bool ByteStream::input_ended() const { return isend; }

size_t ByteStream::buffer_size() const { return writecount-readcount;}

bool ByteStream::buffer_empty() const { return writecount==readcount; }

bool ByteStream::eof() const { return writecount==readcount &&isend; }

size_t ByteStream::bytes_written() const { return writecount; }

size_t ByteStream::bytes_read() const { return readcount; }

size_t ByteStream::remaining_capacity() const { return _capacity-buffer_size(); }
