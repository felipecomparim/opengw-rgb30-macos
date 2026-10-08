#pragma once
#ifndef _WIN32
#include <SDL.h>
namespace profile {
enum Part { GRID_STEP, PARTICLE_STEP, GRID_DRAW, PARTICLE_DRAW, SIMULATION, RENDER, SWAP, COUNT };
extern Uint64 elapsed[COUNT];
extern Uint64 calls[COUNT];
struct Scope {
    Part part;
    Uint64 start;
    explicit Scope(Part p) : part(p), start(SDL_GetPerformanceCounter()) {}
    ~Scope() { elapsed[part] += SDL_GetPerformanceCounter() - start; ++calls[part]; }
};
}
#endif
