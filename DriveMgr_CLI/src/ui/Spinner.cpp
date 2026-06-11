#include "../include/ui/Spinner.hpp"
#include "../include/scf/scf_io.hpp"

std::array<std::string, 4> SimpleSpinner::frames = {"|", "/", "—", "\\"};
int SimpleSpinner::idx = 0;

void SimpleSpinner::tick() {
    scf::print_flush("\r", frames[idx++ % 4], " Running...");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

void SimpleSpinner::done() {
    scf::println(GREEN, "\r✓ ", RESET, "Done       \n");
}

void SimpleSpinner::progressSpinner(std::atomic<bool> &b_done) {
    while (!b_done) {
        
        tick();

    }

    done();
    return;
}