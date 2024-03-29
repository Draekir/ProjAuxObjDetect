// CIRAFIDetection.cpp : Defines the entry point for the console application.
//
// Need to install opencv 3.0.0 libraries
// Need to link to opencv_world300.lib, opencv_world300d.lib, opencv_ts300.lib, opencv_ts300d.lib

#include "stdafx.h"
#include <vector>
#include <map>
#include <iostream>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\features2d\features2d.hpp>
#include "CIRAFI.hpp"

using namespace cv;
using namespace std;
using namespace CIRAFI;

Mat frame; // matrix to hold frame under analysis
Mat imgHSV; // matrix to hold frame converted to HSV colour space
Mat imgThresh; // matrix to hold thresholded image
Mat HSVmask;
Mat testmask;

vector<vector<Point>> contours;
vector<Vec4i> hierarchy;

// HSV values for white
int maxH = 180; int minH = 0;
int maxS = 100; int minS = 0;
int maxV = 255; int minV = 150;

// Inverted HSV values for red
int H_MIN = 10; int H_MAX = 170;
int S_MIN = 0; int S_MAX = 255;
int V_MIN = 0; int V_MAX = 255;

vector<CIRAFIData> LibData;
int LibSize = 36;
vector<char> strID = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

map<char, int> scoreHistory;

int main()
{

	VideoCapture camera(0); // connect to default camera (device 0)
	if (!camera.isOpened()) // if failed to connect to camera, end program
	{
		cout << "Failed to connect to camera. Quitting program." << endl;
		cin.get();
		return -1;
	}

	//Initialise Display Windows
	namedWindow("Camera Input", CV_WINDOW_NORMAL);
	namedWindow("Thresholded Image", CV_WINDOW_NORMAL);

	// Initialise Template Library
	string pathname = "Template Library/";
	int tempRad = 50; //this needs to be half of the largest dimension of the template
	for (int i = 0; i < LibSize; i++)
	{
		string filename = pathname + "cropped_";
		filename += strID[i];
		filename += ".jpg";
		Mat tImg = imread(filename);
		resize(tImg, tImg, Size(tempRad*2, tempRad*2));
		cvtColor(tImg, tImg, CV_BGR2HSV);
		inRange(tImg, Scalar(0, 0, 0), Scalar(255, 100, 255), tImg);
		Mat contImg = tImg.clone();
		cv::findContours(contImg, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
		Mat roi = tImg(boundingRect(contours[contours.size()-1]));
		LibData.push_back(CIRAFIData(roi, strID[i]));
		/*imshow(to_string(strID[i]), roi);
		waitKey(2000);*/
	}

	while (1)
	{
		bool bCheck = camera.read(frame); // read frame from camera
		if (!bCheck)
		{
			cout << "Failed to read frame from camera." << endl;
			break;
		}

		// Resize frame to 640x480
		resize(frame, frame, Size(640, 480));
		
		// Image Pre-Processing
		cvtColor(frame, imgHSV, CV_BGR2HSV); // convert from RGB to HSV colour space
		inRange(imgHSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), HSVmask);
		bitwise_not(HSVmask, HSVmask);
		erode(HSVmask, HSVmask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5))); // apply erosion to image
		erode(HSVmask, HSVmask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5))); // apply erosion to image
		dilate(HSVmask, HSVmask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5))); // apply dilation to image
		floodFill(HSVmask, Point(0, 0), 256);
		bitwise_not(HSVmask, HSVmask);
		frame.copyTo(testmask, HSVmask);
		cvtColor(testmask, testmask, CV_BGR2HSV);
		inRange(testmask, Scalar(minH, minS, minV), Scalar(maxH, maxS, maxV), imgThresh); // threshold image based on set parameters
		HSVmask = Mat::zeros(1, 1, CV_64F);
		testmask = Mat::zeros(1, 1, CV_64F);

		// Morphological Operations
		// Opening
		erode(imgThresh, imgThresh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5))); // apply erosion to image
		dilate(imgThresh, imgThresh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5))); // apply dilation to image

		// Closing
		dilate(imgThresh, imgThresh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5))); // apply dilation to image
		erode(imgThresh, imgThresh, getStructuringElement(MORPH_ELLIPSE, Size(5, 5))); // apply erosion to image
		
		// Find seperate object bounding boxes in image
		Mat contImg = imgThresh.clone();
		cv::findContours(contImg, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		// Loop through each object
		for (int i = 0; i < contours.size(); i++)
		{
			Mat roi = Mat::zeros(640, 480, CV_64F);
			Rect bBox = boundingRect(contours[i]);
			if (bBox.area() >(76800)) continue;
			if (bBox.width < tempRad || bBox.height < tempRad) continue;
			/// bounding box is wrong shape - get centroid of bounding box and expand to square size?
			/*int x = bBox.x + (bBox.width / 2);
			int y = bBox.y + (bBox.height / 2);
			if (bBox.width > bBox.height)
			{
				bBox.y = y - (bBox.width / 2);
				bBox.height = bBox.width;
			}
			else if (bBox.width < bBox.height) {
				bBox.x = x - (bBox.height / 2);
				bBox.width = bBox.height;
			}*/

			roi = imgThresh(bBox);
			//resize(roi, roi, Size(tempRad * 2, tempRad * 2));
			imshow("ROI", roi);

			ObjectData obj = ObjectData(tempRad);
			obj.ObjectAnalysis(roi);

			// Loop through each template to obtain CIRAFI coefficients
			for (int n = 0; n < LibData.size(); n++)
			{
				LibData[n].ObjectCompare(roi, obj.Get());
			}
			
		}
		
		// Calculate total coefficient score for each template
		vector<LetterData> scores;
		double scoreThresh = 0.7;
		for (int n = 0; n < LibData.size(); n++)
		{
			double score = LibData[n].CalculateCoef();
			if (score >= scoreThresh)
			{
				scores.push_back(LetterData(LibData[n].GetTempLetter(), score));
			}
			LibData[n].ResetCoefficients();
		}

		// Sort coefficient scores
		sort(scores.begin(), scores.end(),
			[](LetterData const &a, LetterData const &b) {
			return a.coef > b.coef; });

		// Display letter with highest confidence coefficient
		if (!scores.empty()) {
			//cout << "Most likely letter: " << scores[0].letter << " Score: " << scores[0].coef << endl << endl;
			cout << scores[0].letter << " " << scores[1].letter << " " << scores[2].letter << endl << endl;

			// Add to score history counter
			if (scoreHistory.count(scores[0].letter) > 0) {
				scoreHistory.at(scores[0].letter) += 1;
			}
			else {
				scoreHistory.emplace(scores[0].letter, 1);
			}
		}
		else {
			cout << "No likely matches found." << endl << endl;
		}

		imshow("Camera Input", frame); // show original frame
		resizeWindow("Camera Input", 640, 480);
		imshow("Thresholded Image", imgThresh); // show thresholded image
		resizeWindow("Thresholded Image", 640, 480);

		if (waitKey(30) == 27)
		{
			cout << "Process ended by user" << endl;
			return -1;
		}
	}
    return 0;
}

