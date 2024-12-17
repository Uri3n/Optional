# Optional
This repository contains a small, lightweight implementation of C++17's std::optional.
For a given type T, the class Optional contains a fixed size internal buffer that
maintains the alignment of T. This buffer stores the optional value in-place, and the
type's destructor is called only when the object is in an "active" state.
"Nullopt" here is the equivalent of std::nullopt for the STL's optional. Attempting to
access the contained value while it is inactive will throw a std::runtime_error.

# Requirements
Any compiler that supports the C++23 standard correctly. I used "deducing this" as well as a few
other C++23 features here. I compiled this using GCC, but the code should be portable
across all major toolchains.

# Why did I write this?
For fun, I guess. I was curious about how one might go about implementing
C++17's std::optional. 
