// C library headers
#include <stdio.h>
#include <string.h>

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

#define max '1'
#define min '2'

#define WIDTH 352 	//frame width
#define HEIGHT 288	//frame height

#define VIDEO_ID 3  //camera id. in ubuntu you can check it using ls /dev | grep video

using namespace cv;
using namespace std;

bool getMin(int a, int b){return a < b;}

double calculate(int data)
{
    return 7.283817 + (168.7579 - 7.283817)/(1 + pow((data/221.5929),1.881378)); //a formula that we get from sample
}

const cv::Rect leftRect(0, 0, WIDTH / 3, HEIGHT);
const cv::Rect rightRect(WIDTH * 2/3, 0, WIDTH, HEIGHT);

char typed;

Mat frame;
Mat imgThresholded;
Mat imgHSV;

int main()
{
	/*=================================== create window control ==================================*/
    namedWindow("Control", CV_WINDOW_AUTOSIZE); 

    int iLowH = 77;
    int iHighH = 255;

    int iLowS = 112;
    int iHighS = 255;

    int iLowV = 70;
    int iHighV = 255;
	/*============================================================================================*/

    /*============================= create trackbar in control window ============================*/
    createTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
    createTrackbar("HighH", "Control", &iHighH, 179);

    createTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
    createTrackbar("HighS", "Control", &iHighS, 255);

    createTrackbar("LowV", "Control", &iLowV, 255);//Value (0 - 255)
    createTrackbar("HighV", "Control", &iHighV, 255);
    /*============================================================================================*/

    //start video capture
    VideoCapture cap(VIDEO_ID); 

    if(!cap.isOpened())
    {
        cout << "there is something wrong. please retry!" << endl;
        return -1;
    }

    //set frame size
    cap.set(CV_CAP_PROP_FRAME_WIDTH,WIDTH);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,HEIGHT);

    //main loop
    while(typed!=27)
    {
        typed = waitKey(10);
        cap >> frame;

        /*==================================== start thresholding ====================================*/
        cvtColor(frame, imgHSV, COLOR_BGR2HSV); 

        inRange(imgHSV,
                Scalar(iLowH, iLowS, iLowV),
                Scalar(iHighH, iHighS, iHighV),
                imgThresholded
                ); 

        //create virtual white rectangle to avoid detecting unimportant object
        for(int i = 0 ; i < 80; i ++)
        {
            line(imgThresholded,
            	 Point(i, 0),
            	 Point(i, HEIGHT),
            	 Scalar(255,255,255)
            	 );

            line(imgThresholded,
            	 Point((WIDTH-i), 0),
            	 Point( (WIDTH-i), HEIGHT),
            	 Scalar(255,255,255)
            	 );
        }
        /*============================================================================================*/

        /*==================================== start find contour ====================================*/
        vector<vector<Point> > contours;

        Mat inverted;
        bitwise_not(imgThresholded, inverted); //invert threshold color

        findContours(inverted, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

        int key = -1;
        float biggestArea = 0;

        Mat drawingBox = Mat::zeros( imgThresholded.size(), CV_8UC3 );

        for( int i = 0; i < contours.size(); i++ )
        {
            float ctArea = contourArea(contours[i]);
            if(ctArea > biggestArea) //sort biggestContour
            {
                biggestArea = ctArea;
                key = i;
            }
        }

        /*============================================================================================*/

        /*==================================== start measurement =====================================*/
        if(key < 0)
            cout << "no object detected" << endl;
        else
        {
        	//create a boundingbox
            RotatedRect boundingBox = minAreaRect(contours[key]);
            Point2f boxCorners[4];

            boundingBox.points(boxCorners);

            //create a bounding box in object
            line(frame, boxCorners[0], boxCorners[1], Scalar(0,0,0));
            line(frame, boxCorners[1], boxCorners[2], Scalar(0,0,0));
            line(frame, boxCorners[2], boxCorners[3], Scalar(0,0,0));
            line(frame, boxCorners[3], boxCorners[0], Scalar(0,0,0));

            short myCornersY[4]; //mengikuti jumlah corner yang dideteksi
            for(short i = 0; i<4;i++) //parsing data boxcorners y ke array
            {
                myCornersY[i] = (int)boxCorners[i].y;
            }

            sort(myCornersY, myCornersY + 4, getMin); //ascending sort
            short heightNow = myCornersY[0];    //get smallest Y (highest Y pixel in frame)

             //create virtual red line above heightNow
            line(frame, Point(0, heightNow), Point(WIDTH, heightNow ), Scalar(0,0,255), 2);

            char buff[64];
            double measure = calculate(heightNow); 
            sprintf(buff, "pixel Y : %d || Height :%1.2f", heightNow, measure);
            cout << buff << endl;
            putText(frame, buff, Point(0, heightNow), FONT_HERSHEY_DUPLEX, 0.6, Scalar(0, 255, 0), 2);
        }

        imshow("frame", frame ); //show original frame
        imshow("threshold", imgThresholded); //show thresholded frame
        imshow("invers", inverted); //show inverted frame

        /*============================================================================================*/

    }
    return 0;
}
