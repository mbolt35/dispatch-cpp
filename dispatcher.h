////////////////////////////////////////////////////////////////////////////////
//
// The MIT License (MIT)
// 
// Copyright (c) 2015 Matt Bolt
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
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
#include "listener.h"
#include "helpers.h"


namespace dispatch {
    
    /**
     * This class is used to dispatch signals to various listeners.
     */
    class dispatcher {
        public:
            //----------------------------------
            //  constructor
            //----------------------------------

            dispatcher();

            //----------------------------------
            //  destructor
            //----------------------------------

            ~dispatcher();

            //----------------------------------
            //  methods
            //----------------------------------

            template<class T>
            void add(listener<T>* ptr) {
                //typedef typename remove_all_qualifiers<T>::type TT;
                std::type_index key(typeid(T));
                m_listeners[key].emplace_back(ptr);
            }

            template<class E>
            void remove(const E& dispatchListener) {
                typedef function_param_at<function_type_for_t<E>, 0> T;
                //typedef typename remove_all_qualifiers<T>::type TT;
            
                std::uintptr_t addr = pointer_memory<E>::address_for(dispatchListener);

                std::type_index key(typeid(T));
                std::vector<handler_t>* v = &m_listeners[key];

                auto result = std::find_if(v->begin(), v->end(), [&addr](const handler_t& check) {
                    auto lptr = std::static_pointer_cast<listener<T>>(check);
                    return *lptr == addr;
                });

                if (result != v->end()) {
                    v->erase(result);
                }
            }

            template<class T>
            inline dispatcher& operator+=(const T& dispatchListener) {
                wrap_add(
                    function_wrapper<T>::wrap(dispatchListener),
                    pointer_memory<T>::address_for(dispatchListener));

                return *this;
            }

            template<class T>
            inline dispatcher& operator-=(const T& dispatchListener) {
                remove<T>(dispatchListener);

                return *this;
            }

            template<class T> 
            void dispatch(T value) {
                static_assert(std::is_base_of<signal, full_decay_t<T>>::value, "T type must implement signal.");

                std::type_index key(typeid(T));
                for (auto f : m_listeners[key]) {
                    std::shared_ptr<listener<T>> ptr = std::static_pointer_cast<listener<T>>(f);
                    (*ptr)(value);
                }
            }

        private:
            typedef std::shared_ptr<handler> handler_t;

            std::unordered_map<std::type_index, std::vector<handler_t>> m_listeners;
        
            template<class T>
            void wrap_add(const T& dispatchListener, std::uintptr_t addr) {
                typedef function_param_at<T, 0> E;
                add(new listener<E>(dispatchListener, addr));
            }
    };

    inline dispatcher::dispatcher() { }
    inline dispatcher::~dispatcher() { }
};