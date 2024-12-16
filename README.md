# Optional
This repository contains a small, lightweight implementation of C++17's std::optional.
For a given type T, the class Optional contains a fixed size internal buffer that
maintains the alignment of T. This buffer stores the optional value in-place, and the
type's destructor is called only when the object is in an "active" state.

# Why did I write this?
For fun, I guess. I was curious about how one might go about implementing
C++17's std::optional. 
