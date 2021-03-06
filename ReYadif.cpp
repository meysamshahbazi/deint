#include <vector>
#include <memory>
#include <string.h>

#define MIN(a,b) ((a) > (b) ? (b) : (a))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define ABS(a) ((a) > 0 ? (a) : (-(a)))

#define MIN3(a,b,c) MIN(MIN(a,b),c)
#define MAX3(a,b,c) MAX(MAX(a,b),c)

typedef unsigned char u8;
typedef short s16;


// it suppose calc_spatialDif and calc_spatialScore will be optimize
// by compiler that use SIMD instruction
void calc_spatialDif(
    unsigned char *cur_prev,    
    unsigned char *cur_next,
    s16 *dst,
    int width,
    int shift)
{
    s16 b = ABS(shift);
    cur_prev += shift;
    cur_next -= shift;
    dst += b;
    for (int x = b; x < width - b; x++)
    {
        *dst = ABS((*cur_prev) - (*cur_next));
        dst++;
        cur_prev++;        
        cur_next++;
    }
}

// dif       0 1 2 3 4 5 6 7
// dif+1     1 2 3 4 5 6 7
// dif+2     2 3 4 5 6 7
// score   s s s s s s s s
void calc_spatialScore(
    s16 *dif,
    s16 *score,
    int width)
{
    auto dif1 = dif;
    auto dif2 = dif + 1;
    auto dif3 = dif + 2;
    score += 1;
    
    for (int i = 1; i < width - 1; i++)
    {
        *score = ((*dif1) + (*dif2) + (*dif3));

        score++;
        dif1++;
        dif2++;
        dif3++;
    }
}

int spatialScoreValue_MaxWidth = 0;
std::unique_ptr<s16> spatialDifN2; // -2
std::unique_ptr<s16> spatialDifN1; // -1
std::unique_ptr<s16> spatialDif0;  //  0
std::unique_ptr<s16> spatialDifP1; //  1
std::unique_ptr<s16> spatialDifP2; //  2

std::unique_ptr<s16> spatialScoreN2; // -2
std::unique_ptr<s16> spatialScoreN1; // -1
std::unique_ptr<s16> spatialScore0;  //  0
std::unique_ptr<s16> spatialScoreP1; //  1
std::unique_ptr<s16> spatialScoreP2; //  2


void ReYadif1Row(
    int mode,
    u8 *dst,
    const u8 *prev, const u8 *cur, const u8 *next,
    int w, int step1, int parity)
{
    const u8 *prev2 = parity ? prev : cur;
    const u8 *next2 = parity ? cur : next;

    if (w > spatialScoreValue_MaxWidth)
    {
        spatialScoreValue_MaxWidth = w;
        spatialDifN2.reset(new s16[w]);
        spatialDifN1.reset(new s16[w]);
        spatialDif0.reset(new s16[w]);
        spatialDifP1.reset(new s16[w]);
        spatialDifP2.reset(new s16[w]);
        spatialScoreN2.reset(new s16[w]);
        spatialScoreN1.reset(new s16[w]);
        spatialScore0.reset(new s16[w]);
        spatialScoreP1.reset(new s16[w]);
        spatialScoreP2.reset(new s16[w]);
    }

    // pre-calculate spatial score, can be optimize by SIMD
    calc_spatialDif((u8*)cur - step1, (u8*)cur + step1, spatialDifN2.get(), w, -2);
    calc_spatialDif((u8*)cur - step1, (u8*)cur + step1, spatialDifN1.get(), w, -1);
    calc_spatialDif((u8*)cur - step1, (u8*)cur + step1, spatialDif0.get(),  w,  0);
    calc_spatialDif((u8*)cur - step1, (u8*)cur + step1, spatialDifP1.get(), w, +1);
    calc_spatialDif((u8*)cur - step1, (u8*)cur + step1, spatialDifP2.get(), w, +2);
    calc_spatialScore(spatialDifN2.get(), spatialScoreN2.get(), w);
    calc_spatialScore(spatialDifN1.get(), spatialScoreN1.get(), w);
    calc_spatialScore(spatialDif0.get(),  spatialScore0.get(),  w);
    calc_spatialScore(spatialDifP1.get(), spatialScoreP1.get(), w);
    calc_spatialScore(spatialDifP2.get(), spatialScoreP2.get(), w);


    for (int x = 3; x < w-3; x++)
    {
        int minScore = spatialScore0.get()[x];
        int spatial_pred = (cur[-step1 + 0] + cur[+step1 + 0]) / 2;
        if (minScore > spatialScoreN1.get()[x])
        {
            minScore = spatialScoreN1.get()[x];
            spatial_pred = (cur[-step1 + -1] + cur[+step1 + -1]) / 2;
            if (minScore > spatialScoreN2.get()[x])
            {
                minScore = spatialScoreN2.get()[x];
                spatial_pred = (cur[-step1 + -2] + cur[+step1 + -2]) / 2;
            }
        }
        if (minScore > spatialScoreP1.get()[x])
        {
            minScore = spatialScoreP1.get()[x];
            spatial_pred = (cur[-step1 + 1] + cur[+step1 + 1]) / 2;
            if (minScore > spatialScoreP2.get()[x])
            {
                minScore = spatialScoreP2.get()[x];
                spatial_pred = (cur[-step1 + 2] + cur[+step1 + 2]) / 2;
            }
        }

        int c = cur[-step1];
        int d = (prev2[0] + next2[0]) / 2;
        int e = cur[+step1];
        int temporal_diff0 = ABS(prev2[0] - next2[0]) / 2;
        int temporal_diff1 = (ABS(prev[-step1] - cur[-step1]) + ABS(prev[+step1] - cur[+step1])) / 2;
        int temporal_diff2 = (ABS(next[-step1] - cur[-step1]) + ABS(next[+step1] - cur[+step1])) / 2;
        int diff = MAX3(temporal_diff0, temporal_diff1, temporal_diff2);

        if (mode < 2) {
            int b = (prev2[-2 * step1] + next2[-2 * step1]) >> 1;
            int f = (prev2[+2 * step1] + next2[+2 * step1]) >> 1;
#if 1
            int a = cur[-3 * step1];
            int g = cur[+3 * step1];
            int max = MAX3(d - e, d - c, MIN3(MAX(b - c, f - e), MAX(b - c, b - a), MAX(f - g, f - e)));
            int min = MIN3(d - e, d - c, MAX3(MIN(b - c, f - e), MIN(b - c, b - a), MIN(f - g, f - e)));
#else
            int max = MAX3(d - e, d - c, MIN(b - c, f - e));
            int min = MIN3(d - e, d - c, MAX(b - c, f - e));
#endif

            diff = MAX3(diff, min, -max);
        }
        if (spatial_pred > d + diff)
            spatial_pred = d + diff;
        else if (spatial_pred < d - diff)
            spatial_pred = d - diff;

        *dst = spatial_pred;

        dst++;
        cur++;
        prev++;
        next++;
        prev2++;
        next2++;
    }
}

static void interpolate(u8 *dst, const u8 *src1, const u8 *src2, int width)
{    
    for (int i = 0; i < width; i++)
    {
        *dst = ((*src1) + (*src2)) / 2;

        dst++;
        src1++;
        src2++;
    }
}

void ReYadif1Channel(
    int mode,
    u8 *dst,
    const u8 *prev0, const u8 *cur0, const u8 *next0, int step1,
    int w, int h,
    int parity, int tff)
{
    memset(dst, 0, step1 * 2); // first two line fill blank

    dst += step1 * 2;    
    prev0 += step1 * 2;
    cur0  += step1 * 2;
    next0 += step1 * 2;
    for (int y = 2; y < h - 2; y++)
    {
        if (((y ^ parity) & 1))
            ReYadif1Row(mode, dst, prev0, cur0, next0, w, step1, (parity ^ tff));
        else
            memcpy(dst, cur0, w); // copy original

        dst += step1;
        prev0 += step1;
        cur0 += step1;
        next0 += step1;
    }

    memset(dst + ((h - 2)*step1), 0, step1 * 2); // last two line fill blank
}