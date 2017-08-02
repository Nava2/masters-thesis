
#pragma once
#ifndef PROCSIM_TIME_ASYNCTIMERRECEIVER_HPP_
#define PROCSIM_TIME_ASYNCTIMERRECEIVER_HPP_

#include "procsim/time/Timer.hpp"

#include "procsim/Types.hpp"

#include <memory>
#include <thread>

#include <boost/signals2.hpp>

PROCSIM_NAMESPACE_START

/**
 * An object which relies on an external timer to execute. This is done to
 * better simulate and possibly underclock some operations within the
 * simulator. Most objects will be bound to the main CPU, but may have other
 * clocks associated.
 */
class PROCSIM_EXPORT AsyncTimerReceiver
{
public:
    /**
     * \brief Creates a new AsyncTimerReceiver object.
     * The slot format must be void(void);.
     * \warning The Timer object must be bound after constructing. Use
     *     `get_ptr()`.
     */
    AsyncTimerReceiver(const std::shared_ptr<PROCSIM_N::Timer> timer, const Timer::OnSignalChange& tickFunc);

    virtual ~AsyncTimerReceiver();

    void start();

    void stop();

    inline const bool running() const { return _active; }

private:
    /** \brief Executes a smart waiting loop while waiting for the next `tick`
     * to arrive to execute the next action.
     */
    void _thread_run();

    std::mutex mtx_Tick; //!< Mutex for locking and unlocking a timer, locked while waiting for a tick

    std::unique_ptr<std::thread> _thread;

    const std::shared_ptr<Timer> _timer; //!< Reference to the Timer

    const Timer::OnSignalChange _onSignal;

    std::atomic_bool _active;
};

PROCSIM_NAMESPACE_END

#endif // PROCSIM_TIME_ASYNCTIMERRECEIVER_HPP_
