//#include <vector>
#include <iostream>

using namespace std;
// #include <opencv/cv.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "cvYadif.h"
#include "deblur.hpp"
using namespace cv;
using namespace std;

int LEN = 4;
double THETA = 0;
int snr = 5;

int main(int argc, const char * argv[])
{

	cv::VideoCapture cap{argv[1]};
	cv::Mat frame;
	cap>>frame;
	// cv::resize(frame, frame, cv::Size(1920/2, 1080/2));
	cout<<frame.type()<<endl;
	if (frame.rows == 0 || frame.cols == 0)
           return 0;

	Mat frame_inter_0 = Mat::zeros(frame.size(),frame.type());
	Mat frame_inter_1 = Mat::zeros(frame.size(),frame.type());
	Mat frame_inter_de = Mat::zeros(frame.size(),frame.type());
	int index = 0;
	cout<<frame.rows<<" "<<frame.cols<<endl;
	for (int k = 0;k<2;k++) 
	{
		cap>>frame;
		// cv::resize(frame, frame, cv::Size(1920/2, 1080/2));
		
		if (frame.rows == 0 || frame.cols == 0)
            break;

		// create artifial sampling from frames 
		if (index %2 == 0)
		{
			//fill frmae _0
			for (int i =0; i <frame.rows; i++)
			{
				if(i %2 ==0)
				{
					for(int j = 0; j < frame.cols; j++)
					{
						frame_inter_0.at<Vec3b>(i,j) = frame.at<Vec3b>(i,j);  
					}
				}
			}
		}
		else
		{
			// fill_ frame_1
			for (int i =0; i <frame.rows; i++)
			{
				if(i %2 !=0)
				{
					for(int j = 0; j < frame.cols; j++)
					{
						frame_inter_1.at<Vec3b>(i,j) = frame.at<Vec3b>(i,j);  
					}
				}
			}
		}
		// END of creating artifial sampling from frames 

		for (int i =0; i <frame.rows; i++)
		{
			if( i%2 == 0) 
				for(int j = 0; j < frame.cols; j++)
					frame_inter_de.at<Vec3b>(i,j) = frame_inter_0.at<Vec3b>(i,j); 
			else 
				 for(int j = 0; j < frame.cols; j++)
					frame_inter_de.at<Vec3b>(i,j) = frame_inter_1.at<Vec3b>(i,j); 


		}


		// imshow("frame",frame);
		// imshow("frame_inter_de",frame_inter_de);
		// imshow("frame_inter_0",frame_inter_0);
		// imshow("frame_inter_1",frame_inter_1);
		// imshow("frame",frame);
		// waitKey(0);
		// if (waitKey(30) == 27)break;

		index ++;
	}

	cv::Yadif yadif(frame_inter_de);
	Mat frame_yadif;

	for (;;) 
	{
		cap>>frame;
		// cv::resize(frame, frame, cv::Size(1920/2, 1080/2));
		if (frame.rows == 0 || frame.cols == 0)
            break;

		// frame.convertTo(frame,CV_8UC3);
		// create artifial sampling from frames 
		if (index %2 == 0)
		{
			//fill frmae _0
			for (int i =0; i <frame.rows; i++)
			{
				if(i %2 ==0)
				{
					for(int j = 0; j < frame.cols; j++)
					{
						frame_inter_0.at<Vec3b>(i,j) = frame.at<Vec3b>(i,j);  
					}
				}
			}
		}
		else
		{
			// fill_ frame_1
			for (int i =0; i <frame.rows; i++)
			{
				if(i %2 !=0)
				{
					for(int j = 0; j < frame.cols; j++)
					{
						frame_inter_1.at<Vec3b>(i,j) = frame.at<Vec3b>(i,j);  
					}
				}
			}
		}
		// END of creating artifial sampling from frames 


		
		for (int i =0; i <frame.rows; i++)
		{
			if( i%2 == 0) 
				for(int j = 0; j < frame.cols; j++)
					frame_inter_de.at<Vec3b>(i,j) = frame_inter_0.at<Vec3b>(i,j); 
			else 
				 for(int j = 0; j < frame.cols; j++)
					frame_inter_de.at<Vec3b>(i,j) = frame_inter_1.at<Vec3b>(i,j); 


		}


		yadif.deinterlace(frame_inter_de, frame_yadif);

		/*
		Mat frame_yadif_deblur; 
		Mat deblur_ch;
		Mat c;
		cvtColor(frame_yadif,c,COLOR_BGR2GRAY);
		Rect roi = Rect(0, 0, c.cols & -2, c.rows & -2);
		//Hw calculation (start)
		Mat Hw, h;
		calcPSF(h, roi.size(), LEN, THETA);
		calcWnrFilter(h, Hw, 1.0 / double(snr));
		//Hw calculation (stop)
		c.convertTo(c, CV_32F);
		edgetaper(c, c);
		// filtering (start)
		filter2DFreq(c(roi), deblur_ch, Hw);
		// filtering (stop)
		deblur_ch.convertTo(deblur_ch, CV_8U);
		// normalize(deblur_ch, deblur_ch, 0, 255, NORM_MINMAX);

		cvtColor(c,frame_yadif_deblur,COLOR_GRAY2BGR);

		// for (auto &c:frame_ch){
		// 	Mat deblur_ch;
		// 	Rect roi = Rect(0, 0, c.cols & -2, c.rows & -2);
		// 	//Hw calculation (start)
		// 	Mat Hw, h;
		// 	calcPSF(h, roi.size(), LEN, THETA);
		// 	calcWnrFilter(h, Hw, 1.0 / double(snr));
		// 	//Hw calculation (stop)
		// 	c.convertTo(c, CV_32F);
		// 	edgetaper(c, c);
		// 	// filtering (start)
		// 	filter2DFreq(c(roi), deblur_ch, Hw);
		// 	// filtering (stop)
		// 	deblur_ch.convertTo(deblur_ch, CV_8U);
		// 	// normalize(deblur_ch, deblur_ch, 0, 255, NORM_MINMAX);
		// 	frame_de_ch.push_back(deblur_ch);
		// }

		// merge(frame_de_ch,frame_yadif_deblur);

*/
		// imshow("frame",frame);
		imshow("frame_inter_de",frame_inter_de);
		imshow("frame_yadif",frame_yadif);
		// imshow("frame_yadif_deblur",frame_yadif_deblur);
		// imshow("frame_inter_0",frame_inter_0);
		// imshow("frame_inter_1",frame_inter_1);
		// imshow("frame",frame);
		if (waitKey(30) == 27)break;

		index ++;
	}

	return 0;
}


























/*
int main(int argc, const char * argv[])
{
	cv::VideoCapture cap;
	cap.open(argv[1]);
	cv::Mat img;
	cv::Mat out; 	
	cv::Mat big_in;
	cv::Mat big_out;
		
	// cap >> img;
	// cv::Yadif yadif(img);

	do {
		cap >> img;
		if (!img.empty() && img.data != nullptr && img.rows > 0 && img.cols > 0)
		{
			
			// yadif.deinterlace(img, out);
			// cv::resize(img, big_in, cv::Size(1440, 960));
			// cv::resize(out, big_out, cv::Size(1440, 960));

			cv::imshow("in", img);
			cv::imshow("out", out);

			std::cout<<"img: "<<img.size()<<std::endl;
			std::cout<<"out: "<<out.size()<<std::endl;


		}
	} while (cv::waitKey(100) != 27);
	return 0;
}

*/