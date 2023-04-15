
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

        uint64_t                     total_cycles;
        uint64_t                     profiled_cycles;
        std::vector<accumulator_t *> accumulators;
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

    collector_t collector;
    
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
}

#endif /* MPROF_IMPLEMENTATION */