#pragma once
#ifndef PROCSIM_TIMER_HPP_
#define PROCSIM_TIMER_HPP_

#include "procsim/Macros.hpp"
#include "procsim/Types.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <thread>
#include <vector>

#include <boost/signals2.hpp>

PROCSIM_NAMESPACE_START

// Forward dependancies

class AsyncTimerReceiver;

/**
 * A top level class which allows an object to maintain the time cycle of other
 * AsyncTimerReceiver objects. The Timer will asynchronously update the
 * AsyncTimerReceiver objects it has bound. These objects will then work in their
 * own thread space to execute then wait for the next "tick."
 */
class PROCSIM_EXPORT Timer : public std::enable_shared_from_this<Timer>
{

public:
    typedef std::chrono::nanoseconds res_type;
    typedef uint64_t tick_count_t;

    // FIXME Docs
    enum class SignalState : int16_t
    {
        X    = std::numeric_limits<int16_t>::min(), // lowest neg
        Z    = std::numeric_limits<int16_t>::max(), // highest pos
        LOW  = 0,
        HIGH = 5
    };

    // FIXME Docs
    typedef std::function<void(const SignalState, const tick_count_t)> OnSignalChange;

    /**
     * \brief Creates a Timer object with the specified resolution.
     * \param resolution The period of the `Timer`
     */
    Timer(const res_type& resolution);

    /** \brief Deallocates resources, forcibly stops all related threads if
     * necessary
     */
    virtual ~Timer();

    void start();

    /*!
     * \brief Pauses the timer after the current `tick` completes.
     */
    void pause();

    /*!
     * \brief Returns true if paused.
     */
    bool isPaused() const;

    void setStepFor(const int32_t steps = 1);

    int32_t stepFor() const;

    /*!
     * \brief Attempts to elequently stop all attached objects by setting them
     * inactive followed by std::thread::join. It then joins it's inacting
     * thread.
     */
    void stop();

    // tick
    void tick();

    /**
     * \brief Returns the period of the `Timer` object.
     * \return The period of the `Timer`.
     */
    res_type resolution() const;

    /*!
     * \brief Get the number of ticks that have occured.
     * \returns Count of ticks, total running time is `~(tickCount() * resolution())`
     */
    const tick_count_t tickCount() const;

    /*!
     * \brief Gets the current running time, accurate with pausing.
     * \returns `std::chrono::duration` based on the templated parameter.
     */
    template <class DurType>
    DurType runningTime() const
    {
        // FIXME This only lets you pause once...
        if (_active)
        {
            auto now = std::chrono::system_clock::now();
            DurType dur;
            if (_paused)
            {
                dur = std::chrono::duration_cast<DurType>(_tpPauseMod - _tpRunning);
            }
            else
            {
                dur = std::chrono::duration_cast<DurType>(now - _tpRunning);
            }
            return dur;
        }
        else
        {
            return DurType(0);
        }
    }

    template <class TD>
    std::shared_ptr<TD> makeDependant(const Timer::OnSignalChange&& tickFunc)
    {
        auto ptr = std::make_shared<TD>(shared_from_this(), tickFunc);

        bindDep(ptr);

        return ptr;
    }

    template <class TD, class... U>
    std::shared_ptr<TD> makeDependant(const Timer::OnSignalChange&& tickFunc, U&&... u)
    {
        auto ptr = std::make_shared<TD>(shared_from_this(), tickFunc, std::forward<U>(u)...);

        bindDep(ptr);

        return ptr;
    }

    /*!
     * \brief Get the signal state of the timer.
     * \return Current signal state
     */
    const SignalState signalState() const;

    /*!
     * \brief Causes the currently executing thread to wait for the next
     *      notification from the `std::condition_variable` stored within
     *      the Timer.
     * \param lock The lock to wait on
     * \param LockType The class of the lock, not necessary to specify
     */
    template <class Mutex>
    std::tuple<const SignalState, tick_count_t> waitForTick(std::unique_lock<Mutex>& lock)
    {
        // Get the current amount of ticks
        uint64_t tc = tickCount();

        while (_active && tc == tickCount())
        {
            // it should *never* take longer than `resolution()`
            _cvTick.wait_for(lock, resolution());
        }

        return std::make_tuple(signalState(), tickCount());
    }

    /*!
     * \brief Get the number of dependants currently bound to this \c Timer.
     * \return Count of dependants.
     */
    inline const size_t dependantCount()
    {
        std::lock_guard<std::recursive_mutex> lock(mtx_depObjs);

        cleanDeps();
        return _depObjs.size();
    }

private:
    /*!
     * \brief Binds a AsyncTimerReceiver object to this Timer. It will be called
     * to run when necessary.
     * \param dep AsyncTimerReceiver object to run on Tick
     */
    inline void bindDep(const std::weak_ptr<PROCSIM_N::AsyncTimerReceiver> dep)
    {
        std::lock_guard<std::recursive_mutex> lock(mtx_depObjs);

        if (!dep.expired())
        {
            // Don't worry about multiple inserts, it's a set
            _depObjs.insert(dep);
        }

        cleanDeps();
    }

    /*!
     * \brief Removes a AsyncTimerReceiver object from the timer. This stops its
     * execution until re-added. Additionally, any weak_ptrs that are expired are removed.
     */
    inline void removeDep(const std::weak_ptr<PROCSIM_N::AsyncTimerReceiver> dep)
    {
        std::lock_guard<std::recursive_mutex> lock(mtx_depObjs);

        _depObjs.erase(dep);
        cleanDeps();
    }

    /*!
     * \brief Remove any "dead" dependant weak_ptrs.
     */
    inline void cleanDeps()
    {
        std::lock_guard<std::recursive_mutex> lock(mtx_depObjs);

        for (auto it = _depObjs.begin(); it != _depObjs.end();)
        {
            if (!it->lock())
            {
                it = _depObjs.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    std::atomic_bool _active; //!< If set to false, the thread will stop at its ideal point

    std::unique_ptr<std::thread> _tickThread; //!< "Main" thread executing to "tick" on its objects

    res_type _resolution; //!< Time between consecutive ticks

    // TIME RUNNING INFORMATION
    std::chrono::system_clock::time_point _tpRunning;  //!< Used for accurate time running metric
    std::chrono::system_clock::time_point _tpPauseMod; //!< Used to compute accurate pausing

    // mutable keyword used because there is no reason to not be able to make waitForTick const.
    mutable std::condition_variable _cvTick; // no need to have lock because it's already synchronized

    std::atomic<tick_count_t> _tickCount; // The number of ticks since starting

    mutable std::shared_mutex mtx_signalState;
    SignalState _signalState;

    mutable std::recursive_mutex mtx_depObjs; //!< Used to lock the `depObjs` vector
    std::set<std::weak_ptr<AsyncTimerReceiver>, std::owner_less<std::weak_ptr<AsyncTimerReceiver>>>
        _depObjs; //!< The timer depenant objects

    mutable std::mutex mtx_Step;
    int32_t _step;

    std::condition_variable _cvPause;
    std::mutex mtx_Pause, mtx_PauseWait; //!< Needed for condition_variable
    bool _paused;                        //!< If true, the timer will wait
};

PROCSIM_NAMESPACE_END

#endif /* PROCSIM_TIMER_HPP_ */
