# Parity Bitmap Sketch

## Usage
```c++
#include "PBS.hpp"
std::vector<uint> host = {1,2,3}, remote = {2,3,4};
pbs = PBS(host, remote);
pbs.reconcile(sub_num, logn, t, seed, split_num);
```
See demo for details.
## Demo
You will need 'keypool.txt' and 'seedpool.txt' to run this demo. Both files should contain enough distinct uint32 items, one in each line. Keys are the items to reconcile. Seeds are for randomization.

### Build
1. mkdir build && cd build
2. cmake .. && make

### Run
- ./demo $n_test $n_host $n_remote $n_intersect $delta $logn $t $split_num
- Parameters:
    - $n_test: number of tests
    - $n_host: number of host's items 
    - $n_remote: number of remote's items
    - $n_intersect: number of items both in host and remote
    - $delta: 1/detla = average disticnt items in each group
    - $logn, $t: BCH code parameters, use BCH(n,t) code
    - $split_num: number of partitions when BCH code failed
- Example:
./demo 10 100000 100000 99900 0.2 10 13 3

## Reference
1. Gong, Long, et al. "Space-and Computationally-Efficient Set Reconciliation via Parity Bitmap Sketch (PBS)."
2. minisketch: https://github.com/sipa/minisketch
3. xxhash: https://github.com/Cyan4973/xxHash