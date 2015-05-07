////////////////////////////////////////////////////////////////////////////////
//
//  MATTBOLT.BLOGSPOT.COM
//  Copyright(C) 2014 Matt Bolt
//
//  Licensed under the Apache License, get_version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at:
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <type_traits>
#include <typeinfo>
#include <typeindex>
#include "handler.h"
#include "signal.h"
#include "helpers.h"


namespace dispatch {
  
    /**
     * The listener class is the global wrapper for all listening function types awaiting signals. These
     * types include <code>std::function</code>, lambdas, <code>std::bind</code>, and C-Style function pointers.
     * The key to comparison is by memory address, so each of the type's addresses must be maintained through the
     * listening process. These addresses can be passed in as a ctor parameter or we can use the <code>pointer_memory</code>
     * tool to identify it.
     */
    template<class T> 
    struct listener : public handler {
        static_assert(std::is_base_of<signal, full_decay_t<T>>::value, "listener<T>: T instance must implement signal.");

        template<class E>
        listener(const E& callable, std::uintptr_t addr)
            : m_callable(callable),
              m_addr(addr)
        { }
        
        template<class E>
        listener(const E& callable) 
            : m_callable(callable),
              m_addr(pointer_memory<E>::address_for(callable))
        { }

        inline void operator()(const T& s) const {
            m_callable(s);
        }

        inline bool operator==(const listener<T>& rhs) const {
            return m_addr == rhs.m_addr;
        }

        inline bool operator==(std::uintptr_t addr) const {
            return m_addr == addr;
        }

        private:
            std::function<void(T)> m_callable;
            std::uintptr_t m_addr;
    };

};
