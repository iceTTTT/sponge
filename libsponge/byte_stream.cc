#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;
ByteStream::ByteStream(const size_t capacity):buffer(new char[capacity+1]),capacit(capacity) {}

size_t ByteStream::write(const string &data) 
{
    size_t written=remaining_capacity();
    if(data.size()<=written)
      written=data.size();
    writecount+=written;
    for(size_t i=0;i<written;i++)
    {
        buffer[rear++]=data[i];
        rear=rear%(capacit+1);
    } 
    return written;
}
void ByteStream::writechar(const char& c)
{
   if(remaining_capacity())
      buffer[rear++]=c;
   rear=rear%(capacit+1);
}
//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const 
{
    size_t temp=buffer_size();
    size_t thead=head;
    if(len<=temp)
       temp=len;
    string ret(temp,'0');
    for(size_t i=0;i<temp;i++)
    {
      ret[i]=buffer[thead++];
      thead=thead%(capacit+1);
    }
    return ret;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) 
{ 
    size_t temp=buffer_size();
    if(len<temp)
      temp=len;
    readcount+=temp;
    head=(head+temp)%(capacit+1);
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string ret=peek_output(len);
    pop_output(len);
    return ret;
}

void ByteStream::end_input() {isend=true;}

bool ByteStream::input_ended() const { return isend; }

size_t ByteStream::buffer_size() const 
{ if(rear==head) 
     return 0;
  if(rear>head)
     return rear-head;
 return (capacit+1)-head+rear;
}

bool ByteStream::buffer_empty() const { return head==rear; }

bool ByteStream::eof() const { return head==rear&&isend; }

size_t ByteStream::bytes_written() const { return writecount; }

size_t ByteStream::bytes_read() const { return readcount; }

size_t ByteStream::remaining_capacity() const { return capacit-buffer_size(); }
