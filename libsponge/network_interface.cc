#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>
#include <algorithm>
// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();

    if(iptoep.find(next_hop_ip) == iptoep.end())
    {
      //unkown ep address
      //if not been broadcast in last 5s :broadcast.
      if(requesttimeout.find(next_hop_ip) == requesttimeout.end())
      {
      requesttimeout[next_hop_ip]=0;

      EthernetFrame out;
      out.header().type=EthernetHeader::TYPE_ARP;
      out.header().src=_ethernet_address;
      out.header().dst=ETHERNET_BROADCAST;

      ARPMessage arp;
      arp.opcode=ARPMessage::OPCODE_REQUEST;
      arp.sender_ethernet_address=_ethernet_address;
      arp.sender_ip_address= _ip_address.ipv4_numeric();
      arp.target_ip_address=next_hop_ip;

      out.payload()=arp.serialize();
      _frames_out.push(out);
      }
      //queue the datagram till get the ARP reply;
      queue.insert(make_pair(next_hop_ip,dgram));

    }
    else
    { //ep address is already known
      EthernetFrame out;
      out.header().type=EthernetHeader::TYPE_IPv4;
      out.header().src=_ethernet_address;
      out.header().dst=iptoep.at(next_hop_ip);
      out.payload()=dgram.serialize();
      _frames_out.push(out);
    }

}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    if(frame.header().dst != ETHERNET_BROADCAST &&  frame.header().dst!=_ethernet_address)
         return {};
    if(frame.header().type==EthernetHeader::TYPE_ARP)
    {
      ARPMessage arp;
      if(arp.parse(frame.payload()) == ParseResult::NoError)
       {
           iptoep[arp.sender_ip_address]=arp.sender_ethernet_address;
           maptimeout[arp.sender_ip_address]=0;
           
           //check if any queueing datagram's target is this. send the frame.
           while(queue.find(arp.sender_ip_address) != queue.end())
           {    
                auto i=queue.find(arp.sender_ip_address);

                EthernetFrame out;
                out.header().type=EthernetHeader::TYPE_IPv4;
                out.header().src=_ethernet_address;
                out.header().dst= arp.sender_ethernet_address;
                out.payload()=i->second.serialize();
                _frames_out.push(out);

                queue.erase(i);
           }

          //request for this ipep.broadcast
          if(arp.opcode == ARPMessage::OPCODE_REQUEST && arp.target_ip_address == _ip_address.ipv4_numeric())
          {
              EthernetFrame out;
              out.header().type=EthernetHeader::TYPE_ARP;
              out.header().src=_ethernet_address;
              out.header().dst=arp.sender_ethernet_address;

              ARPMessage rearp;
              rearp.opcode=ARPMessage::OPCODE_REPLY;
              rearp.sender_ethernet_address=_ethernet_address;
              rearp.sender_ip_address= _ip_address.ipv4_numeric();

              rearp.target_ip_address= arp.sender_ip_address;
              rearp.target_ethernet_address=arp.sender_ethernet_address;
              

              out.payload()=rearp.serialize();
              _frames_out.push(out);
          }
       }
    }
    else
    {
       InternetDatagram ret;
       if(ret.parse(frame.payload()) == ParseResult::NoError)
           return ret;
    }

    return {};
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) 
{ 
  //request timeout 5s. 
  for(auto i=requesttimeout.begin();i!=requesttimeout.end();)
  {   
      i->second+=ms_since_last_tick;
      if(i->second>=5000)
        i=requesttimeout.erase(i);
      else
        i++;
  }

  //memory maptime 30s.
  for(auto i=maptimeout.begin();i!=maptimeout.end();)
  {
     i->second+=ms_since_last_tick;
     if(i->second >= 30000)
     {
       iptoep.erase(i->first);
       i=maptimeout.erase(i);
     }
     else
       i++; 
  }
}
