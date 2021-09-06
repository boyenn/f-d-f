// check the biome at a block position
#include "finders.h"
#include <stdio.h>
STRUCT(threadinfo_t)
{
    int start, end;
};

static void *doShit(void *data)
{
    threadinfo_t *info = (threadinfo_t *)data;

    printf("Setup thread with start %d and end %d\n", info->start, info->end);
    // Initialize a stack of biome layers that reflects the biome generation of
    // Minecraft 1.17
    LayerStack g;
    BiomeFilter filter;
    setupGenerator(&g, MC_1_17);
    uint64_t seed = 2791111690993685248LL;

    applySeed(&g, seed);

    // seeds are internally represented as unsigned 64-bit integers
    Pos pos = {0, 0}; // block position to be checked

    int stepSize = 32;
    int radius = 128;
    int wanted[] = {
        river};
    printf("a\n");

    filter = setupBiomeFilter(wanted, sizeof(wanted) / sizeof(int));
    printf("a\n");

    int entry = L_RIVER_MIX_4;
    int w = 5, h = 5;
    printf("a\n");

    int *area = allocCache(&g.layers[entry], w, h);
    printf("aaa\n");

    for (int x = info->start; x <= info->end; x += stepSize)
    {
        for (int z = info->start; z <= info->end; z += stepSize)
        {
            if (checkForBiomes(&g, entry, area, seed, x, z, w, h, filter, 1) == 0)
            {
                continue;
            }
            int count = 0;
            int sampleCount = 0;
            int preSampleCount = 0;
            for (int iz = z - radius; iz <= z + radius; iz += 40)
            {
                int64_t dx = (int)sqrt(radius * radius - (iz - z) * (iz - z));
                dx = dx;
                // printf("radius: %d, iz:%d, dx: %d \n", radius, iz, dx);

                for (int ix = x - dx; ix <= x + dx; ix += 30)
                {
                    Pos pos = {ix, iz};
                    if (getBiomeAtPos(&g, pos) == river)
                    {
                        preSampleCount++;
                        // printf("River at X: %d, Z: %d\n", ix, iz);
                    }
                }
            }
            if (preSampleCount >= 8)
            {
                for (int iz = z - radius; iz <= z + radius; iz += 10)
                {
                    int64_t dx = (int)sqrt(radius * radius - (iz - z) * (iz - z));
                    dx = dx;
                    // printf("radius: %d, iz:%d, dx: %d \n", radius, iz, dx);

                    for (int ix = x - dx; ix <= x + dx; ix += 10)
                    {
                        Pos pos = {ix, iz};
                        if (getBiomeAtPos(&g, pos) == river)
                        {
                            sampleCount++;
                            // printf("River at X: %d, Z: %d\n", ix, iz);
                        }
                    }
                }

                if (sampleCount > 80)
                {
                    printf("match\n");
                    for (int iz = z - radius; iz <= z + radius; iz++)
                    {
                        int64_t dx = (int)sqrt(radius * radius - (iz - z) * (iz - z));
                        dx = dx;
                        // printf("radius: %d, iz:%d, dx: %d \n", radius, iz, dx);

                        for (int ix = x - dx; ix <= x + dx; ix++)
                        {
                            Pos pos = {ix, iz};
                            if (getBiomeAtPos(&g, pos) == river)
                            {
                                count++;
                                // printf("River at X: %d, Z: %d\n", ix, iz);
                            }
                        }
                    }
                    printf("sampleCount: %d, preSampleCount: %d X:%d , Z:%d = %d\n", sampleCount, preSampleCount, x, z, count);
                }
            }
        }
    }
}

int main()
{
    thread_id_t *tids = (thread_id_t *)malloc(8 * sizeof(*tids));
    threadinfo_t *info = (threadinfo_t *)malloc(8 * sizeof(*info));

    int checkRange = 100000;

    int threads = 8;
    for (int i = 0; i <= threads; i++)
    {

        info[i].start = (i * (checkRange) / threads);
        info[i].end = ((i + 1) * (checkRange) / threads - 1);
        pthread_create(&tids[i], NULL, doShit, (void *)&info[i]);
    }

    for (int t = 0; t < threads; t++)
    {
        pthread_join(tids[t], NULL);
    }
    return 0;
}
