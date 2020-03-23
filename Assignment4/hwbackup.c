#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "ppmFile.c"
// #include <omp.h>

#define NUM_THREADS 4

int main(int argc, char const *argv[])
{
    Image* target = ImageRead("fox.ppm");
    int h, w;
    int r = atoi(argv[1]);
    h = ImageHeight(target);
    w = ImageWidth(target);
    Image* out = ImageCreate(w, h);
    printf("%d,%d",h,w);
    for (int i = 0; i <= h-1;i++) {
        for (int k = 0; k<=w-1;k++){
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
                        sum += ImageGetPixel(target, horizontal, vertical, n);
                    }
                }
                ImageSetPixel(out, k, i, n, sum/totalPixels);
                // if (i>=r & k>=r & i + r <= h & k + r <= w){
                //     int sum = 0;
                //     for (int m = i - r; m <= i + r; m ++) {
                //         for (int t = k - r; k<= k + r; k++) {
                //             sum += ImageGetPixel(target, t, m, n);
                //         }
                //     }
                // ImageSetPixel(out,k, i, n, sum/(4*r^2));
                // }
                // else if (i )
            }
            printf("Done.\n");
        }

    }

    char const * pfilename = ("out.ppm");
    ImageWrite(out, pfilename);
    // unsigned char p = ImageGetPixel(target, 1, 1, 1);
    // printf("%c", p);
    return 0;
}


#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "ppmFile.c"
#include <time.h>
#include <omp.h>

#define NUM_THREADS 8

int main(int argc, char const *argv[])
{   
    clock_t time = clock();
    Image* target = ImageRead("fox.ppm");
    int h, w;
    int r = atoi(argv[1]);
    h = ImageHeight(target);
    w = ImageWidth(target);
    Image* out = ImageCreate(w, h);
    // printf("%d,%d",h,w);

    #pragma omp parallel for num_threads(NUM_THREADS) shared(out) 
        for (int i = 0; i <= h-1;i++) {
            for (int k = 0; k<=w-1;k++){
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
                            sum += ImageGetPixel(target, horizontal, vertical, n);
                        }
                    }
                    #pragma omp critical 
                    {
                        ImageSetPixel(out, k, i, n, sum/totalPixels);
                    }
                    // if (i>=r & k>=r & i + r <= h & k + r <= w){
                    //     int sum = 0;
                    //     for (int m = i - r; m <= i + r; m ++) {
                    //         for (int t = k - r; k<= k + r; k++) {
                    //             sum += ImageGetPixel(target, t, m, n);
                    //         }
                    //     }
                    // ImageSetPixel(out,k, i, n, sum/(4*r^2));
                    // }
                }
                printf("Done.\n");
            }
    }

    ImageWrite(out, "out.ppm");
    // unsigned char p = ImageGetPixel(target, 1, 1, 1);
    // printf("%c", p);
    time = clock() - time;
    double time_taken = ((double)time)/CLOCKS_PER_SEC;
    printf("The program took %f seconds to execute", time_taken);
    return 0;
}
