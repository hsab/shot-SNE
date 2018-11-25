#pragma once

#include "ofMain.h"
#include <atomic>

/// This is a simple example of a ThreadedObject created by extending ofThread.
/// It contains data (count) that will be accessed from within and outside the
/// thread and demonstrates several of the data protection mechanisms (aka
/// mutexes).
class ThreadedObject : public ofThread {
public:
    /// On destruction wait for the thread to finish
    /// so we don't destroy the pixels while they are
    /// being used. Otherwise we would crash
    ~ThreadedObject()
    {
        stop();
        waitForThread(false);
    }

    void setup(string c)
    {
        cmd = c;
        start();
    }

    /// Start the thread.
    void start()
    {
        startThread();
    }

    /// Signal the thread to stop.  After calling this method,
    /// isThreadRunning() will return false and the while loop will stop
    /// next time it has the chance to.
    /// In order for the thread to actually go out of the while loop
    /// we need to notify the condition, otherwise the thread will
    /// sleep there forever.
    /// We also lock the mutex so the notify_all call only happens
    /// once the thread is waiting. We lock the mutex during the
    /// whole while loop but when we call condition.wait, that
    /// unlocks the mutex which ensures that we'll only call
    /// stop and notify here once the condition is waiting
    void stop()
    {
        system("killall hecate");
        std::unique_lock<std::mutex> lck(mutex);
        stopThread();
        condition.notify_all();
    }

    /// Everything in this function will happen in a different
    /// thread which leaves the main thread completelty free for
    /// other tasks.
    void threadedFunction()
    {
        while (isThreadRunning()) {
            if (!isProcessing) {
                isProcessing = true;
                // Lock the mutex until the end of the block, until the closing }
                // in which this variable is contained or we unlock it explicitly
                std::unique_lock<std::mutex> lock(mutex);

                // The mutex is now locked so we can modify
                // the shared memory without problem
                exec(cmd.c_str());

                // Now we wait for the main thread to finish
                // with this frame until we can generate a new one
                // This sleeps the thread until the condition is signaled
                // and unlocks the mutex so the main thread can lock it
                condition.wait(lock);
            }
        }
    }

    void exec(const char* cmd)
    {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
        if (!pipe)
            throw std::runtime_error("popen() failed!");
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
            result += buffer.data();

        metadata = result;
        processed = true;
        cout << result << endl;
    }

    void update()
    {
        // if we didn't lock here we would see
        // tearing as the thread would be updating
        // the pixels while we upload them to the texture
        std::unique_lock<std::mutex> lock(mutex);
        // CODE
        condition.notify_all();
    }

    void updateNoLock()
    {
        // we don't lock here so we will see
        // tearing as the thread will update
        // the pixels while we upload them to the texture
        // CODE
        condition.notify_all();
    }

    /// This drawing function cannot be called from the thread itself because
    /// it includes OpenGL calls
    void draw()
    {
        // tex.draw(0, 0);
    }

    bool getProcessed()
    {
        return processed;
    }

protected:
    // pixels represents shared data that we aim to always access from both the
    // main thread AND this threaded object and at least from one of them for
    // writing. Therefore, we need to protect it with the mutex.
    // Otherwise it wouldn't make sense to lock.
    string metadata;
    string cmd;
    bool isProcessing = false;
    bool processed = false;

    std::condition_variable condition;
    int threadFrameNum = 0;
};
