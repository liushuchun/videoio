#include "extract_flows.h"

using namespace std;
using namespace cv;



inline bool isFlowCorrect(Point2f u)
{
	return !cvIsNaN(u.x) && !cvIsNaN(u.y) && fabs(u.x) < 1e9 && fabs(u.y) < 1e9;
}


static Vec3b computeColor(float fx, float fy)
{
	static bool first = true;

	// relative lengths of color transitions:
	// these are chosen based on perceptual similarity
	// (e.g. one can distinguish more shades between red and yellow
	//  than between yellow and green)
	const int RY = 15;
	const int YG = 6;
	const int GC = 4;
	const int CB = 11;
	const int BM = 13;
	const int MR = 6;
	const int NCOLS = RY + YG + GC + CB + BM + MR;
	static Vec3i colorWheel[NCOLS];

	if (first)
	{
		int k = 0;

		for (int i = 0; i < RY; ++i, ++k)
			colorWheel[k] = Vec3i(255, 255 * i / RY, 0);

		/*
		for (int i = 0; i < YG; ++i, ++k)
		colorWheel[k] = Vec3i(255 - 255 * i / YG, 255, 0);

		for (int i = 0; i < GC; ++i, ++k)
		colorWheel[k] = Vec3i(0, 255, 255 * i / GC);

		for (int i = 0; i < CB; ++i, ++k)
		colorWheel[k] = Vec3i(0, 255 - 255 * i / CB, 255);

		for (int i = 0; i < BM; ++i, ++k)
		colorWheel[k] = Vec3i(255 * i / BM, 0, 255);

		for (int i = 0; i < MR; ++i, ++k)
		colorWheel[k] = Vec3i(255, 0, 255 - 255 * i / MR);
		*/
		first = false;

	}

	const float rad = sqrt(fx * fx + fy * fy);
	const float a = atan2(-fy, -fx) / (float)CV_PI;

	const float fk = (a + 1.0f) / 2.0f * (NCOLS - 1);
	const int k0 = static_cast<int>(fk);
	const int k1 = (k0 + 1) % NCOLS;
	const float f = fk - k0;

	Vec3b pix;

	for (int b = 0; b < 3; b++)
	{
		const float col0 = colorWheel[k0][b] / 255.0f;
		const float col1 = colorWheel[k1][b] / 255.0f;

		float col = (1 - f) * col0 + f * col1;

		if (rad <= 1)
			col = 1 - rad * (1 - col); // increase saturation with radius
		else
			col *= .75; // out of range

		pix[2 - b] = static_cast<uchar>(255.0 * col);
	}

	return pix;
}

static void drawOpticalFlow(const Mat_<float>& flowx, const Mat_<float>& flowy, Mat& dst, float maxmotion = -1)
{
	dst.create(flowx.size(), CV_8UC3);
	dst.setTo(Scalar::all(0));

	// determine motion range:
	float maxrad = maxmotion;

	if (maxmotion <= 0)
	{
		maxrad = 1;
		for (int y = 0; y < flowx.rows; ++y)
		{
			for (int x = 0; x < flowx.cols; ++x)
			{
				Point2f u(flowx(y, x), flowy(y, x));

				if (!isFlowCorrect(u))
					continue;

				maxrad = max(maxrad, sqrt(u.x * u.x + u.y * u.y));
			}
		}
	}

	for (int y = 0; y < flowx.rows; ++y)
	{
		for (int x = 0; x < flowx.cols; ++x)
		{
			Point2f u(flowx(y, x), flowy(y, x));

			if (isFlowCorrect(u))
				dst.at<Vec3b>(y, x) = computeColor(u.x / maxrad, u.y / maxrad);
		}
	}
}

int compute_flows(Mat frame0, Mat frame1, serialize_params*& pFlowData, int iIndex)
{
	cv::Mat planes[2], flowx, flowy, fflow;
	cv::calcOpticalFlowFarneback(frame0, frame1, fflow, 0.702, 5, 10, 2, 7, 1.5, cv::OPTFLOW_FARNEBACK_GAUSSIAN);
	cv::split(fflow, planes);
	cv::normalize(planes[0], flowx, 0, 255, NORM_MINMAX);
	cv::normalize(planes[1], flowy, 0, 255, NORM_MINMAX);
	unsigned long iFlowDataSize = flowx.rows*flowx.cols * sizeof(char);
	memcpy(pFlowData->pContent + iIndex*iFlowDataSize*2, flowx.datastart, iFlowDataSize);
	memcpy(pFlowData->pContent + iIndex*iFlowDataSize*2 + iFlowDataSize, flowy.datastart, iFlowDataSize);
/*
	Mat out;
	drawOpticalFlow(planes[0], planes[1], out, 10);
	cvtColor(out, out, CV_RGB2GRAY);
	normalize(out, out, 0, 255, NORM_MINMAX);
	char name[20];
	sprintf(name, "flowshow%02d.png", iIndex);
	imwrite(name, out);
*/
	return 0;
}


int  CalOptFlow(serialize_params*  decode_memory, serialize_params*&  extract_output)
{
	int  iRet = 0;
	int  iLoop = 0;

	int iInputCount = decode_memory->count;
	int iOutputCount = decode_memory->count - 1;
	int iWidth = decode_memory->width;
	int iHeight = decode_memory->height;
	int iChannels = decode_memory->channels;
	
	extract_output->count = decode_memory->count - 1;
	extract_output->width = decode_memory->width;
	extract_output->height = decode_memory->height;
	extract_output->channels = 2;

	
	do
	{
		if (decode_memory == NULL)
		{
			break;
		}

		cv::Mat grey_prev, grey_cur;
		for (iLoop = 0; iLoop < iInputCount; iLoop++)
		{
			cv::Mat mat_cur(iHeight, iWidth, CV_8UC3, decode_memory->pContent + iLoop * iWidth * iHeight * iChannels);
			cvtColor(mat_cur, grey_cur, CV_BGR2GRAY);

			if(iLoop > 0){
				iRet = compute_flows(grey_prev, grey_cur, extract_output, iLoop - 1);
			}

			grey_prev = grey_cur.clone();

		}

	} while (0);

	return 0;
}

