#pragma once
#include <string>
#include <iostream>

struct Note
{
    int pitch;
    int velocity;
    double startTick;
    double duration;
    Note(int p, int v, double s, double d) : pitch(p), velocity(v), startTick(s), duration(d) {}
};