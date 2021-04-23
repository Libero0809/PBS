#include <iostream>
#include <queue>
#include <set>
#include <unordered_set>
#include <vector>

#include "minisketch.h"
#include "xxhash.h"

uint myhash(uint key, uint seed) { return XXH32(&key, sizeof(key), seed); }

class Host {
   public:
    uint t_ = 0;
    uint logn_ = 0;
    uint n_ = 0;
    uint checksum_ = 0;
    minisketch* sketch_ = nullptr;
    std::unordered_set<uint> items_ = {};
    std::vector<uint> xor_sums_ = {};
    std::set<uint> mines_ = {};
    std::set<uint> others_ = {};
    Host(uint logn, uint t) {
        logn_ = logn;
        n_ = (1 << logn_) - 1;
        t_ = t;
    }

    void init_checksum() {
        for (const uint& item : items_) {
            checksum_ += item;
        }
    }

    void create_sketch(uint seed) {
        xor_sums_.assign(n_, 0);
        sketch_ = minisketch_create(logn_, 0, t_);
        for (const uint& item : items_) {
            uint i = myhash(item, seed) % n_;
            minisketch_add_uint64(sketch_, i + 1);
            xor_sums_[i] ^= item;
        }
    }

    std::vector<Host> split(uint n, uint seed) {
        auto hosts = std::vector<Host>(n, Host(logn_, t_));
        for (const uint& item : items_) {
            uint i = myhash(item, seed) % n;
            hosts[i].items_.insert(item);
            hosts[i].checksum_ += item;
        }
        for (const uint& mine : mines_) {
            uint i = myhash(mine, seed) % n;
            hosts[i].mines_.insert(mine);
        }
        for (const uint& other : others_) {
            uint i = myhash(other, seed) % n;
            hosts[i].others_.insert(other);
        }
        return hosts;
    }

    unsigned char* encode() {
        static size_t buffer_size = minisketch_serialized_size(sketch_);
        unsigned char* buffer = new unsigned char[buffer_size];
        minisketch_serialize(sketch_, buffer);
        minisketch_destroy(sketch_);
        return buffer;
    }

    int reconcile(std::vector<uint>& results) {
        if (results[0] == t_ + 1) {  // too many distinct items case
            return -1;
        }
        auto end = results[0] << 1;
        // update A ^ D1 ^ D2...... and D1 ^ D2
        for (uint i = 1; i <= end; i += 2) {
            auto d = xor_sums_[results[i]] ^ results[i + 1];
            xor_sums_[results[i]] ^= d;
            if (items_.find(d) != items_.end()) {
                items_.erase(d);
                checksum_ -= d;
                if (others_.find(d) != others_.end()) {
                    others_.erase(d);
                } else {
                    mines_.insert(d);
                }
            } else {
                items_.insert(d);
                checksum_ += d;
                if (mines_.find(d) != mines_.end()) {
                    mines_.erase(d);
                } else {
                    others_.insert(d);
                }
            }
        }
        auto checksum = results[end + 1];
        if (checksum == checksum_) {
            return 0;
        }
        return 1;
    }
};

class Remote {
   public:
    uint t_ = 0;
    uint logn_ = 0;
    uint n_ = 0;
    uint checksum_ = 0;
    minisketch* sketch_ = nullptr;
    std::unordered_set<uint> items_ = {};
    std::vector<uint> xor_sums_ = {};
    std::set<uint> mines_ = {};
    std::set<uint> others_ = {};
    Remote(uint logn, uint t) {
        logn_ = logn;
        n_ = (1 << logn_) - 1;
        t_ = t;
    }

    void init_checksum() {
        for (const uint& item : items_) {
            checksum_ += item;
        }
    }

    void create_sketch(uint seed) {
        xor_sums_.assign(n_, 0);
        sketch_ = minisketch_create(logn_, 0, t_);
        for (const uint& item : items_) {
            uint i = myhash(item, seed) % n_;
            minisketch_add_uint64(sketch_, i + 1);
            xor_sums_[i] ^= item;
        }
    }

    std::vector<Remote> split(uint n, uint seed) {
        auto remotes = std::vector<Remote>(n, Remote(logn_, t_));
        for (const uint& item : items_) {
            uint i = myhash(item, seed) % n;
            remotes[i].items_.insert(item);
            remotes[i].checksum_ += item;
        }
        for (const uint& mine : mines_) {
            uint i = myhash(mine, seed) % n;
            remotes[i].mines_.insert(mine);
        }
        for (const uint& other : others_) {
            uint i = myhash(other, seed) % n;
            remotes[i].others_.insert(other);
        }
        return remotes;
    }

    std::vector<uint> decode(unsigned char* buffer) {
        auto sketch = minisketch_create(logn_, 0, t_);
        minisketch_deserialize(sketch, buffer);
        delete[] buffer;
        minisketch_merge(sketch_, sketch);
        minisketch_destroy(sketch);
        uint64_t* differences = new uint64_t[t_];

        ssize_t num = minisketch_decode(sketch_, t_, differences);
        minisketch_destroy(sketch_);
        std::vector<uint> results = {};
        if (num < 0) {  // too many bit errors
            results.push_back(t_ + 1);
        } else {  // decode success, find the positions and xor sums
            results.push_back(num);
            for (int i = 0; i < num; ++i) {
                results.push_back(static_cast<uint>(differences[i] - 1));
                results.push_back(xor_sums_[differences[i] - 1]);
            }
            results.push_back(checksum_);
        }
        delete[] differences;
        return results;
    }
};

class PBS {
   public:
    const std::vector<uint> host_items_;
    const std::vector<uint> remote_items_;
    std::vector<uint> host_uniques_ = {};    // The unique items on host side
    std::vector<uint> remote_uniques_ = {};  // The unique items on remote side
    uint host_unique_checksum_ = 0;
    uint remote_unique_checksum_ = 0;
    std::queue<Host> hosts_ = {};
    std::queue<Remote> remotes_ = {};
    PBS(std::vector<uint> host_items, std::vector<uint> remote_items)
        : host_items_(std::move(host_items)),
          remote_items_(std::move(remote_items)) {}

    void clear() {
        std::queue<Host> hosts = {};
        hosts_.swap(hosts);
        std::queue<Remote> remotes = {};
        remotes_.swap(remotes);
        host_uniques_.clear();
        remote_uniques_.clear();
        host_unique_checksum_ = 0;
        remote_unique_checksum_ = 0;
    }
    // split the items into multiple groups, so d in each group is small
    void split(uint group_num, uint logn, uint t, uint seed) {
        auto hosts = std::vector<Host>(group_num, Host(logn, t));
        auto remotes = std::vector<Remote>(group_num, Remote(logn, t));
        for (const uint& host_item : host_items_) {
            uint i = myhash(host_item, seed) % group_num;
            hosts[i].items_.insert(host_item);
        }
        for (const uint& remote_item : remote_items_) {
            uint i = myhash(remote_item, seed) % group_num;
            remotes[i].items_.insert(remote_item);
        }
        for (auto& host : hosts) {
            host.init_checksum();
            hosts_.push(std::move(host));
        }
        for (auto& remote : remotes) {
            remote.init_checksum();
            remotes_.push(std::move(remote));
        }
    }

    void reconcile(uint group_num, uint logn, uint t, uint seed,
                   uint split_num) {
        auto sketch = minisketch_create(logn, 0, t);
        size_t bch_size = minisketch_serialized_size(sketch) << 3;
        minisketch_destroy(sketch);
        size_t group_index_size = ceil(log2(group_num));
        size_t xor_sum_size = (sizeof(uint) << 3) + logn;
        size_t difference_size = ceil(log2(t + 2));
        size_t checksum_size = sizeof(uint) << 3;

        uint bch_num = 0;
        uint group_index_num = 0;
        uint xor_sum_num = 0;
        uint difference_num = 0;
        uint checksum_num = 0;

        // Split the items to fall into the 'PBS for small d' case
        split(group_num, logn, t, seed);
        uint round = 1;
        while (not hosts_.empty()) {
            group_num = hosts_.size();
            bch_num += group_num;
            difference_num += group_num;
            while (group_num--) {
                auto& host = hosts_.front();
                auto& remote = remotes_.front();
                // create sketch on both side
                host.create_sketch(seed + round);
                remote.create_sketch(seed + round);
                // encode on host side
                auto buffer = host.encode();
                // decode on remote side
                auto results = remote.decode(buffer);
                if (results.size() > 1) {
                    checksum_num++;
                    xor_sum_num += (results.size() >> 1) - 1;
                }
                // reconcile on host side
                auto ret = host.reconcile(results);
                if (ret == 0) {  // reconcile success
                    host_uniques_.insert(host_uniques_.end(),
                                         host.mines_.begin(),
                                         host.mines_.end());
                    remote_uniques_.insert(remote_uniques_.end(),
                                           host.others_.begin(),
                                           host.others_.end());
                } else if (ret == -1) {  // too many distinct items, split into
                                         // $split_num
                    auto hosts = host.split(split_num, seed + round);
                    auto remotes = remote.split(split_num, seed + round);
                    for (auto& h : hosts) {
                        hosts_.push(std::move(h));
                    }
                    for (auto& r : remotes) {
                        remotes_.push(std::move(r));
                    }
                } else if (ret ==
                           1) {  // checksum check failed, go to next round
                    hosts_.push(std::move(host));
                    remotes_.push(std::move(remote));
                    group_index_num++;
                }
                hosts_.pop();
                remotes_.pop();
            }
            round++;
        }
        for (auto& key : host_uniques_) {
            host_unique_checksum_ += key;
        }
        for (auto& key : remote_uniques_) {
            remote_unique_checksum_ += key;
        }

        uint64_t total_bits =
            bch_num * bch_size + group_index_num * group_index_size +
            xor_sum_num * xor_sum_size + difference_num * difference_size +
            checksum_num * checksum_size;
        std::cout << "Rounds: " << round - 1 << std::endl;
        std::cout << "Total Bits:" << total_bits << std::endl;
    }
};