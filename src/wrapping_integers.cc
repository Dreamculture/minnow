#include "wrapping_integers.hh"
#include "debug.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  uint32_t relative = static_cast<uint32_t>(n) + zero_point.raw_value_;
  // Your code here.
  //debug( "unimplemented wrap( {}, {} ) called", n, zero_point.raw_value_ );
  return Wrap32 { relative };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint32_t offset = raw_value_ - zero_point.raw_value_ ; 
  uint64_t base = checkpoint & 0xffffffff00000000ULL;
  uint64_t candidate = base + offset ; 
  uint64_t diff = candidate > checkpoint ? candidate - checkpoint : checkpoint - candidate ; 
  if(diff > (1ULL << 31)){
    if(candidate < checkpoint)
      candidate += (1ULL << 32);
    else {
        if(candidate < (1ULL << 32)){
            return candidate ; 
        }
        candidate -= (1ULL << 32);      
    }

  }
  // Your code here.
  //debug( "unimplemented unwrap( {}, {} ) called", zero_point.raw_value_, checkpoint );
  return candidate;
}


/*
uint64_t Wrap32::unwrap(Wrap32 zero_point, uint64_t checkpoint) const
{
    uint64_t offset = static_cast<uint32_t>(raw_value_ - zero_point.raw_value_);

    // Start with the candidate in the same 2^32 block as checkpoint
    uint64_t candidate = (checkpoint & 0xffffffff00000000ULL) + offset;

    // Now adjust if necessary
    if (candidate > checkpoint) {
        if (candidate - checkpoint > (1ULL << 31))
            candidate -= (1ULL << 32);
    } else {
        if (checkpoint - candidate > (1ULL << 31))
            candidate += (1ULL << 32);
    }

    return candidate;
}
uint64_t Wrap32::unwrap(Wrap32 zero_point, uint64_t checkpoint) const
{
    // 1. Find the 32-bit offset from the ISN
    uint32_t offset = raw_value_ - zero_point.raw_value_;

    // 2. Find the candidate in the same 2^32 "epoch" as the checkpoint
    // We use (checkpoint & ~0xFFFFFFFFULL) to get the base of the current 2^32 block
    uint64_t candidate = (checkpoint & 0xFFFFFFFF00000000ULL) | offset;

    // 3. Check the neighbors (one block up, one block down) 
    // and pick the one with the smallest absolute difference to checkpoint.
    
    // Potential candidate one block higher
    uint64_t higher = candidate + (1ULL << 32);
    // Potential candidate one block lower (careful with underflow!)
    uint64_t lower = (candidate >= (1ULL << 32)) ? candidate - (1ULL << 32) : candidate;

    // Standard "find closest" logic
    uint64_t dist_mid = (candidate > checkpoint) ? (candidate - checkpoint) : (checkpoint - candidate);
    uint64_t dist_high = (higher > checkpoint) ? (higher - checkpoint) : (checkpoint - higher);
    uint64_t dist_low = (checkpoint > lower) ? (checkpoint - lower) : (lower - checkpoint);

    if (dist_low < dist_mid && dist_low < dist_high && candidate >= (1ULL << 32)) {
        return lower;
    } else if (dist_high < dist_mid && dist_high < dist_low) {
        return higher;
    }
    
    return candidate;
}*/