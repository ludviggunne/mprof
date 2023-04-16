
/*
 * mprof - single header C++ library for collecting and emitting profiling data
 * Copyright (C) 2023 Ludvig Gunne Lindström
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
*/

#ifndef MPROF_HEADER
#define MPROF_HEADER

#include <cstdint>
#include <vector>
#include <functional>
#include <stdio.h>

#if defined(__GNUC__) && ((__GNUC__ >= 4 && __GNUC_MINOR__ >= 5) || (__GNUC__ >= 5))
#include <x86intrin.h>
#define _MPROF_FUNCTION __PRETTY_FUNCTION__
#else
#error "Only GCC (v4.5 or higher) supported"
#endif

#define MPROF_PROFILE_HERE()\
    static auto __accumulator = new mprof::accumulator_t(_MPROF_FUNCTION);\
    mprof::scope_t __local_scope(__accumulator);

namespace mprof {

    struct accumulator_t {

        accumulator_t(const char * fnname);

        uint64_t     cycles;
        uint64_t     calls;
        const char * fnname;
    };

    struct result_t {

        uint64_t                           total_cycles;
        uint64_t                           profiled_cycles;
        std::vector<const accumulator_t *> accumulators;
    };

    void set_result_handler(std::function<void(const result_t &)> handler);

    class collector_t {
    public:
        collector_t()
        {
            result.total_cycles = __rdtsc();
        }

        ~collector_t()
        {
            result.total_cycles = __rdtsc() - result.total_cycles ;
            result_handler(result);
            for (auto accumulator : result.accumulators) {
                delete accumulator;
            }
        }

    private:

        void register_accumulator(accumulator_t * accumulator)
        {
            result.accumulators.push_back(accumulator);
        }

        friend class scope_t;
        friend class accumulator_t;
        friend void  set_result_handler(std::function<void(const result_t &)>);

        result_t result;
        std::function<void(const result_t &)> result_handler;
    };

    extern collector_t collector;
    
    class scope_t {

    public:
        scope_t(accumulator_t * accumulator)
            : _accumulator(accumulator)
        {
            _begin = __rdtsc();
        }

        ~scope_t()
        {
            auto cycles = __rdtsc();
            cycles -= _begin;

            _accumulator->cycles += cycles;
            _accumulator->calls++;

            collector.result.profiled_cycles += cycles;
        }

    private:
        uint64_t        _begin; 
        accumulator_t * _accumulator;
    };
}

#endif /* MPROF_HEADER */

#ifdef MPROF_IMPLEMENTATION

namespace mprof {

    accumulator_t::accumulator_t(const char * fnname)
        : fnname (fnname), 
          cycles (0), 
          calls  (0) 
    {
        collector.register_accumulator(this);
    }

    void set_result_handler(std::function<void(const result_t &)> handler) {
        collector.result_handler = handler;
    }

    collector_t collector;
}

#endif /* MPROF_IMPLEMENTATION */