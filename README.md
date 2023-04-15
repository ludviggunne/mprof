# mprof
Single header C++ library for collecting and emitting profiling data.

## Usage

Include `mprof.hpp` and add `MPROF_PROFILE_HERE();` to the top of every function body you want to profile.
```c++
#include "mprof.h"

// . . . 

void do_something() {

    MPROF_PROFILE_HERE();

    // . . .

}
```
Define `MPROF_IMPLEMENTATION` before including header in exactly one source file.
```c++
#define MPROF_IMPLEMENTATION
#include "mprof.hpp"
// . . .
```
Set how mprof should handle th eprofiling result on program exit, by using `mprof::set_result_handler`. This function is passed a const reference to a `result_t` struct, containing
- `total_cycles`: the total number of CPU cycles recorded by mprof during execution.
- `profiled_cycles`: the total number of CPU cycles spent in functions marked with `MPROF_PROFILE_HERE`.
- `acc->mulators`: a `std::vector` of pointers to `acc->mulator_t` structs, each associated with a profiled function. Each acc->mulator contains
    - `fnname`: the name of the function as a C-string.
    - `cycles`: number of CPU cycles spent inside function.
    - `calls`: number of times funciton was called
### Example:
```c++
#include <stdio.h>
#include "mprof.h"

int main(it argc, char *argv[]) {

    mprof::set_result_handler([](const mprof::result_t & result) {

        for (auto acc : result.acc->mulators) {

            printf(
                "Function %s:\n"
                "    Calls:                %ld\n"
                "    Cycles:               %ld\n"
                "    %% of profiled cycles: %d\n"
                "    %% of total cycles:    %d\n\n",
                acc->fnname,
                acc->calls,
                acc->cycles,
                static_cast<int> (100.0f * acc->cycles / result.profiled_cycles),
                static_cast<int> (100.0f * acc->cycles / result.total_cycles)
            );
        }
    });

    // . . .

}
```
### Result:

```consolel
Function void do_number_stuff(int, double):
    Calls:                5
    Cycles:               30570
    % of profiled cycles: 62
    % of total cycles:    48

Function int do_other_stuff(const char*):
    Calls:                2
    Cycles:               17996
    % of profiled cycles: 37
    % of total cycles:    28
```