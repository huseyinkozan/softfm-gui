#ifndef MAIN_H
#define MAIN_H

inline int    mhz_to_khz(double freqAsMhz) { return freqAsMhz * 1000; }
inline double khz_to_mhz(int freqAsKhz)    { return (double) freqAsKhz / 1000; }

#endif // MAIN_H
