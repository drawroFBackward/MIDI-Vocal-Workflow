#pragma once
#include <string>
#include <iostream>

struct Note
{
    int pitch;
    int velocity;
    int track;
    int channel;
    double startTick;
    double duration; // Duration in ticks
    Note(int p, int v, int t, int c, double s, double d) : pitch(p), velocity(v), track(t), channel(c), startTick(s), duration(d) {}
};