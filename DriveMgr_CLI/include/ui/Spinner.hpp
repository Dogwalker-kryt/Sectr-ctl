#pragma once

#include "../DmgrLib.h"
#include <thread>
#include <chrono>
#include <atomic>

class SimpleSpinner {
private:

    static scf::array<scf::str8, 4> frames;
    static int idx;

    static void tick();

    static void done();

public:

    static void progressSpinner(std::atomic<bool> &b_done);

};

#define SPINNER(b_done) SimpleSpinner::progressSpinner(b_done);