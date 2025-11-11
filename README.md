# Implementation of SLAP - simple, efficient lattice-based PSA.
Polynomial.h has the polynomial primitive operations, along with some RNS base conversion and scaling routines.

DiscreteLaplacian.h implements noise generation for differential privacy.

Aggregator\_NTL.h and Aggregator\_RNS implement SLAP with NTL (basic, slower version) and Polynomial.h (fast full-RNS version). aggregation.cpp is the driver program.

Other files (possibly not included with anonymous submission) include implementation of the complex canonical embedding for multiplicative aggregation, job scripts for testing on research machines, and other utilities.
