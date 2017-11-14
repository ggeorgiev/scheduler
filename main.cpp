/**
  * Add any include you need
  */

#include <functional>
#include <iostream>
#include <chrono>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iomanip>
#include <cassert>

void printTime(std::chrono::system_clock::time_point t) {
    auto c = std::chrono::system_clock::to_time_t(t);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t.time_since_epoch()) -
              std::chrono::duration_cast<std::chrono::seconds>(t.time_since_epoch());
    
    std::cout << std::put_time(std::localtime(&c), "%T.")
              << std::setfill('0') << std::setw(3) << ms.count() << ": ";
}

class JobScheduler {
    typedef std::function<void()> function_t;
    typedef std::chrono::time_point<std::chrono::steady_clock> time_point_t;

    // Priority queue item struct. Holds the time_point a job needs to be executed and the job.
    struct QueueItem {
        // QueueItem constrctor
        QueueItem(time_point_t timePoint, function_t job) :
            timePoint(timePoint),
            job(job) {
        }
        
        // Less operator. Allows for priority determination.
        const bool operator <(const QueueItem& r) const {
            // Reverse the direction. We want the earliest time first.
            return timePoint > r.timePoint;
        }

        time_point_t timePoint;
        function_t job;
    };
    
    typedef std::priority_queue<QueueItem> priority_queue;

public:
    /**
     * Starts the executor. Returns immediately
     */
    void start() {
        // Start a working thread to observe serve the shaduled jobs.
        workThread.reset(new std::thread([=]{
            std::unique_lock<std::mutex> lockQueue(queueMutex);
            for (;;) {
                // Wait until we have at least one scheduled item, or we want to stop.
                queueVariable.wait(lockQueue, [=]{return stopped || queue.size() > 0;});
                
                // Loop until there are items in the queue.
                while (queue.size() > 0) {
                    
                    // Grab the job from the top. It needs to be executed sooner.
                    auto top = queue.top();

                    // If its time is already reached execute it and move to the next.
                    if (top.timePoint <= std::chrono::steady_clock::now()) {
                        printTime(std::chrono::system_clock::now());
                        top.job();
                        queue.pop();
                        continue;
                    }
                    
                    // Wait until the first job time comes, or we have a new one scheduled.
                    // We need to check, it might be sooner than the current one.
                    size_t size = queue.size();
                    queueVariable.wait_until(lockQueue, top.timePoint, [=]{return size != queue.size();});
                }
                
                // We finished all scheduled jobs, if we need to stop it is a good time for that.
                if (stopped) {
                    break;
                }
            }
        }));
    }
    
    /**
     * Executes/Schedules the job "job" after "milliseconds"
     * It will return immediately
     */
    void schedule(function_t job, uint32_t milliseconds) {
        std::cout << "job scheduled at ";
        printTime(std::chrono::system_clock::now());
        std::cout << " to execute after " << milliseconds << "ms" << std::endl;

        {
            // Add the job to the queue.
            std::unique_lock<std::mutex> lockQueue(queueMutex);
            queue.emplace(std::chrono::steady_clock::now() + std::chrono::milliseconds(milliseconds), job);
        }
        // Notify the working thread for the new job.
        queueVariable.notify_all();
    }
    
    /**
     * Will return after all the jobs have been executed
     */
    void stop() {
        assert(workThread != nullptr);

        {
            // Set the stopped flag
            std::unique_lock<std::mutex> lockQueue(queueMutex);
            stopped = true;
        }
        // Notify the working thread for the change
        queueVariable.notify_all();
        
        // Wait until all the jobs are executed.
        workThread->join();
    }
    
private:
    std::mutex queueMutex;
    std::condition_variable queueVariable;
    priority_queue queue;
    
    bool stopped = false;
    std::unique_ptr<std::thread> workThread;
};


int main() {
    JobScheduler scheduler;
    scheduler.start();

    // Add some jobs
    scheduler.schedule([] { std::cout << "job 3s" << std::endl; }, 3000);
    scheduler.schedule([] { std::cout << "job 4s" << std::endl; }, 4000);
    scheduler.schedule([] { std::cout << "job 5s" << std::endl; }, 5000);

    // Wait for a second to make sure that the working thread is waiting on
    // the first job for 2 more seconds.
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // Add a job that needs to runn before the current top one.
    scheduler.schedule([] { std::cout << "job 2s" << std::endl; }, 1000);
    scheduler.schedule([] { std::cout << "job 6s" << std::endl; }, 5000);
    
    // Wait until all the current jobs are executed to get out from the internal loop.
    std::this_thread::sleep_for(std::chrono::milliseconds(7000));
    
    // Add some more jobs
    scheduler.schedule([] { std::cout << "job 11s" << std::endl; }, 3000);
    scheduler.schedule([] { std::cout << "job 9s" << std::endl; }, 1000);
    scheduler.schedule([] { std::cout << "job 10s" << std::endl; }, 2000);

    scheduler.stop();
}
