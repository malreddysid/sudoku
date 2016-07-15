#include "getImage.h"

void drawLine(Vec2f line, Mat &img, Scalar rgb = CV_RGB(0,0,255))
{
    if(line[1]!=0)
    {
        float m = -1/tan(line[1]);
        
        float c = line[0]/sin(line[1]);
        
        cv::line(img, Point(0, c), Point(img.size().width, m*img.size().width+c), rgb);
    }
    else
    {
        cv::line(img, Point(line[0], 0), Point(line[0], img.size().height), rgb);
    }
    
}

void mergeRelatedLines(vector<Vec2f> *lines, Mat &img)
{
    vector<Vec2f>::iterator current;
    for(current=lines->begin();current!=lines->end();current++)
    {
        if((*current)[0]==0 && (*current)[1]==-100) continue;
        float p1 = (*current)[0];
        float theta1 = (*current)[1];
        Point pt1current, pt2current;
        if(theta1>CV_PI*45/180 && theta1<CV_PI*135/180)
        {
            pt1current.x=0;
            
            pt1current.y = p1/sin(theta1);
            
            pt2current.x=img.size().width;
            pt2current.y=-pt2current.x/tan(theta1) + p1/sin(theta1);
        }
        else
        {
            pt1current.y=0;
            
            pt1current.x=p1/cos(theta1);
            
            pt2current.y=img.size().height;
            pt2current.x=-pt2current.y/tan(theta1) + p1/cos(theta1);
            
        }
        vector<Vec2f>::iterator    pos;
        for(pos=lines->begin();pos!=lines->end();pos++)
        {
            if(*current==*pos) continue;
            if(fabs((*pos)[0]-(*current)[0])<20 && fabs((*pos)[1]-(*current)[1])<CV_PI*10/180)
            {
                float p = (*pos)[0];
                float theta = (*pos)[1];
                Point pt1, pt2;
                if((*pos)[1]>CV_PI*45/180 && (*pos)[1]<CV_PI*135/180)
                {
                    pt1.x=0;
                    pt1.y = p/sin(theta);
                    pt2.x=img.size().width;
                    pt2.y=-pt2.x/tan(theta) + p/sin(theta);
                }
                else
                {
                    pt1.y=0;
                    pt1.x=p/cos(theta);
                    pt2.y=img.size().height;
                    pt2.x=-pt2.y/tan(theta) + p/cos(theta);
                }
                if(((double)(pt1.x-pt1current.x)*(pt1.x-pt1current.x) + (pt1.y-pt1current.y)*(pt1.y-pt1current.y)<64*64) &&
                   ((double)(pt2.x-pt2current.x)*(pt2.x-pt2current.x) + (pt2.y-pt2current.y)*(pt2.y-pt2current.y)<64*64))
                {
                    // Merge the two
                    (*current)[0] = ((*current)[0]+(*pos)[0])/2;
                    
                    (*current)[1] = ((*current)[1]+(*pos)[1])/2;
                    
                    (*pos)[0]=0;
                    (*pos)[1]=-100;
                }
            }
        }
    }
}

void getImage(char *image_file, int **puzzle)
{
    vector<Mat> overlays(9);
    
    overlays[0] = imread("1.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    overlays[1] = imread("2.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    overlays[2] = imread("3.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    overlays[3] = imread("4.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    overlays[4] = imread("5.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    overlays[5] = imread("6.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    overlays[6] = imread("7.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    overlays[7] = imread("8.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    overlays[8] = imread("9.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    
    Mat image = imread(image_file, CV_LOAD_IMAGE_GRAYSCALE);
    GaussianBlur(image, image, Size(5,5), 0);
    Mat thresholded_image;
    adaptiveThreshold(image, thresholded_image, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 7, 5);
    bitwise_not(thresholded_image, thresholded_image);
    Mat dilated_image;
    Mat element = (Mat_<uchar>(3,3) << 0,1,0,1,1,1,0,1,0);
    dilate(thresholded_image, dilated_image, element);
    
    // Use floodfill to idenify the borders
    
    int count = 0;
    int max = -1;
    Point maxPt;
    for(int i = 0; i < dilated_image.size().height; i++)
    {
        uchar *row = dilated_image.ptr(i);
        for(int j = 0; j < dilated_image.size().width; j++)
        {
            if(row[j] >= 128)
            {
                int area = floodFill(dilated_image, Point(j, i), CV_RGB(0,0,64));
                if(area > max)
                {
                    max = area;
                    maxPt = Point(j,i);
                }
            }
        }
    }
    
    floodFill(dilated_image, maxPt, CV_RGB(255,255,255));
    
    for(int i = 0; i < dilated_image.size().height; i++)
    {
        uchar *row = dilated_image.ptr(i);
        for(int j = 0; j < dilated_image.size().width; j++)
        {
            if(row[j] == 64 && i != maxPt.x && j != maxPt.y)
            {
                int area = floodFill(dilated_image, Point(j,i), CV_RGB(0,0,0));
            }
        }
    }
    
    Mat eroded_image, lined_image;
    erode(dilated_image, eroded_image, element);
    erode(dilated_image, lined_image, element);
    
    // Detect the lines
    vector<Vec2f> lines;
    HoughLines(eroded_image, lines, 1, CV_PI/180, 200);
    mergeRelatedLines(&lines, lined_image); // Add this line
    for(int i=0;i<lines.size();i++)
    {
        drawLine(lines[i], eroded_image, CV_RGB(0,0,128));
    }
    
    
    // Detecting the extremes
    Vec2f topEdge = Vec2f(1000,1000);    double topYIntercept=100000, topXIntercept=0;
    Vec2f bottomEdge = Vec2f(-1000,-1000);        double bottomYIntercept=0, bottomXIntercept=0;
    Vec2f leftEdge = Vec2f(1000,1000);    double leftXIntercept=100000, leftYIntercept=0;
    Vec2f rightEdge = Vec2f(-1000,-1000);        double rightXIntercept=0, rightYIntercept=0;
    for(int i=0;i<lines.size();i++)
    {
        
        Vec2f current = lines[i];
        
        float p=current[0];
        
        float theta=current[1];
        
        if(p==0 && theta==-100)
            continue;
        double xIntercept, yIntercept;
        xIntercept = p/cos(theta);
        yIntercept = p/(cos(theta)*sin(theta));
        if(theta>CV_PI*80/180 && theta<CV_PI*100/180)
        {
            if(p<topEdge[0])
                
                topEdge = current;
            
            if(p>bottomEdge[0])
                bottomEdge = current;
        }
        else if(theta<CV_PI*10/180 || theta>CV_PI*170/180)
        {
            if(xIntercept>rightXIntercept)
            {
                rightEdge = current;
                rightXIntercept = xIntercept;
            }
            else if(xIntercept<=leftXIntercept)
            {
                leftEdge = current;
                leftXIntercept = xIntercept;
            }
        }
    }
    drawLine(topEdge, lined_image, CV_RGB(0,0,0));
    drawLine(bottomEdge, lined_image, CV_RGB(0,0,0));
    drawLine(leftEdge, lined_image, CV_RGB(0,0,0));
    drawLine(rightEdge, lined_image, CV_RGB(0,0,0));
    
    Point left1, left2, right1, right2, bottom1, bottom2, top1, top2;
    
    int height=lined_image.size().height;
    
    int width=lined_image.size().width;
    
    if(leftEdge[1]!=0)
    {
        left1.x=0;        left1.y=leftEdge[0]/sin(leftEdge[1]);
        left2.x=width;    left2.y=-left2.x/tan(leftEdge[1]) + left1.y;
    }
    else
    {
        left1.y=0;        left1.x=leftEdge[0]/cos(leftEdge[1]);
        left2.y=height;    left2.x=left1.x - height*tan(leftEdge[1]);
        
    }
    
    if(rightEdge[1]!=0)
    {
        right1.x=0;        right1.y=rightEdge[0]/sin(rightEdge[1]);
        right2.x=width;    right2.y=-right2.x/tan(rightEdge[1]) + right1.y;
    }
    else
    {
        right1.y=0;        right1.x=rightEdge[0]/cos(rightEdge[1]);
        right2.y=height;    right2.x=right1.x - height*tan(rightEdge[1]);
        
    }
    
    bottom1.x=0;    bottom1.y=bottomEdge[0]/sin(bottomEdge[1]);
    
    bottom2.x=width;bottom2.y=-bottom2.x/tan(bottomEdge[1]) + bottom1.y;
    
    top1.x=0;        top1.y=topEdge[0]/sin(topEdge[1]);
    top2.x=width;    top2.y=-top2.x/tan(topEdge[1]) + top1.y;
    
    // Next, we find the intersection of  these four lines
    double leftA = left2.y-left1.y;
    double leftB = left1.x-left2.x;
    
    double leftC = leftA*left1.x + leftB*left1.y;
    
    double rightA = right2.y-right1.y;
    double rightB = right1.x-right2.x;
    
    double rightC = rightA*right1.x + rightB*right1.y;
    
    double topA = top2.y-top1.y;
    double topB = top1.x-top2.x;
    
    double topC = topA*top1.x + topB*top1.y;
    
    double bottomA = bottom2.y-bottom1.y;
    double bottomB = bottom1.x-bottom2.x;
    
    double bottomC = bottomA*bottom1.x + bottomB*bottom1.y;
    
    // Intersection of left and top
    double detTopLeft = leftA*topB - leftB*topA;
    
    CvPoint ptTopLeft = cvPoint((topB*leftC - leftB*topC)/detTopLeft, (leftA*topC - topA*leftC)/detTopLeft);
    
    // Intersection of top and right
    double detTopRight = rightA*topB - rightB*topA;
    
    CvPoint ptTopRight = cvPoint((topB*rightC-rightB*topC)/detTopRight, (rightA*topC-topA*rightC)/detTopRight);
    
    // Intersection of right and bottom
    double detBottomRight = rightA*bottomB - rightB*bottomA;
    CvPoint ptBottomRight = cvPoint((bottomB*rightC-rightB*bottomC)/detBottomRight, (rightA*bottomC-bottomA*rightC)/detBottomRight);// Intersection of bottom and left
    double detBottomLeft = leftA*bottomB-leftB*bottomA;
    CvPoint ptBottomLeft = cvPoint((bottomB*leftC-leftB*bottomC)/detBottomLeft, (leftA*bottomC-bottomA*leftC)/detBottomLeft);
    
    int maxLength = (ptBottomLeft.x-ptBottomRight.x)*(ptBottomLeft.x-ptBottomRight.x) + (ptBottomLeft.y-ptBottomRight.y)*(ptBottomLeft.y-ptBottomRight.y);
    int temp = (ptTopRight.x-ptBottomRight.x)*(ptTopRight.x-ptBottomRight.x) + (ptTopRight.y-ptBottomRight.y)*(ptTopRight.y-ptBottomRight.y);
    
    if(temp>maxLength) maxLength = temp;
    
    temp = (ptTopRight.x-ptTopLeft.x)*(ptTopRight.x-ptTopLeft.x) + (ptTopRight.y-ptTopLeft.y)*(ptTopRight.y-ptTopLeft.y);
    
    if(temp>maxLength) maxLength = temp;
    
    temp = (ptBottomLeft.x-ptTopLeft.x)*(ptBottomLeft.x-ptTopLeft.x) + (ptBottomLeft.y-ptTopLeft.y)*(ptBottomLeft.y-ptTopLeft.y);
    
    if(temp>maxLength) maxLength = temp;
    
    maxLength = sqrt((double)maxLength);
    
    Point2f src[4], dst[4];
    src[0] = ptTopLeft;            dst[0] = Point2f(0,0);
    src[1] = ptTopRight;        dst[1] = Point2f(maxLength-1, 0);
    src[2] = ptBottomRight;        dst[2] = Point2f(maxLength-1, maxLength-1);
    src[3] = ptBottomLeft;        dst[3] = Point2f(0, maxLength-1);
    
    // Warp the image
    
    Mat undistorted = Mat(Size(maxLength, maxLength), CV_8UC1);
    cv::warpPerspective(image, undistorted, cv::getPerspectiveTransform(src, dst), Size(maxLength, maxLength));
    
    // Now repeat the floodfill to identify and remove the borderes. Then we are left with only numbers. It hepls with the accuracy.
    Mat undistortedThreshed = undistorted.clone();
    adaptiveThreshold(undistorted, undistortedThreshed, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY_INV, 101, 1);
    erode(undistortedThreshed, undistortedThreshed, element);
    
    
    dilate(undistortedThreshed, dilated_image, element);
    
    count = 0;
    max = -1;
    for(int i = 0; i < dilated_image.size().height; i++)
    {
        uchar *row = dilated_image.ptr(i);
        for(int j = 0; j < dilated_image.size().width; j++)
        {
            if(row[j] >= 128)
            {
                int area = floodFill(dilated_image, Point(j, i), CV_RGB(0,0,64));
                if(area > max)
                {
                    max = area;
                    maxPt = Point(j,i);
                }
            }
        }
    }
    
    floodFill(dilated_image, maxPt, CV_RGB(255,255,255));
    
    for(int i = 0; i < dilated_image.size().height; i++)
    {
        uchar *row = dilated_image.ptr(i);
        for(int j = 0; j < dilated_image.size().width; j++)
        {
            if(row[j] == 64 && i != maxPt.x && j != maxPt.y)
            {
                int area = floodFill(dilated_image, Point(j,i), CV_RGB(0,0,0));
            }
        }
    }
    
    erode(dilated_image, eroded_image, element);
    
    undistortedThreshed = undistortedThreshed - eroded_image;
    erode(undistortedThreshed, undistortedThreshed, element);
    dilate(undistortedThreshed, undistortedThreshed, element);
    
    Mat resized = Mat(Size((undistortedThreshed.cols/9)*9, (undistortedThreshed.rows/9)*9), CV_8UC1);
    cv::resize(undistortedThreshed, resized, Size((undistortedThreshed.cols/9)*9, (undistortedThreshed.rows/9)*9));
    
    // Split the image into 81 parts, so as to identify the numbers.
    
    for(int i = 0; i < 9; i++)
    {
        for(int j = 0; j < 9; j++)
        {
            puzzle[i][j] = 0;
            Mat cell = Mat(Size(resized.cols/9, resized.rows/9), CV_8UC1);
            for(int ii = 0; ii < cell.rows; ii++)
            {
                for(int jj = 0; jj < cell.cols; jj++)
                {
                    cell.data[ii*cell.cols+jj] = resized.data[i*cell.rows*resized.cols + ii*resized.cols + jj + j*cell.cols];
                }
            }
            
            int area = countNonZero(cell);
            if(area < cell.rows * cell.cols / 24)
                continue;
            
            int start_h = 0, end_h = cell.rows;
            int start_w = 0, end_w = cell.cols;
            
            for(int ii = 0; ii < cell.rows; ii++)
            {
                for(int jj = 0; jj < cell.cols; jj++)
                {
                    if(cell.data[ii*cell.cols+jj] != 0)
                    {
                        start_h = ii;
                        break;
                    }
                }
                if(start_h != 0)
                {
                    break;
                }
            }
            
            for(int ii = cell.rows - 1; ii >= 0; ii--)
            {
                for(int jj = 0; jj < cell.cols; jj++)
                {
                    if(cell.data[ii*cell.cols+jj] != 0)
                    {
                        end_h = ii;
                        break;
                    }
                }
                if(end_h != cell.rows)
                {
                    break;
                }
            }
            end_h++;
            
            for(int jj = cell.cols/8; jj < cell.cols-cell.cols/8; jj++)
            {
                for(int ii = 0; ii < cell.rows; ii++)
                {
                    if(cell.data[ii*cell.cols+jj] != 0)
                    {
                        start_w = jj;
                        break;
                    }
                }
                if(start_w != 0)
                {
                    break;
                }
            }
            
            for(int jj = cell.cols-1-cell.cols/8; jj >= cell.cols/8; jj--)
            {
                for(int ii = 0; ii < cell.rows; ii++)
                {
                    if(cell.data[ii*cell.cols+jj] != 0)
                    {
                        end_w = jj;
                        break;
                    }
                }
                if(end_w != cell.cols)
                {
                    break;
                }
            }
            end_w++;
            
            int height = end_h - start_h;
            int width = (height*OVERLAY_COLS)/OVERLAY_ROWS;
            if(width == 0)
                width = 1;
            int left_pad = 0, right_pad = 0;
            if(width < OVERLAY_COLS)
            {
                int dif = OVERLAY_COLS - width;
                left_pad = dif/2;
                right_pad = OVERLAY_COLS-width-left_pad;
            }
            
            cv::Mat number = cv::Mat(Size(width, height), CV_8UC1);
            for(int ii = 0; ii < number.rows; ii++)
            {
                for(int jj = 0; jj < number.cols; jj++)
                {
                    if(jj < left_pad)
                        number.data[ii*number.cols+jj] = 0;
                    else if(jj >= right_pad)
                        number.data[ii*number.cols+jj] = 0;
                }
            }
            
            for(int ii = start_h; ii < end_h; ii++)
            {
                for(int jj = start_w; jj < end_w; jj++)
                {
                    number.data[(ii-start_h)*number.cols+(jj-start_w)+left_pad] = cell.data[ii*cell.cols+jj];
                }
            }
            cv::Mat number_final;
            cv::resize(number, number_final, Size(OVERLAY_COLS, OVERLAY_ROWS));
            
            // Compare the identified numbers with the templates to identify the number.
            
            long long int sum[9];
            for(int ii = 0; ii < 9; ii++)
            {
                sum[ii] = 0;
            }
            
            for(int kk = 0; kk < 9; kk++)
            {
                for(int ii = 0; ii < OVERLAY_ROWS; ii++)
                {
                    for(int jj = 0; jj < OVERLAY_COLS; jj++)
                    {
                        sum[kk] += number_final.data[ii*OVERLAY_COLS+jj] * overlays[kk].data[ii*OVERLAY_COLS+jj];
                    }
                }
            }
            int max_index = 0;
            for(int ii = 0; ii < 9; ii++)
            {
                if(sum[max_index] < sum[ii])
                    max_index = ii;
            }
            puzzle[i][j] = max_index+1;
        }
    }
    return;
}
