# scheduler

Scheduler is a single c++ file prepared with XCode Version 6.2.

To compile on a Mac with already installed XCode clone the repo, navigate to its root directory and run the command:

    clang++ -std=c++11 main.cpp -o scheduler
   
This will produce an executable scheduler. You can execute it with the command:

    ./scheduler
    
The expected result is something like the following, where the time in the beginning of the line will depend on the time you running it: 

    job scheduled at 21:09:10.182:  to execute after 3000ms
    job scheduled at 21:09:10.182:  to execute after 4000ms
    job scheduled at 21:09:10.182:  to execute after 5000ms
    job scheduled at 21:09:11.183:  to execute after 1000ms
    job scheduled at 21:09:11.183:  to execute after 5000ms
    21:09:12.183: job 2s
    21:09:13.183: job 3s
    21:09:14.183: job 4s
    21:09:15.183: job 5s
    21:09:16.184: job 6s
    job scheduled at 21:09:18.184:  to execute after 3000ms
    job scheduled at 21:09:18.184:  to execute after 1000ms
    job scheduled at 21:09:18.184:  to execute after 2000ms
    21:09:19.184: job 9s
    21:09:20.184: job 10s
    21:09:21.185: job 11s
