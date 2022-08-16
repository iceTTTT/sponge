#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : auxiliary(),_output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) 
{   size_t datasize=data.size();
    size_t indata;
    //put in stream
    if(index<=nextneeded && index+datasize-1>=nextneeded)
    {      
           indata=nextneeded-index;      
           for(;indata<datasize;indata++)
           {
                if(isfull())
                {
                 if(!_output.remaining_capacity())
                    break;
                 if(nextneeded==cacheleast)
                  {
                      auxiliary.pop_front();  
                      cacheleast++; 
                  }
                  else  
                   auxiliary.pop_back();
                }
                _output.writechar(data[indata]);
                nextneeded++; 
           } 
        if(_output.remaining_capacity())
        {
            if(nextneeded>=cacheleast)
            {   
                while(nextneeded>cacheleast && auxiliary.size())
                {
                     auxiliary.pop_front();
                     cacheleast++;
                }
               while(auxiliary.size())
                 {
                    _output.writechar(auxiliary.front());
                   auxiliary.pop_front();
                 }
            }
        }
        
    }
    else if(nextneeded<index && nextneeded+_capacity > index)  
          auxexpand(index,datasize,data);

    if(eof)
      _eof=true;
    if(empty())
     _output.end_input();
}
size_t StreamReassembler::unassembled_bytes() const { return auxiliary.size(); }

bool StreamReassembler::empty() const { return _eof && !auxiliary.size(); }

bool StreamReassembler::isfull(){    return (_output.buffer_size()+auxiliary.size() ==_capacity);  }

void StreamReassembler::auxexpand(const size_t index,const size_t datasize,const std::string& data)
{
  if(!cacheleast)
          {
              cacheleast=index;
          for(size_t i=0;index+i<nextneeded+_capacity && i<datasize && !isfull();i++)
                  auxiliary.push_back(data[i]);
          }
          else if(index<cacheleast && index+datasize-1>=cacheleast-1)
          {        size_t i;
                   for(i=cacheleast-1-index;!isfull();i--)
                      {auxiliary.push_front(data[i]);
                        if(!i)
                          break;
                      }
                   cacheleast=i+index+1;
                   if(!i)
                     cacheleast--;
                   for(i=cacheleast+auxiliary.size()-index;i<datasize&& !isfull();i++)
                       auxiliary.push_back(data[i]);
                         
          }
          else if(index>=cacheleast && index<=cacheleast+auxiliary.size() && index+datasize-1>=cacheleast+auxiliary.size())
          {

              for(size_t i=cacheleast+auxiliary.size()-index;i<datasize && !isfull();i++)
                    auxiliary.push_back(data[i]);
          }
}
