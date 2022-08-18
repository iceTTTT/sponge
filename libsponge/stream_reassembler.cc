#include "stream_reassembler.hh"
// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : aux(),_output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) 
{   size_t datasize=data.size();
    size_t indata;
    set<pair,compare>::iterator p,hint;
    //put in stream
    if(datasize+index-1>maxaccepted && datasize)
        maxaccepted=datasize+index-1;

    if(index<=nextneeded && index+datasize-1>=nextneeded)
    {      
           indata=nextneeded-index;  
           size_t auxsize=aux.size();
           size_t bufsize=_output.buffer_size();    
           for(;indata<datasize;indata++)
           {
                if(auxsize+bufsize==_capacity)
                {
                 if(bufsize==_capacity)
                    break;
                 if(nextneeded==aux.begin()->index)
                     {aux.erase(aux.begin());auxsize--;}
                 else
                   {   
                    p=aux.end();
                    aux.erase(--p);
                    auxsize--;
                   }
                }
                _output.writechar(data[indata]);
                bufsize++;
                nextneeded++; 
           } 
          while(!aux.empty() && nextneeded>aux.begin()->index )
                aux.erase(aux.begin());
          while(!aux.empty() && nextneeded==aux.begin()->index )
            {
                nextneeded++;
                _output.writechar(aux.begin()->value);
                aux.erase(aux.begin());
            }     
    }
    else if(nextneeded<index && nextneeded+_capacity > index)  
    {     
          size_t bufsize=_output.buffer_size();
          size_t h=0;
         for(size_t i=0;i<datasize && i+index<=nextneeded+_capacity;i++)
      {    
          if(bufsize+aux.size() ==_capacity)
          {  
             if(i+index>=aux.rbegin()->index)
                break;
             else
              {
                p=aux.end();
                aux.erase(--p);
              }
          }
          if(h)
            hint=aux.insert(hint,pair(i+index,data[i]));
          else
          {
            hint=aux.insert(pair(i+index,data[i])).first;
            h++;
          }
          
      }      
    }

    if(eof)
      _eof=true;
    if(empty())
     _output.end_input();
}
size_t StreamReassembler::unassembled_bytes() const { return aux.size(); }

bool StreamReassembler::empty() const { return _eof && aux.empty() && (maxaccepted== nextneeded-1 || (nextneeded==0 && maxaccepted==0)); }