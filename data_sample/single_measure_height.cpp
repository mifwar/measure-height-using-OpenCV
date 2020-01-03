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

#define VIDEO_ID 3
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

int main()
{
    namedWindow("Control", CV_WINDOW_AUTOSIZE); //membuat window control

    /*============ membuat variable control untuk thresholding ============*/
    int iLowH = 77;
    int iHighH = 255;

    int iLowS = 112;
    int iHighS = 255;

    int iLowV = 70;
    int iHighV = 255;
    /*=====================================================================*/

    //membuat trackbar di control window
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
        flip(frame, frame, -1); //frame dibalik karena kamera yang digunakan sengaja dibalik agar imbang

        cvtColor(frame, imgHSV, COLOR_BGR2HSV); //konersi ke HSV agar bisa dithreshold 

        Mat imgThresholded;

        inRange(imgHSV,
                Scalar(iLowH, iLowS, iLowV),
                Scalar(iHighH, iHighS, iHighV),
                imgThresholded
                ); //Threshold the image

        //menggambar garis virtual warna putih di kiri dan kanan gambar threshold agar bisa fokus ke objek di tengah
        for(int i = 0 ; i < 80; i ++)
        {
            line(imgThresholded, Point(i, 0), Point(i, HEIGHT), Scalar(255,255,255));
            line(imgThresholded, Point((WIDTH-i), 0), Point( (WIDTH-i), HEIGHT), Scalar(255,255,255));
        }

        /*===================== proses pencarian contour =====================*/
        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;

        Mat inverted;
        bitwise_not(imgThresholded, inverted);

        findContours(inverted, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

        int biggestContourIdx = -1;
        float biggestContourArea = 0;
        Mat drawingBox = Mat::zeros( imgThresholded.size(), CV_8UC3 );
        for( int i = 0; i< contours.size(); i++ )
        {
            Scalar color = Scalar(0, 100, 0);
            drawContours( drawingBox, contours, i, color, 1, 8, hierarchy, 0, Point() );

            float ctArea = contourArea(contours[i]);
            if(ctArea > biggestContourArea) //sort contour terbesar
            {
                biggestContourArea = ctArea;
                biggestContourIdx = i;
            }
        }

        /*====================================================================*/

        if(biggestContourIdx < 0)
            cout << "mboten wonten tiang" << endl;
        else
        {
            RotatedRect boundingBox = minAreaRect(contours[biggestContourIdx]);
            Point2f boxCorners[4];

            boundingBox.points(boxCorners);

            //mengotaki objek yangterdeteksi
            line(frame, boxCorners[0], boxCorners[1], Scalar(0,0,0));
            line(frame, boxCorners[1], boxCorners[2], Scalar(0,0,0));
            line(frame, boxCorners[2], boxCorners[3], Scalar(0,0,0));
            line(frame, boxCorners[3], boxCorners[0], Scalar(0,0,0));

            short myCornersY[4]; //mengikuti jumlah corner yang dideteksi
            for(short i = 0; i<4;i++) //parsing data boxcorners y ke array
            {
                myCornersY[i] = (int)boxCorners[i].y;
            }

            sort(myCornersY, myCornersY + 4, getMin); //sortir ascending
            short heightNow = myCornersY[0];    //ambil Y terkecil sebagai tinggi badan

             //menggambar garis virtual merah sebagai representasi tinggi objek
            line(frame, Point(0, heightNow), Point(WIDTH, heightNow ), Scalar(0,0,255), 2);

            char buff[64];

            //mengukur tinggi badan menggunakan rumus yang telah digenerate dari myCurvefit.com
            double measure = heightNow == 0 ? 180 : calculate(heightNow); 

            //isi data char untuk dtulis di piuttext
            sprintf(buff, "pixel Y : %d || Height :%1.2f", heightNow, measure);

            //print ke terminal
            cout << buff << endl;

            //tulis di layar
            putText(frame, buff, Point(0, heightNow), FONT_HERSHEY_DUPLEX, 0.6, Scalar(0, 255, 0), 2);
        }

        imshow("frame", frame ); //menampilkan frame asli dan hasil pengukuran
        imshow("threshold", imgThresholded); //menampilkan hasil threshold
        imshow("invers", inverted); //menampilkan hasil invert

    }
    return 0;
}
