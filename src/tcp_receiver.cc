#include "tcp_receiver.hh"
#include "debug.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{

  if(message.RST){
    reassembler_.writer().set_error();
    return ; 
  }
  if(!isn_){
    if(!message.SYN){
      return ; 
    }
    isn_ = message.seqno ; 
  }
  uint64_t checkpoint = reassembler_.writer().bytes_pushed() + 1 ;
  uint64_t abs_seqno = message.seqno.unwrap( isn_.value() , checkpoint );
  uint64_t stream_index ; 
  if(message.SYN){
    stream_index = 0 ; 
  }
  else{
    stream_index = abs_seqno - 1; 
  }
  reassembler_.insert(stream_index , message.payload , message.FIN);
  // Your code here.
  //debug( "unimplemented receive() called" );
  (void)message;
}

/*
TCPReceiverMessage TCPReceiver::send() const
{
  TCPReceiverMessage msg ; 
  if(isn_){
    uint64_t ackno_abs = reassembler_.writer().bytes_pushed() + 1 ; 
    if(reassembler_.writer().is_closed()){
      ackno_abs += 1 ; 
    }
    msg.ackno = Wrap32::wrap(ackno_abs , isn_.value());
  }
  if(!isn_){
    msg.window_size = reassembler_.writer().available_capacity() - 1 ; 
  }
  else{
    msg.window_size = reassembler_.writer().available_capacity();
  }
  msg.RST = reassembler_.writer().has_error();
  // Your code here.
  //debug( "unimplemented send() called" );
  return msg;
}
*/
TCPReceiverMessage TCPReceiver::send() const
{
  TCPReceiverMessage msg;

  if (isn_) {
    uint64_t ackno_abs = reassembler_.writer().bytes_pushed() + 1;

    if (reassembler_.writer().is_closed()) {
      ackno_abs += 1;
    }

    msg.ackno = Wrap32::wrap(ackno_abs, isn_.value());
  }

  uint64_t avail = reassembler_.writer().available_capacity();
  msg.window_size = static_cast<uint16_t>(
      min(avail, static_cast<uint64_t>(UINT16_MAX)));

  msg.RST = reassembler_.writer().has_error();

  return msg;
}