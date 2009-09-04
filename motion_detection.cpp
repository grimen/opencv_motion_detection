#include "motion_detection.h"

// Calculate orientation angle for a specified image segment.
int calculate_orientation(CvRect rect, IplImage* silhouette)
{
  double motion_angle;
  
  cvSetImageROI(mhi, rect);
  cvSetImageROI(orientation, rect);
  cvSetImageROI(orientation_mask, rect);
  cvSetImageROI(silhouette, rect);
  
  motion_angle = 360.0 - cvCalcGlobalOrientation(orientation, orientation_mask, mhi, timestamp, MHI_DURATION);
  
  cvResetImageROI(mhi);
  cvResetImageROI(orientation);
  cvResetImageROI(orientation_mask);
  cvResetImageROI(silhouette);
  
  return motion_angle;
}

// Draw clock that illustrates motion direction.
void draw_orientation(IplImage* destination_image, CvRect* rect, int angle, int magnitude, CvScalar color, bool show_direction)
{
  CvPoint center = cvPoint((rect->x + rect->width/2),
                    (rect->y + rect->height/2));
  
  cvCircle(destination_image, center, cvRound(magnitude), color, 2, CV_AA, 0);
  if (show_direction)
  {
  cvLine(destination_image, center, cvPoint(
    cvRound(center.x + magnitude * cos(angle * CV_PI / 180)),
    cvRound(center.y - magnitude * sin(angle * CV_PI / 180))
    ), color, 2, CV_AA, 0);
  }
}

bool intersect_rect_ex(CvRect* r1, CvRect* r2, CvRect* r3)
{
  bool intersects = intersect_rect(r1, r2);
  
  if (intersects)
  {
    r3->x = std::max(r1->x, r2->x);
    r3->y = std::max(r1->y, r2->y);
    r3->width = std::min(r1->x + r1->width, r2->x + r2->width);
    r3->height = std::min(r1->y + r1->height, r2->y + r2->height);
  }
  else
  {
    r3->x = 0;
    r3->y = 0;
    r3->width = 0;
    r3->height = 0;
  }
  return intersects;
}

// Check if two rectangles are intersecting.
bool intersect_rect(CvRect* r1, CvRect* r2)
{
  return ! (r2->x > (r1->x + r1->width)
      || (r2->x + r2->width) < r1->x
      || r2->y > (r1->y + r1->height)
      || (r2->y + r2->height) < r1->y
      );
}

// Check if a specified image segment contains any motion features.
bool contains_motion(CvRect* motion_segment_rect, CvSeq* motion_feature_sequence)
{
  bool intersects = false;
  int area = 0;
  
  CvRect* motion_feature_rect;
  
  for(int i = 0; i < motion_feature_sequence->total; i++)
  {
    motion_feature_rect = &((CvConnectedComp*)cvGetSeqElem(motion_feature_sequence, i))->rect;
    //area = motion_feature_rect->width * motion_feature_rect->height;
    
    //printf("%d\n", area);
    
    CvRect* intersection_area = &CvRect();
    
    //if (area > MIN_MOTION_FEATURE_AREA)
    intersects = intersect_rect_ex(motion_segment_rect, motion_feature_rect, intersection_area);
    
    
    
    if (intersects)
      area += intersection_area->width * intersection_area->height;
    
    //result = intersect_rect(motion_segment_rect, motion_feature_rect);
    
    //if (result == true) break;
  }
  
  return area > 0;
}

long calculate_motion(CvRect* motion_segment_rect, CvSeq* motion_feature_sequence)
{
  bool intersects = false;
  long area = 0;
  
  CvRect* motion_feature_rect;
  
  for(int i = 0; i < motion_feature_sequence->total; i++)
  {
    motion_feature_rect = &((CvConnectedComp*)cvGetSeqElem(motion_feature_sequence, i))->rect;
    
    CvRect* intersection_area = &CvRect();
    intersects = intersect_rect_ex(motion_segment_rect, motion_feature_rect, intersection_area);
    
    
    
    if (intersects)
      area += intersection_area->width * intersection_area->height;
  }
  
  //printf("area: %i, ", area);
  //fflush(stdout);
  
  return area;
}

// Clean up allocaated resources.
void clean_up_images()
{
  for(int i = 0; i < N; i++) {
    cvReleaseImage(&image_buffer[i]);
  }
  
  cvReleaseImage(&mhi);
  cvReleaseImage(&silhouette);
  cvReleaseImage(&orientation);
  cvReleaseImage(&orientation_mask);
  cvReleaseImage(&segment_mask);
  
  if (storage)
    cvClearMemStorage(storage);
}

// Initialize heavily used images (once).
void initialize_images(CvSize image_size)
{
  // allocate images at the beginning or
  // reallocate them if the frame image_size is changed
  if (!mhi || mhi->width != image_size.width || mhi->height != image_size.height) {
      if (image_buffer == 0) {
          image_buffer = (IplImage**)malloc(N*sizeof(image_buffer[0]));
          memset(image_buffer, 0, N*sizeof(image_buffer[0]));
      }
      
      clean_up_images();
      
      for(int i = 0; i < N; i++) {
          image_buffer[i] = cvCreateImage(image_size, IPL_DEPTH_8U, 1);
          cvZero(image_buffer[i]);
      }
      
      mhi = cvCreateImage(image_size, IPL_DEPTH_32F, 1);
      cvZero(mhi); // clear MHI at the beginning
      silhouette = cvCreateImage(image_size, IPL_DEPTH_32F, 1);
      orientation = cvCreateImage(image_size, IPL_DEPTH_32F, 1);
      segment_mask = cvCreateImage(image_size, IPL_DEPTH_32F, 1);
      orientation_mask = cvCreateImage(image_size, IPL_DEPTH_8U, 1);
  }
}

// Update Motion History Image: Calculate motion features and orientation.
void motionDetection(IplImage* image, IplImage* destination_image, MotionInfo* motionInfo)
{
    double timestamp = (double)clock()/CLOCKS_PER_SEC; // get current time in seconds
    CvSize image_size = cvSize(image->width, image->height); // get current frame image_size
    int previous_frame_index = last_index, current_frame_index;
    
    initialize_images(image_size);
    
    cvCvtColor(image, image_buffer[last_index], CV_BGR2GRAY); // convert frame to grayscale
    
    current_frame_index = (last_index + 1) % N; // index of (last_index - (N-1))th frame
    last_index = current_frame_index;
    
    silhouette = image_buffer[current_frame_index];
    
    cvAbsDiff(image_buffer[previous_frame_index], image_buffer[current_frame_index], silhouette); // Get difference between frames
    cvThreshold(silhouette, silhouette, DIFFERENCE_THRESHOLD, 1, CV_THRESH_BINARY); // Add threshold
    //cvDilate(silhouette, silhouette, 0, 18);
    //cvErode(silhouette, silhouette, 0, 10);
    
    cvUpdateMotionHistory(silhouette, mhi, timestamp, MHI_DURATION); // Update MHI
    
    // Convert MHI to blue 8U image
    cvCvtScale(mhi, orientation_mask, 255./MHI_DURATION, (MHI_DURATION - timestamp)*255./MHI_DURATION);
    
    if (destination_image) {
      cvZero(destination_image);
      cvCvtPlaneToPix(orientation_mask, 0, 0, 0, destination_image);
    }
    
    // Calculate motion gradient orientation and valid orientation mask
    cvCalcMotionGradient(mhi, orientation_mask, orientation, MAX_TIME_DELTA, MIN_TIME_DELTA, 3);
    
    // motion_feature_sequence = extract_motion_features();
    if(!storage)
        storage = cvCreateMemStorage(0);
    else
        cvClearMemStorage(storage);
    
    CvSeq* motion_feature_sequence = cvSegmentMotion(mhi, segment_mask, storage, timestamp, MAX_TIME_DELTA);
    
    int SEGMENT_WIDTH = image_size.width / MAX_SEGMENTS_X;
    int SEGMENT_HEIGHT = image_size.height / MAX_SEGMENTS_Y;
    
    // Global motion
    CvRect global_motion_segment = cvRect(0, 0, image_size.width, image_size.height);
    motionInfo->global_angle = calculate_orientation(global_motion_segment, silhouette);
    
    if (destination_image)
      draw_orientation(destination_image, &global_motion_segment, motionInfo->global_angle, 100, CV_RGB(0, 255, 0), true);
    
    long area = 0;
    long totalArea = 0;
    int totalMovingSegments = 0;
    bool hasValidMovement = false;
    CvRect segmentRect;
    
    // Segmented motion
    for(int x = 0; x < MAX_SEGMENTS_X; x++)
    {
      for(int y = 0; y < MAX_SEGMENTS_Y; y++)
      {
        segmentRect = cvRect(x * SEGMENT_WIDTH, y * SEGMENT_HEIGHT, SEGMENT_WIDTH, SEGMENT_HEIGHT);
        area = calculate_motion(&segmentRect, motion_feature_sequence);
        hasValidMovement = (area > MIN_MOTION_FEATURE_AREA);
        
        motionInfo->segment_motion_areas[x][y] = area;
        motionInfo->segment_movements[x][y] = hasValidMovement;
        motionInfo->segment_angles[x][y] = calculate_orientation(segmentRect, silhouette);
        
        totalArea += area;
        totalMovingSegments += (area > MIN_MOTION_FEATURE_AREA);
        
        //printf("%i, ", area);
        //fflush(stdout);
        
        if (hasValidMovement)
          if (destination_image)
            draw_orientation(destination_image, &segmentRect, motionInfo->segment_angles[x][y], 20, CV_RGB(255, 0, 0), true);
      }
    }
    motionInfo->total_motion_area = totalArea;
    motionInfo->total_segments_with_movements = totalMovingSegments;
    motionInfo->SEGMENTS_X = MAX_SEGMENTS_X;
    motionInfo->SEGMENTS_Y = MAX_SEGMENTS_Y;
    
    printf("%i, %f\n", totalArea, (float)totalArea / (float)(image_size.width*image_size.height));
    //fflush(stdout);
}