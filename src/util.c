#include <stdlib.h>
#include "util.h"

#define frand(range)	((float)rand() / (float)RAND_MAX * (range))

/* generates a sample position for sample number sidx, in the unit square
 * by recursive subdivision and jittering
 */
void calc_sample_pos_rec(int sidx, float xsz, float ysz, float *pos)
{
    static const float subpt[4][2] = {
        {-0.25, -0.25}, {0.25, -0.25}, {-0.25, 0.25}, {0.25, 0.25}
    };

    if(!sidx) {
        /* we're done, just add appropriate jitter */
        pos[0] += frand(xsz / 2.0) - xsz / 4.0;
        pos[1] += frand(ysz / 2.0) - ysz / 4.0;
        return;
    }

    /* determine which quadrant to recurse into */
    int quadrant = ((sidx - 1) % 4);
    pos[0] += subpt[quadrant][0] * xsz;
    pos[1] += subpt[quadrant][1] * ysz;

    calc_sample_pos_rec((sidx - 1) / 4, xsz / 2, ysz / 2, pos);
}
