#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> lck(_mutex);
    _condition.wait(lck);
    T received_phase = std::move(*_queue.begin());
    _queue.pop_front();
    return received_phase;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
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
    threads.emplace_back(&TrafficLight::cycleThroughPhases, this);
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // create number generator
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
