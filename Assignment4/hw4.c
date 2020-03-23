#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "ppmFile.c"
#include <omp.h>

#define NUM_THREADS 512

int main(int argc, char const *argv[])
{   
    Image* file = ImageRead(argv[2]);
    int h, w;
    int r = atoi(argv[1]);
    h = ImageHeight(file);
    w = ImageWidth(file);
    Image* out = ImageCreate(w, h);
    // printf("%d,%d",h,w);
    int block_height = h/(NUM_THREADS-1);
    #pragma omp parallel for num_threads(NUM_THREADS) shared(out)
    {   
        int my_rank = omp_get_thread_num();
        // for (int i = 0; i <= h-1;i++) {
        //     for (int k = 0; k<=w-1;k++){
        for(int k = 0; k < w; k++){
            for(int i = my_rank*block_height; i < h && i < (my_rank+1)*block_height; i++){
                printf("Working on %d, %d\n", k, i);
                int left, right, up, down;
                left = k - r;
                right = k + r;
                up = i - r;
                down = i + r;
                if (left < 0) {
                    left = 0;
                }
                if (right > w) {
                    right = w;
                }
                if (up < 0) {
                    up = 0;
                }
                if (down > h) {
                    down = h;
                }
                int totalPixels = (right - left)*(down - up);
                for (int n = 0; n <= 2; n++) {
                    int sum = 0;
                    for (int vertical = up; vertical < down; vertical ++){
                        for (int horizontal = left; horizontal < right; horizontal ++ ) {
                            sum += ImageGetPixel(file, horizontal, vertical, n);
                        }
                    }
                    #pragma omp critical 
                    {
                        ImageSetPixel(out, k, i, n, sum/totalPixels);
                    }
                }
                printf("Done.\n");
            }
        }
    }
    ImageWrite(out, argv[3]);
    return 0;
}
