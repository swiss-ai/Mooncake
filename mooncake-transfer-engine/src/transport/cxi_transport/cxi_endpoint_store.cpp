// Copyright 2024 KVCache.AI
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "transport/cxi_transport/cxi_transport.h"

#include <fcntl.h>
#include <sys/epoll.h>

#include <atomic>
#include <cassert>
#include <fstream>
#include <memory>
#include <thread>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "config.h"
#include "transport/cxi_transport/cxi_endpoint.h"
#include "transport/cxi_transport/cxi_transport.h"
#include "transport/transport.h"

// CxiEndpointStore implementation
namespace mooncake {

std::shared_ptr<CxiEndpoint> CxiEndpointStore::get(
    const std::string &peer_nic_path) {
    RWSpinlock::ReadGuard guard(lock_);
    auto it = endpoints_.find(peer_nic_path);
    if (it != endpoints_.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<CxiEndpoint> CxiEndpointStore::getOrInsert(
    const std::string &peer_nic_path, std::shared_ptr<CxiEndpoint> new_ep) {
    RWSpinlock::WriteGuard guard(lock_);
    auto it = endpoints_.find(peer_nic_path);
    if (it != endpoints_.end()) {
        return it->second;  // Another thread already created it
    }
    endpoints_[peer_nic_path] = new_ep;
    return new_ep;
}

void CxiEndpointStore::add(const std::string &peer_nic_path,
                           std::shared_ptr<CxiEndpoint> endpoint) {
    RWSpinlock::WriteGuard guard(lock_);
    endpoints_[peer_nic_path] = endpoint;
}

void CxiEndpointStore::remove(const std::string &peer_nic_path) {
    RWSpinlock::WriteGuard guard(lock_);
    endpoints_.erase(peer_nic_path);
}

int CxiEndpointStore::disconnectAll() {
    RWSpinlock::WriteGuard guard(lock_);
    for (auto &entry : endpoints_) {
        if (entry.second) {
            entry.second->disconnect();
        }
    }
    return 0;
}

size_t CxiEndpointStore::size() const {
    RWSpinlock::ReadGuard guard(lock_);
    return endpoints_.size();
}

}