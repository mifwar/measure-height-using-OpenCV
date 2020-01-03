// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

#define max '1'
#define min '2'

#define WIDTH 352
#define HEIGHT 288

#define VIDEO_ID 4
#define INIT_SERIAL

using namespace cv;
using namespace std;

bool getMax(int a, int b){return a > b;}
bool getMin(int a, int b){return a < b;}

double calculate(int data)
{
    return 7.283817 + (168.7579 - 7.283817)/(1 + pow((data/221.5929),1.881378));
}

const cv::Rect leftRect(0, 0, WIDTH / 3, HEIGHT);
const cv::Rect rightRect(WIDTH * 2/3, 0, WIDTH, HEIGHT);

char typed;

cv::Mat frame;
Mat imgHSV;

struct dataUtama
{
    int min_x;
    int min_y;
    int max_x;
    int max_y;
    int heightNow;
};

struct contourQ
{
    float area;
    int index;
};

bool biggestArea(contourQ a, contourQ b){return a.area > b.area;}
bool compareq(int a, int b){return a < b;}

int proceed(short arrX[], short arrY[])
{
    dataUtama temp;

    sort(arrX, arrX + 4, getMax);
    temp.max_x = arrX[0];

    sort(arrX, arrX + 4, getMin);
    temp.min_x = arrX[0];

    sort(arrY, arrY + 4, getMax);
    temp.max_y = arrY[0];

    sort(arrY, arrY + 4, getMin);
    temp.min_y = arrY[0];

    short heightNow = arrY[0];

    return heightNow;
}

int main()
{
    namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"

    int iLowH = 77;
    int iHighH = 255;

    int iLowS = 40;
    int iHighS = 255;

    int iLowV = 70;
    int iHighV = 255;

    //Create trackbars in "Control" window
    createTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
    createTrackbar("HighH", "Control", &iHighH, 179);

    createTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
    createTrackbar("HighS", "Control", &iHighS, 255);

    createTrackbar("LowV", "Control", &iLowV, 255);//Value (0 - 255)
    createTrackbar("HighV", "Control", &iHighV, 255);

    VideoCapture cap(VIDEO_ID);

    if(!cap.isOpened())
    {
        cout << "there is something wrong. please retry!" << endl;
        return -1;
    }
    cap.set(CV_CAP_PROP_FRAME_WIDTH,WIDTH);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,HEIGHT);


    while(typed!=27)
    {
        typed = waitKey(10);
        cap >> frame;
        flip(frame, frame, -1);

        int width = cap.get(CAP_PROP_FRAME_WIDTH);
        int height = cap.get(CAP_PROP_FRAME_HEIGHT);

//        cout << "width : " << width << endl;
//        cout << "height : " << height << endl;

        cvtColor(frame, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

        Mat imgThresholded;

        inRange(imgHSV,
                Scalar(iLowH, iLowS, iLowV),
                Scalar(iHighH, iHighS, iHighV),
                imgThresholded
                ); //Threshold the image

        erode(imgThresholded,
               imgThresholded,
               getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1) ));

        dilate(imgThresholded,
               imgThresholded,
               getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1) ));

        for(int i = 0 ; i < 80; i ++)
        {
            line(imgThresholded, Point(i, 0), Point(i, HEIGHT), Scalar(255,255,255));
            line(imgThresholded, Point((WIDTH-i), 0), Point( (WIDTH-i), HEIGHT), Scalar(255,255,255));
        }

        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;

        Mat inverted;
        bitwise_not(imgThresholded, inverted);

        findContours(inverted, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

        int totalContour = contours.size();

        Mat drawingBox = Mat::zeros( inverted.size(), CV_8UC3 );
        contourQ myData[totalContour ];
        for( int i = 0; i< totalContour ; i++ )
        {
            Scalar color = Scalar(0, 100, 0);
            drawContours( drawingBox, contours, i, color, 1, 8, hierarchy, 0, Point() );

            float ctArea = contourArea(contours[i]);
            myData[i].area = ctArea;
            myData[i].index = i;
        }

        sort(myData, myData + totalContour, biggestArea);

//        cout << "myData0 : " << myData[0].area << endl;
//        cout << "myData1 : " << myData[1].area << endl;

        if(contours.size() < 0)
            cout << "mboten wonten tiang" << endl;
        else
        {
            RotatedRect boundingBox[totalContour];
            Point2f boxCorners[totalContour][4];
            int heightNow[totalContour];
            short myCornersX[totalContour][4];
            short myCornersY[totalContour][4];
            for(int i = 0 ; i < totalContour ; i++)
            {
                boundingBox[i] = minAreaRect(contours[i]);
                Point2f boxCorners[i][4];
                boundingBox[i].points(boxCorners[i]);

                line(frame, boxCorners[i][0], boxCorners[i][1], Scalar(0,0,0));
                line(frame, boxCorners[i][1], boxCorners[i][2], Scalar(0,0,0));
                line(frame, boxCorners[i][2], boxCorners[i][3], Scalar(0,0,0));
                line(frame, boxCorners[i][3], boxCorners[i][0], Scalar(0,0,0));

                for(short j = 0; j < 4; j++)
                {
                    myCornersX[i][j] = (int)boxCorners[i][j].x;
                    myCornersY[i][j] = (int)boxCorners[i][j].y;
                }
                heightNow[i] = proceed(myCornersX[i], myCornersY[i]);
            }

            sort(heightNow, heightNow + totalContour, compareq);

            char buff[totalContour][64];
            double measure[totalContour];

            for(int i = 0; i < totalContour ; i ++)
            {
                line(frame, Point(0, heightNow[i]), Point(WIDTH, heightNow[i] ), Scalar(0,0,255), 2);
                measure[i] = heightNow[i] == 0 ? 180 : calculate(heightNow[i]);
                sprintf(buff[i], "PERSON%d : pixel Y : %d Height :%1.2f", i+1, heightNow[i], measure[i]);
                cout << buff[i] << endl;
                putText(frame, buff[i], Point(0, heightNow[i]), FONT_HERSHEY_DUPLEX, 0.6, Scalar(0, 0, 255), 2);
            }
        }

        imshow("frame", frame );
        imshow("threshold", imgThresholded);
        imshow("invers", inverted);

    }
    return 0;
}
