#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
using namespace std;
using namespace cv;

int start_x = 0;
int start_y = 0;
int end_x = 0;
int end_y = 0;
int rect_w;
int rect_h;

bool moving = false;
bool cropped = false;
Mat image;
Mat mask;
Mat maskImage;
uchar* imagedata;
uchar* maskdata;
uchar* dstdata;

string basePath = "I:/Projects/samples.txt";
string target = "I:/Projects/labels.txt";
string cfgPath = "I:/Projects/cfg.txt";
string sSourceWindow = "plane";
string sCropWindow = "crop";

struct params {
	Mat* pSource;
	fstream* pFile;
	string roiWindow;
};

void MouseHandler(int event, int x, int y, int flags, void *args) {
	stringstream* ps = (stringstream*)args;
	if (event == CV_EVENT_LBUTTONDOWN)
	{
		cout << "LButton down" << endl;
		start_x = x;
		start_y = y;
		moving = true;
	}
	else if (event == CV_EVENT_MOUSEMOVE) {
		if (!moving)
			return;
		end_x = max(start_x, x);
		end_y = max(start_y, y);
		start_x = min(start_x, x);
		start_y = min(start_y, y);
		if (start_x < 0)	start_x = 0;
		if (start_y < 0)	start_y = 0;
		if (end_x >= image.cols)	end_x = image.cols - 1;
		if (end_y >= image.rows)	end_y = image.rows - 1;
		mask.setTo(Scalar(255, 255, 255));
		rectangle(mask, Rect(start_x, start_y, end_x - start_x, end_y - start_y), Scalar(0,0,0));
		for (int i = 0; i < 3 * image.rows*image.cols; i+=3) {
			for (int j = 0; j < 3; ++j) {
				dstdata[i + j] = imagedata[i + j] & maskdata[i + j];
			}
		}
		rectangle(maskImage, Rect(start_x, start_y, end_x - start_x, end_y - start_y), Scalar(255, 0, 0));
		imshow(sSourceWindow, maskImage);
	}
	else if (event == CV_EVENT_LBUTTONUP)
	{
		cout << "LButton up" << endl;
		moving = false;
		start_x = min(x, start_x);
		start_y = min(y, start_y);
		end_x = max(x, start_x);
		end_y = max(y, start_y);
		if (start_x < 0)	start_x = 0;
		if (start_y < 0)	start_y = 0;
		if (end_x >= image.cols)	end_x = image.cols - 1;
		if (end_y >= image.rows)	end_y = image.rows - 1;
		rect_w = end_x - start_x;
		rect_h = end_y - start_y;
		if (rect_w < 5 || rect_h < 5)
			return;
		imshow(sCropWindow, image(Rect(start_x, start_y, rect_w, rect_h)));
		imshow(sSourceWindow, image);
		(*ps) << start_x << ' ' << start_y << ' ' << rect_w << ' ' << rect_h << '\n';
		cropped = true;
	}
}

int main() {
	int cnt = 0;
	fstream sampleFile, targetFile, cfg;
	fstream* pf=NULL;
	stringstream content;
	sampleFile.open(basePath, ios::in);
	targetFile.open(target, ios::in | ios::app);
	cfg.open(cfgPath, ios::in);
	cfg >> cnt;
	cfg.close();
	string line;
	vector<string> fileNames;
	while (!sampleFile.eof()) {
		sampleFile >> line;
		if (line.length() > 0)
			fileNames.push_back(line);
	}
	sampleFile.close();
	namedWindow(sSourceWindow);
	namedWindow(sCropWindow);

	setMouseCallback(sSourceWindow, MouseHandler, &content);
	Mat mSource, mCropped;
	char key = 0;
	for (int i = cnt; i < fileNames.size(); ++i) {
		key = 0;
		cout << fileNames[i] << endl;
		image = imread(fileNames[i], 1);
		mask = Mat(image.size(), image.type());
		maskImage = Mat(image.size(), image.type());
		imagedata = image.data;
		maskdata = mask.data;
		dstdata = maskImage.data;
		if (image.rows == 0 || image.cols == 0)
			continue;
		imshow(sSourceWindow, image);

		while (key != ' ' && key != 'q')
			key = waitKey(50);
		if (key == 'q')	break;
		if (cropped) {
			targetFile << fileNames[i] << "\n{\n";
			targetFile << content.str();
			targetFile << "}\n";
			cropped = false;
		}
		content.str("");
		++cnt;
		cout << "Next image" << endl;
	}
	cout << "Processed images: " << cnt << endl;
	cfg.open(cfgPath, ios::out);
	cfg << cnt;
	targetFile.close();
	cfg.close();
	/*
	string base_dir = "I:/Projects/images/0.jpg";
	image = imread(base_dir, 1);
	mask = Mat(image.size(), image.type());
	maskImage = Mat(image.size(), image.type());
	imagedata = image.data;
	maskdata = mask.data;
	dstdata = maskImage.data;
	namedWindow(sSourceWindow);
	setMouseCallback(sSourceWindow, MouseHandler);
	imshow(sSourceWindow, image);
	waitKey();
	*/
	destroyAllWindows();
	return 0;
}