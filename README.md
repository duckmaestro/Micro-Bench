# Micro Bench
A microbenchmark suite for 32-bit Windows and compatible.

## History
**Micro Bench** started life as the "CSE 221 Profiler", a joint project
between Clifford Champion and Daniel Cashman for a UC San Diego 
winter 2012 graduate course on operating system design.

After completion, we both agreed this was worthwhile code and decided 
to post somewhere, Github being an obvious choice.

## Words of caution
First a word of caution-- the benchmark code is very intensive when ran. 
Through the course of its execution it may:

* Spawn many threads and processes at priority 31 (`REALTIME` / `TIME_CRITICAL`)
* Consume multiple GiBytes of virtual memory (upwards of 12 GB)
* Heavily utilize hard disk I/O

!IMPORTANT! We highly recommend you review the source code of the one or more
experiment(s) you are interested in running, before you actually run it. We are 
not resposible for lost data or other harm that may occur for one reason or another.
Our code was written and used for our class project and has been tested in our
own development environments only.

## What to expect
Micro Bench is a Win32 console application, built and tested for Visual Studio 2010.
By default no experiments run at start up besides a simple self-test 
to measure overhead of our stopwatch. 

In `main.cpp` you will find a 
number of "Experiments" available to specify for inclusion in the main run. 
Once you have included one or more experiments, you may run the application 
and results will be printed on-screen (usually in microseconds / μs) upon completion.

Also note that some experiments assume hard-coded root disk paths (i.e. "D:\" or "K:\"). 
Please refer to the top of some c++ files for current values of constants.

## What we'd like to see

Future work we'd like to see happen:

* A more data-driven test system (config file, more command line switches)
* Better visualization of on-screen progress
* Ability to output full results to CSV or other charting friendly file formats

## The Authors

Clifford Champion

* email: cchampio [@t] cs.ucsd.edu
* tw: [@duckmaestro](http://twitter.com/duckmaestro)

Daniel Cashman

* email: dcashman [@t] cs.ucsd.edu
 

