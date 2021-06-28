#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> lck(_mutex);
    _condition.wait(lck);
    T received_phase = std::move(*_queue.begin());
    _queue.pop_front();
    return received_phase;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lck(_mutex);
    _queue.push_back(msg);
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true) {
        auto lightPhase = _messages.receive();
        if (lightPhase == TrafficLightPhase::green) {
            break;
        }
    }
}

TrafficLight::TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(&TrafficLight::cycleThroughPhases, this);
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> number_generator(4.0, 6.0);
    double cycle_duration = number_generator(mt);

    // initialize chrono variables for looping
    using namespace std::chrono_literals;  // so we can do 1ms
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end-start);

    while (true) {
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::seconds>(end-start);
        if ( duration.count() > cycle_duration ) {
            // toggle traffic light phase
            _currentPhase = _currentPhase == TrafficLightPhase::red ? TrafficLightPhase::green : TrafficLightPhase::red;
            _messages.send(std::move(_currentPhase));
            start = std::chrono::system_clock::now();
            cycle_duration = number_generator(mt);
        }
        std::this_thread::sleep_for(1ms);
    }
}
