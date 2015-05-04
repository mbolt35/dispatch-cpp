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

#include <fstream>
#include <iostream>
#include <iomanip>
#include <memory>
#include <map>
#include <string>
#include <sstream>
#include <type_traits>
#include <utility>
#include <functional>
#include <initializer_list>
#include <type_traits>
#include <algorithm>
#include <vector>
#include <string>
#include <tuple>
#include <cstdlib>
#include <cstring>


namespace dispatch {

    //--------------------------------
    //  From C++14 Standard
    //--------------------------------

    template<class T> struct is_null_pointer 
        : std::integral_constant<bool, std::is_same<T, std::nullptr_t>::value> { };


    //--------------------------------
    //  Basic Function Aliases
    //--------------------------------

    /**
     * runnable typedef
     */
    typedef std::function<void()> runnable;
    
    /**
     * Template alias for a function which accepts N arguments and has a void return type. 
     */
    template<class... Ts> 
    using action = std::function<void(const Ts&...)>;
    
    /**
     * Template alias for a function which accepts N arguments and has a <code>T</code> return type.
     */
    template<class T, class... Ts> 
    using func = std::function<T(const Ts&...)>;


    //--------------------------------
    //  Func Traits
    //--------------------------------

    /**
     * Used as the base type for func_traits.
     */
    template<class E, class...Args> struct func_trait_helper {
        /**
         * The return type of the function.
         */
        typedef E result_type;
        
        /**
         * The total number of arguments in this function.
         */
        static constexpr std::size_t arg_count = sizeof...(Args);
        
        /**
         * Extracts the Nth argument type from the function.
         */
        template<std::size_t N> struct arg {
            static_assert(N < arg_count, "N must be less than the number of arguments in the function.");
            typedef typename std::tuple_element<N, std::tuple<Args...>>::type type;
        };
    };

    /**
     * The <code>func_traits</code> implementations were designed to handle lambdas specifically,
     * but have ended up being used in all function type resolutions. This trait can be used to
     * find a parameter type, return size, or total number of arguments.
     */
    template<class T> struct func_traits 
        : func_traits<decltype(&T::operator())> { };

    template<class T, class E, class...Args> struct func_traits<E(T::*)(Args...)> 
        : func_trait_helper<E, Args...> { };

    template<class T, class E, class...Args> struct func_traits<E(T::*)(Args...) const>
        : func_trait_helper<E, Args...> { };

    template<class E, class...Args> struct func_traits<E(*)(Args...)>
        : func_trait_helper<E, Args...> { };
    

    //--------------------------------
    //  Func Trait Aliases
    //--------------------------------

    /**
     * Retrieves the type for a function parameter at the <code>N</code> index.
     */
    template<class T, std::size_t N>
    using function_param_at = typename func_traits<T>::template arg<N>::type;
 

    //--------------------------------
    //  Full Type Decay
    //--------------------------------

    /**
     * Removes the pointer from <code>std::decay</code> output.
     */
    template<class T> struct full_decay 
        : std::remove_pointer<typename std::decay<T>::type> { };

    /**
     * Full Decay Alias
     */
    template<class T> 
    using full_decay_t = typename full_decay<T>::type;
    

    //--------------------------------
    //  Func Conditionals
    //--------------------------------

    /**
     * Removes the pointer and checks against std::is_function.
     */
    template<class T> struct is_global_function_pointer 
        : std::is_function<full_decay_t<T>> { };

    /**
     * Some global namespace function pointers require being stripped of qualifiers using 
     * <code>std::decay</code>, then removing and re-adding the pointer to the type. 
     */
    template<class T> struct sanitize
        : std::conditional<
            is_global_function_pointer<T>::value, 
            typename std::add_pointer<full_decay_t<T>>::type,
            T> { };

    /**
     * Checks to see if a pointer is a global/static function pointer or is a member function pointer.
     */
    template<class T> struct is_function_pointer 
        : std::integral_constant<
            bool, 
            is_global_function_pointer<T>::value || std::is_member_function_pointer<T>::value> { };


    //--------------------------------
    //  Template Type Traits
    //--------------------------------

    /**
     * The template_params implementations leverage <code>std::tuple</code> and <code>std::tuple_element</code> to 
     * extract single types from template parameters.
     */
    template<class...Ts> struct template_params {
        /**
         * A <code>std::tuple</code> containing the types.
         */
        typedef std::tuple<Ts...> types;

        /**
         * The total number of template parameters.
         */
        static constexpr std::size_t num_params = sizeof...(Ts);

        /**
         * This inner template leverages <code>std::tuple_element</code> to extract the 
         * template parameter at the <code>N</code> index.
         */
        template<std::size_t N> struct type_at {
            static_assert(N < num_params, "N must be less than the number of inner types.");
            typedef typename std::tuple_element<N, types>::type type;
        };
    };

    template<template<class...> class T, class...Ts> struct template_params<T<Ts...>>
        : template_params<Ts...> { };

    template<template<bool, class...> class T, bool B, class...Ts> struct template_params<T<B, Ts...>>
        : template_params<Ts...> { };


    //--------------------------------
    //  Template Conditions
    //--------------------------------

    /**
     * This trait contains a value parameter set to the result of the query.
     */
    template<class T> struct has_template_params 
        : public std::false_type { };

    template<template<class...> class T, class...Ts> struct has_template_params<T<Ts...>> 
        : public std::true_type { };

    template<template<bool, class...> class T, bool A, class...Ts> struct has_template_params<T<A, Ts...>> 
        : public std::true_type { };
    
    template<class T, std::size_t N>
    using template_param_at = typename template_params<T>::template type_at<N>::type;
    

    //--------------------------------
    //  Function Binding Traits
    //--------------------------------

    /**
     * This trait essentially searches a platform independent implementation of a std::bind result
     * that contains a reference to the inner function. 
     */
    template<class T, class...Ts> struct function_in_binding 
        : std::conditional<
            has_template_params<T>::value,
            typename function_in_binding<T>::type,
            typename std::conditional<
                is_function_pointer<T>::value,
                typename sanitize<T>::type,
                typename function_in_binding<Ts...>::type
            >::type> { };

    /** 
     * This is the evaluation piece that runs last (only single T instance remaining).
     */
    template<class T> struct function_in_binding<T>
        : std::conditional<
            is_function_pointer<T>::value, 
            typename sanitize<T>::type,
            std::nullptr_t> { };

    /**
     * This is a basic nested search implementation to allow inner recursion if the template
     * types containg templated classes.
     */
    template<template<class...> class T, class...Ts> struct function_in_binding<T<Ts...>> 
        : function_in_binding<Ts...> { };

    /**
     * Hack: This implementation is specifically for deriving the MSVC implementation of std::bind
     */
    template<template<bool, class...> class T, bool A, class...Ts> struct function_in_binding<T<A, Ts...>>
        : function_in_binding<Ts...> { };

    /**
     * Function in Binding Alias
     */
    template<class T> 
    using function_in_binding_t = typename function_in_binding<T>::type;

    /**
     * Determines whether or not a function existings in the binding.
     */
    template<class T> struct is_function_in_binding 
        : std::integral_constant<bool, !is_null_pointer<function_in_binding_t<T>>::value> { };


    //--------------------------------
    //  Func Traits
    //--------------------------------

    /**
     * This method examines the <code>T</code> type as if it was a function pointer or binding
     * expression. If binding expression, it will find the inner function pointer. Otherwise,
     * the <code>type</code> is simply <code>T</code>
     */
    template<class T> struct function_type_for 
        : std::conditional<
            std::is_bind_expression<T>::value,
            typename function_in_binding<T>::type,
            T> { };

    /**
     * Function Type For Alias.
     */
    template<class T> 
    using function_type_for_t = typename function_type_for<T>::type;


    //--------------------------------
    //  Function Binding Wrapper
    //--------------------------------

    /**
     * Since <code>std::bind</code> creates an object that doesn't necessarily work with std::function 
     * too well, we use our <code>function_in_binding</code> traits to build a compatible wrapper at
     * compile time which will delegate calls to the std::bind result.
     */
    template<class T, bool B=std::is_bind_expression<T>::value> struct function_wrapper {
        static T wrap(const T& t) { return t; }
    };

    template<class T>  struct function_wrapper<T, true> {
        typedef function_param_at<function_in_binding_t<T>, 0> E;
        
        static std::function<void(const E&)> wrap(const T& t) {
            // We pass by reference to ensure that the memory address for the binding
            // is consistent. In order to wrap the binding, we have to remove the const
            // which gives us the original std::bind() result.
            T& callable = const_cast<T&>(t);
            return [&callable](const E& v) { callable(v); };
        }
    };


    //--------------------------------
    //  Pointer Address Helpers
    //--------------------------------

    template<class T, bool P = std::is_pointer<T>::value>
    struct pointer_memory {
        static std::uintptr_t address_for(const T& p) {
            return reinterpret_cast<std::uintptr_t>(std::addressof(p));
        }
    };

    template<class T>
    struct pointer_memory<T, true> {
        static std::uintptr_t address_for(const T& p) {
            typedef decltype(*p) E;

            E& addr = *p;
            return reinterpret_cast<std::uintptr_t>(std::addressof(addr));
        }
    };

    //--------------------------------
    //  Pointer Type Resolution
    //--------------------------------

    /**
     * Used to dereference a type.
     */
    template<class T> struct deref {
        typedef T type;
    };
    
    /**
     * Specialized version to dereference a pointer type back to T
     */
    template<class T> struct deref<T*> {
        typedef T type;
    };
    
    /**
     * Specialized version to dereference a shared_ptr back to T.
     */
    template<class T> struct deref<std::shared_ptr<T>> {
        typedef T type;
    };
    
    /**
     * Specialized version to dereference a weak_ptr back to T.
     */
    template<class T> struct deref<std::weak_ptr<T>> {
        typedef T type;
    };
    
    /**
     * Specialized version to dereference a unique_ptr back to T.
     */
    template<class T> struct deref<std::unique_ptr<T>> {
        typedef T type;
    };
    
};
