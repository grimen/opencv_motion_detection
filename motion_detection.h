#ifndef MOTION_DETECTION
#define MOTION_DETECTION

#include <cv.h>
#include <highgui.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <algorithm>

#define MAX_SEGMENTS_X 20
#define MAX_SEGMENTS_Y 20
#define MIN_MOTION_FEATURE_AREA 100

typedef struct 
{
  int SEGMENTS_X;
  int SEGMENTS_Y;
  
  int global_angle;
  int segment_angles[MAX_SEGMENTS_X][MAX_SEGMENTS_Y];
  
  //CvRect segment_rects[MAX_SEGMENTS_X][MAX_SEGMENTS_Y];
  
  int total_segments_with_movements;
  int segment_movements[MAX_SEGMENTS_X][MAX_SEGMENTS_Y];
  
  double total_motion_area;
  long segment_motion_areas[MAX_SEGMENTS_X][MAX_SEGMENTS_Y];
} MotionInfo;

// Tracking parameters (in seconds)
const double MHI_DURATION = 1.0;
const double MAX_TIME_DELTA = 0.5;
const double MIN_TIME_DELTA = 0.1;

// Number of cyclic frame image_bufferfer used for motion detection (depend on FPS)
const int N = 2;
const int DIFFERENCE_THRESHOLD = 40;

// Image image_buffer
static IplImage **image_buffer = 0;
static int last_index = 0;
static double timestamp = 0;

// Temporary images
static IplImage *mhi = 0;
static IplImage *silhouette = 0; 
static IplImage *orientation = 0;
static IplImage *orientation_mask = 0;
static IplImage *segment_mask = 0;

// Temporary storage
static CvMemStorage *storage = 0;

int calculate_orientation(CvRect rect, IplImage* silhouette);
void draw_orientation(IplImage* dst, CvRect rect, int angle, int magnitude, CvScalar color, bool show_direction);
void clean_up_images();
void initialize_images(CvSize image_size);
void motionDetection(IplImage* image, IplImage* destination_image, MotionInfo* motionInfo);
bool contains_motion(CvRect* motion_segment_rect, CvSeq* motion_feature_sequence);
long calculate_motion(CvRect* motion_segment_rect, CvSeq* motion_feature_sequence);
bool intersect_rect(CvRect* r1, CvRect* r2);
bool intersect_rect_ex(CvRect* r1, CvRect* r2, CvRect* r3);

#endif //MOTION_DETECTION