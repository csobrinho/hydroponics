#ifndef HYDROPONICS_SIMULATION_H
#define HYDROPONICS_SIMULATION_H

#define WITH_THRESHOLD(value, threshold) ((value) + (float) (random() % (long) ((threshold) * 1024 * 2)) / 1024.f - (threshold))

#endif //HYDROPONICS_SIMULATION_H
