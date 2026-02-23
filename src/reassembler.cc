#include "reassembler.hh"
#include "debug.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  if(is_last_substring){
    eof_received_ = true ; 
    eof_index_ = first_index + data.size();
  }
  if (eof_received_ && next_index_ == eof_index_) {
    output_.writer().close();
  }
  if(first_index + data.size() <= next_index_){
    return ; 
  }
  if(first_index < next_index_){
    data = data.substr(next_index_ - first_index);
    first_index = next_index_ ; 
  }  
  uint64_t window_end = next_index_ + writer().available_capacity();
  if(first_index >= window_end){
    return ; 
  }
  if(first_index + data.size() > window_end){
    data.resize(window_end - first_index);
  }
  //segments_[first_index] = data ; 
  //algorithm gpt teach

  uint64_t new_start = first_index;
uint64_t new_end = first_index + data.size();
std::string new_data = std::move(data);

// Find first segment that might overlap
auto it = segments_.lower_bound(new_start);

// Also check previous segment
if (it != segments_.begin()) {
    auto prev = std::prev(it);
    if (prev->first + prev->second.size() >= new_start) {
        it = prev;
    }
}

// Merge all overlapping segments
while (it != segments_.end() &&
       it->first < new_end) {

    uint64_t seg_start = it->first;
    uint64_t seg_end = seg_start + it->second.size();

    if (seg_end < new_start) {
        ++it;
        continue;
    }

    // Expand new interval
    uint64_t merged_start = std::min(new_start, seg_start);
    uint64_t merged_end = std::max(new_end, seg_end);

    std::string merged_data(merged_end - merged_start, '\0');

    // Copy old new_data
    std::copy(new_data.begin(), new_data.end(),
              merged_data.begin() + (new_start - merged_start));

    // Copy existing segment
    std::copy(it->second.begin(), it->second.end(),
              merged_data.begin() + (seg_start - merged_start));

    new_start = merged_start;
    new_end = merged_end;
    new_data = std::move(merged_data);

    it = segments_.erase(it);
}

// Insert merged segment
segments_[new_start] = std::move(new_data);

  while (true)
  {
    auto push_it = segments_.find(next_index_);
    if(push_it == segments_.end()){
      break;
    }
    output_.writer().push(push_it->second);
    next_index_ = next_index_ + push_it->second.size();
    segments_.erase(push_it);  
  }
  if(eof_received_ && next_index_ == eof_index_){
    output_.writer().close();
  }
  //debug( "unimplemented insert({}, {}, {}) called", first_index, data, is_last_substring );
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  uint64_t total = 0 ; 
  for(const auto& pend : segments_){
    total += pend.second.size();
  }
  //debug( "unimplemented count_bytes_pending() called" );
  return total;
}
