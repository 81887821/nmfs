#ifndef SLICE_H
#define SLICE_H

#include <string>

namespace nmfs {

class slice {

	// Create an empty slice.
  	slice() : data(""), size(0) {}

  	// Create a slice that refers to d[0,n-1].
  	slice(const char* d, size_t n) : data(d), size(n) {}

  	// Create a slice that refers to the contents of "s"
  	slice(const std::string& s) : data(s.data()), size(s.size()) {}

  	// Create a slice that refers to s[0,strlen(s)-1]
  	slice(const char* s) : data_(s), size(strlen(s)) {}
	
	private:
		const char* data;
		size_t len;
}; // class slice 

} // namespace nmfs 

#endif 
