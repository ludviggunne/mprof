
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

#if defined(__GNUC__) && ((__GNUC__ >= 4 && __GNUC_MINOR__ >= 5) || (__GNUC__ >= 5))
#include <x86intrin.h>
#define _MPROF_FUNCTION __PRETTY_FUNCTION__
#else
#error "Only GCC (v4.5 or higher) supported"
#endif

#define MPROF_PROFILE_HERE()\
    static mprof::record_t &__record = mprof::_internal::new_record(_MPROF_FUNCTION);\
    mprof::_internal::scope_t __scope(__record);

namespace mprof {

    struct record_t {

        record_t(const char *fnname) : fnname(fnname), cycles(0), calls(0) {}

        const char *fnname;
        uint64_t    cycles;
        uint64_t    calls;
    };

    struct result_t {

        result_t();

        uint64_t              cycles;
        std::vector<record_t> records;
    };

    void set_result_handler(void (*handler)(const result_t &));

    namespace _internal {

        record_t &new_record(const char *fnname);

        class scope_t {
        public:
            scope_t(record_t &record)
                : _record(record),
                _begin(__rdtsc())
            {
            }

            ~scope_t()
            {
                _record.cycles += __rdtsc() - _begin;
                _record.calls++;
            }

        private:
            record_t      &_record;
            const uint64_t _begin;
        };
    }
}

#endif /* MPROF_HEADER */

#ifdef MPROF_IMPLEMENTATION

namespace mprof {

    static void (*result_handler)(const result_t &) = nullptr;
    static struct _result_wrapper {

        _result_wrapper() : begin(__rdtsc()) {}

        ~_result_wrapper()
        {   
            result.cycles = __rdtsc() - begin;
            result_handler(result);
        }

        uint64_t begin;
        result_t result;
    } result_wrapper;

    result_t::result_t() : cycles(0) {}

    void set_result_handler(void (*handler)(const result_t &)) 
    {
        result_handler = handler;
    }

    namespace _internal {

        record_t &new_record(const char *fnname) {

            return result_wrapper.result.records.emplace_back(fnname);
        }
    }
}

#endif /* MPROF_IMPLEMENTATION */