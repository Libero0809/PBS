#include <math.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "PBS.hpp"

int main(int argc, char** argv) {
    int ntest = atoi(argv[1]);           // number of tests
    int host_size = atoi(argv[2]);       // Host's size
    int remote_size = atoi(argv[3]);     // Remote's size
    int intersect_size = atoi(argv[4]);  // Intersection's size
    assert(host_size >= intersect_size && remote_size >= intersect_size);
    float delta = atof(argv[5]);         // group number/$diffsize
    uint logn = atoi(argv[6]);           // BCH(n, t) code
    uint n = (1 << logn) - 1;            // number of bins(PBS small d case)
    uint t = atoi(argv[7]);              // error correction capability
    uint split_num = atoi(argv[8]);  // if decode fails(too many errors), divide
                                     // into $split_num subsets
    std::vector<uint> seeds(ntest);
    std::ifstream myfile;
    myfile.open("seedpool.txt");
    for (int i = 0; i < ntest; ++i) {
        myfile >> seeds[i];
    }
    myfile.close();
    std::vector<uint> host(host_size), remote(remote_size);
    myfile.open("keypool.txt");
    uint host_unique_checksum = 0;
    uint remote_unique_checksum = 0;
    for (int i = 0; i < intersect_size; ++i) {
        myfile >> host[i];
        remote[i] = host[i];
    }
    for (int i = intersect_size; i < host_size; ++i) {
        myfile >> host[i];
        host_unique_checksum += host[i];
    }
    for (int i = intersect_size; i < remote_size; ++i) {
        myfile >> remote[i];
        remote_unique_checksum += remote[i];
    }
    myfile.close();
    uint sub_num = ceil(
        delta * (host_size + remote_size - intersect_size - intersect_size));
    auto pbs = PBS(host, remote);
    for (int k = 0; k < ntest; ++k) {
        pbs.reconcile(sub_num, logn, t, seeds[k], split_num);
        if (host_unique_checksum != pbs.host_unique_checksum_ ||
            remote_unique_checksum != pbs.remote_unique_checksum_) {
            std::cout << "Reconcilation Failed!" << std::endl;
        }
        pbs.clear();
    }
    return 0;
}