#include <stdio.h>
#include <vector>
#include <stdlib.h>

#include <opencv2/opencv.hpp>
#include <opencv/cv.hpp>
#include <opencv/highgui.h>

using namespace std;
using namespace cv;

const int NUMRECT_DIM = 20;
const int NUMRECT_PIX = NUMRECT_DIM * NUMRECT_DIM;

// images for number overlay
const int OVERLAY_ROWS = 30;
const int OVERLAY_COLS = 26;

void getImage(char *image, int **puzzle);