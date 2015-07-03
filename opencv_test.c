#include <cv.h>
#include <highgui.h>

int main () {
    cvNamedWindow ("test", CV_WINDOW_AUTOSIZE);
    cvWaitKey (0);
    cvDestroyWindow ("test");
    return 0;
}
