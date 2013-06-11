Multicore Processor Model
=================================================
Designed and implmented a multicore processor in C, including a pipeline, branch predictor, and caches.
Implemented cache LRU, register renaming, and dynamic scheduling algorithms. Improved the effectiveness of parallelism for instruction level by out of order excution(dynamic scheduling).
## _About_
* Computer Architecture, Fall 2012, University of Pennslyvania.

## _Contribution_
1. ISA: measured the performance impact of an ISA change (small branch offsets) and a micro-architectural technique (macro-op fusion) from instruction trace generated from x86 "functional" simulator.
2. Pipeline: simulated different load latencies and its impact on CPI in the 5 stage pipeline model.
3. Simulated the impact of branch direction prediction to program using binomial, Gshare and Tournament predictor.
4. Caches: simulated the impact of different caches configurations(size, write-back and write-through) to performance(miss rate). Implemented the LRU algorithm for set-associate caches.
5. Susperscalar: simulated the effectiveness of superscalar execution, including pipeline, branch predictor and cache.
6. Multicore: explored out-of-order execution (dynamic scheduling) and its effectiveness in extracting instruction-level-parallelism. 
Implmented register renaming algorithm and dynamic scheduling alogrithm.
