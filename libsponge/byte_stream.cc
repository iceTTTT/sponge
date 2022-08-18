#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;
ByteStream::ByteStream(const size_t capacity):buffer(),_capacity(capacity) {}

size_t ByteStream::write(const string &data) 
{
    size_t written=remaining_capacity();
    if(data.size()<=written)
      written=data.size();
    writecount+=written;
    buffer.insert(buffer.end(),data.begin(),data.begin()+written);
    return written;
}
void ByteStream::writechar(const char& c)
{
   buffer.push_back(c);
   writecount++;
}
//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const 
{
    size_t temp=buffer_size();
    if(len<=temp)
       temp=len;
    return string(buffer.begin(),buffer.begin()+temp);
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) 
{ 
    size_t temp=buffer_size();
    if(len<temp)
      temp=len;
    readcount+=temp;
    buffer.erase(buffer.begin(),buffer.begin()+temp);
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    size_t temp=buffer_size();
    if(len<temp)
      temp=len;
    readcount+=temp;
    string ret(buffer.begin(),buffer.begin()+temp);
    buffer.erase(buffer.begin(),buffer.begin()+temp);
    return ret;
}

void ByteStream::end_input() {isend=true;}

bool ByteStream::input_ended() const { return isend; }

size_t ByteStream::buffer_size() const { return buffer.size();}

bool ByteStream::buffer_empty() const { return buffer.empty(); }

bool ByteStream::eof() const { return buffer.empty()&&isend; }

size_t ByteStream::bytes_written() const { return writecount; }

size_t ByteStream::bytes_read() const { return readcount; }

size_t ByteStream::remaining_capacity() const { return _capacity-buffer.size(); }
