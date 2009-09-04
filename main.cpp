#include "motion_detection.h"

int main(int argc, char** argv)
{
  // Note: Just put a stupid frame count...
  
  IplImage *motion = 0;
  CvCapture *capture = 0;
  
  if (argc == 1 || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0])))
      capture = cvCaptureFromCAM(argc == 2 ? argv[1][0] - '0' : 0);
  else if (argc == 2)
      capture = cvCaptureFromFile(argv[1]);
  
  
  
  if (capture)
  {
      cvNamedWindow("Motion", 1);
      
      while (true)
      {
          IplImage* image;
          if (!cvGrabFrame(capture))
              break;
          image = cvRetrieveFrame(capture);
          
          //printf("[%d]\n", cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT));
          
          if (image)
          {
              if (!motion)
              {
                  motion = cvCreateImage(cvSize(image->width, image->height), 8, 3);
                  cvZero(motion);
                  motion->origin = image->origin;
              }
          }
          
          MotionInfo* mi = new MotionInfo();
          
          motionDetection(image, motion, mi);
          
          delete mi;
          
          cvShowImage("Motion", motion);
          
          if (cvWaitKey(10) >= 0)
              break;
      }
      
      cvReleaseCapture(&capture);
      clean_up_images();
      cvDestroyWindow("Motion");
}

return 0;
}