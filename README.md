# CacheHashTable

Implementation of different cache data structures. In particular, Finite Hash Table (FHT) is the main component of a paper I co-authored ([not available yet]()). The purpose of this program is to evaluate the performance of different cache data structures. Here are the implemented data structures:

| Name | Codename | Description
| - | - | - |
| Finite Hash Table | ```fht``` | LRU approximation from ([not available yet]())
| LRU (standard map) | ```std``` | LRU implemented with ```std::unordered_map```
| LRU (emhash map) | ```emh``` | LRU implemented with ```emhash7``` from ([source code](https://github.com/ktprime/emhash))

### Compilation

```g++ -std=c++20 CacheHashTable/CacheHashTable/*.cpp -o ht -O3```

### Execution

| Parameter | Type | Description |
| - | - | - |
| ```<path>``` | string | path the dataset folder (must end with "/") |
| ```<dataset>``` | string | filename of the dataset (must be inside ```<path>```) |
| ```<hash>``` | string | codename of the hash function |
| ```<log2_slot>``` | int | base-2 logarithm of the number of slots (see ([not available yet]())) |
| ```<length>``` | int | number of bytes per slot (see ([not available yet]())) |
| ```<capacity>``` | int | number of entries in the cache |

Execution for the FHT data structure:

 ```./ht <path> <dataset> <hash> fht       <log2_slots> <length>```

Execution for the LRU data structures:
 
 ```./ht <path> <dataset> <hash> (std|emh) <capacity>```

### Dataset

The dataset must be a text file consisting of one string per line.

# CacheCounter

Specialisation of the FHT data structure for the frequency estimation problem, called FHTC. It relies on Morris Counters ([original paper](https://dl.acm.org/doi/10.1145/359619.359627)).

### Compilation

```g++ -std=c++20 CacheHashTable/CacheCounter/*.cpp -o hc -O3```

### Execution

| Parameter | Type | Description |
| - | - | - |
| ```perf``` | string | run in _performance_ mode |
| ```track``` | string | run in _tracking_ mode |
| ```fht{n}``` | string | Set the base of the Morris Counters to $2^{1/n}$ |

```./hc (perf|track) <path> <dataset> <hash> (fht1|fht2|fht3) <log2_slots> <length>```

```./hc (perf|track) <path> <dataset> <hash> (std|emh)  <capacity>```

The _performance_ mode outputs some statistics about the performamce of FHTC. Here is the format of the output:

```filename,model,hash function,model parameter,latency (ns),hitrate (%),number of pairs,content size (Bytes),overhead size (Bytes)```

The _tracking_ mode displays periodically the counter values of the 256 most frequent items, each line of the output is of the following format:

```model,hash function,model parameter,key1,counter1,...,key256,counter256```

# dataset-analysis

This is a simple program which goal is to estimate the reading time of a dataset

### Compilation

```g++ -std=c++20 CacheHashTable/dataset-analysis/*.cpp -o da -O3```

### Execution

```./da <path> <dataset>```
