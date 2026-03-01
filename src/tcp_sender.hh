#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include <deque>
#include <functional>

class TCPSender
{
public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( ByteStream&& input, Wrap32 isn, uint64_t initial_RTO_ms )
    : input_( std::move( input ) ),
    isn_( isn ), 
    initial_RTO_ms_( initial_RTO_ms ),
    next_seqno_absolute_ (0) , 
    highest_ack_absolute_(0),
    receiver_window_size_(1),
    outstanding_(),
    bytes_in_flight_(0),
    rto_(initial_RTO_ms),
    timer_elapsed_(0),
    timer_running_(false),
    consecutive_retransmissions_(0),
    syn_sent_(false),
    fin_sent_(false)    
  {}

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage make_empty_message() const;

  /* Receive and process a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Type of the `transmit` function that the push and tick methods can use to send messages */
  using TransmitFunction = std::function<void( const TCPSenderMessage& )>;

  /* Push bytes from the outbound stream */
  void push( const TransmitFunction& transmit );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called */
  void tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit );

  // Accessors
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive retransmissions have happened?
  const Writer& writer() const { return input_.writer(); }
  const Reader& reader() const { return input_.reader(); }
  Writer& writer() { return input_.writer(); }

private:
  Reader& reader() { return input_.reader(); }

  ByteStream input_;
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;

  uint64_t next_seqno_absolute_ ; 
  uint64_t highest_ack_absolute_ ; 

  uint16_t receiver_window_size_ ; 

  std::deque <TCPSenderMessage> outstanding_ ; 
  uint64_t bytes_in_flight_ ; 
  
  
  uint64_t rto_ ; 

  uint64_t timer_elapsed_ ; 
  bool timer_running_ ; 

  uint64_t consecutive_retransmissions_ ; 

  bool syn_sent_ ; 
  bool fin_sent_ ; 
};
