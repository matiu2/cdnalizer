#pragma once

#include <string>

namespace cdnalizer {
namespace apache {

/// This is an event that we need to handle
struct BucketEvent {
  /// Where to split the current bucket (nullptr if no splitting required)
  char *splitPoint = nullptr;
  /// New data to push down the line after the split
  std::string newData;
};
    
} /* apache */ 
} /* cdnalizer  */ 
