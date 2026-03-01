#include "tcp_sender.hh"
#include "debug.hh"
#include "tcp_config.hh"

using namespace std;

// How many sequence numbers are outstanding?
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  //debug( "unimplemented sequence_numbers_in_flight() called" );
  return bytes_in_flight_;
}

// How many consecutive retransmissions have happened?
uint64_t TCPSender::consecutive_retransmissions() const
{
  //debug( "unimplemented consecutive_retransmissions() called" );
  return consecutive_retransmissions_;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  if (writer().has_error()) {
    TCPSenderMessage msg;
    msg.RST = true;
    msg.seqno = Wrap32::wrap(next_seqno_absolute_, isn_);
    transmit(msg);
    return;
}

  uint64_t window = receiver_window_size_ ; 
  if(window == 0 ){
    window = 1 ; 
  }
  while (bytes_in_flight_ < window)
  {
    TCPSenderMessage msg ; 
    msg.seqno = Wrap32::wrap(next_seqno_absolute_ , isn_);

    if(!syn_sent_){
      msg.SYN = true ; 
      //syn_sent_ = true ; 
    }



    uint64_t remaining = window - bytes_in_flight_ ; 

    if(msg.SYN){
      if(remaining == 0 ){
        break;
      }
      remaining -= 1 ; 
    }

    size_t payload_size = std::min({remaining , static_cast<uint64_t>(TCPConfig::MAX_PAYLOAD_SIZE ), reader().bytes_buffered()});

    msg.payload = reader().peek().substr(0 , payload_size);

    //if(reader().is_finished() && !fin_sent_ && remaining > payload_size ){
    if(writer().is_closed() && !fin_sent_ && remaining > payload_size && reader().bytes_buffered() == payload_size){
      msg.FIN = true ; 
      fin_sent_ = true ; 
    }

    if(msg.sequence_length() == 0){
      break; 
    }

    next_seqno_absolute_ += msg.sequence_length();
    bytes_in_flight_ += msg.sequence_length();

    reader().pop(msg.payload.size());

    outstanding_.push_back(msg);

    if(msg.SYN) 
      syn_sent_ = true ;     

    if(!timer_running_){
      timer_running_ = true ; 
      timer_elapsed_ = 0 ; 
    }
    
    transmit(msg);
  }

  //debug( "unimplemented push() called" );
  //(void)transmit;
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  TCPSenderMessage msg;
  msg.seqno = Wrap32::wrap(next_seqno_absolute_ , isn_);  
  msg.RST = writer().has_error();
  //debug( "unimplemented make_empty_message() called" );
  return msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  receiver_window_size_ = msg.window_size ; 

  if(msg.RST){
    writer().set_error();
    outstanding_.clear();
    bytes_in_flight_ = 0 ; 
    timer_running_ = false ; 
    return ; 
  }

  if(!msg.ackno.has_value())
    return ;
  
  uint64_t ack_abs = msg.ackno->unwrap(isn_ , next_seqno_absolute_);

  if(ack_abs > next_seqno_absolute_)
    return ; 
  
  bool ack_newdata = false ; 
  while (!outstanding_.empty())
  {
    const TCPSenderMessage& seg = outstanding_.front();
    uint64_t seg_start = seg.seqno.unwrap(isn_ , next_seqno_absolute_);
    uint64_t seg_end = seg_start + seg.sequence_length();

    if(seg_end <= ack_abs){
      bytes_in_flight_ -= seg.sequence_length();
      outstanding_.pop_front();
      ack_newdata = true ; 
    }
    else{
      break;
    }
  }
  
  if(ack_newdata){    
    consecutive_retransmissions_ = 0;
    rto_ = initial_RTO_ms_;
    timer_elapsed_ = 0;
    /*
    timer_running_ = true ; 
    timer_elapsed_ = 0 ; 
    */
  }

    if(ack_abs > highest_ack_absolute_){
      highest_ack_absolute_ = ack_abs ; 
      consecutive_retransmissions_ = 0 ; 
    }
  //debug( "unimplemented receive() called" );
  //(void)msg;
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  if(!timer_running_ || outstanding_.empty())
    return ; 
  timer_elapsed_ += ms_since_last_tick ; 
  if(timer_elapsed_ >= rto_){
    const TCPSenderMessage & seg = outstanding_.front();
    transmit(seg);

    if(receiver_window_size_ > 0){
      rto_ *= 2 ;
      consecutive_retransmissions_ += 1 ;   
    }     
    timer_elapsed_ = 0 ; 
  }
/*
  if(outstanding_.empty()){
    timer_running_ = false ; 
    timer_elapsed_ = 0 ; 
  }*/
  //debug( "unimplemented tick({}, ...) called", ms_since_last_tick );
  //(void)transmit;
}
