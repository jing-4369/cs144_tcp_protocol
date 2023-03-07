For build prereqs, see [the CS144 VM setup instructions](https://web.stanford.edu/class/cs144/vm_howto).

## benchmark performance

CPU-limited throughput : 3.39 Gbit/s

CPU-limited throughput with reordering: 1.59 Gbit/s

## circular queue

use `vector<char>` or `string` as buffer

CPU-limited throughput : 0.40 Gbit/s

CPU-limited throughput with reordering: 0.37 Gbit/s

## deque

use `deque<char>` as buffer

CPU-limited throughput : 2.00 Gbit/s

CPU-limited throughput with reordering: 1.30 Gbit/s

## BufferList

use `BufferList` as buffer

CPU-limited throughput : 3.39 Gbit/s

CPU-limited throughput with reordering: 1.59 Gbit/s
