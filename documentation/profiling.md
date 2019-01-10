## Profiling
CPU profiling can be performed by running the executable with `CPUPROFILE` environment variable set and specifying cmake parameter `PROFILE=true`.
For example:
```
CPUPROFILE=/tmp/prof.out <path/to/binary> [binary args]
```  
Use pprof to display the information. For example, to display as an SVG map in the browser:
```
google-pprof -web <path/to/binary> /tmp/prof.out
```
Note, this requires google perftools installed (tcmalloc and pprof). `gperftools` can be installed with Homebrew on OS X, or system repositories for most Linux distros.
