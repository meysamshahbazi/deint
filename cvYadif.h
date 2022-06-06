#ifndef cvYadif_H__
#define cvYadif_H__

#include <opencv2/core/core.hpp>

namespace cv
{
	class Yadif
	{
	public:
		Yadif(const cv::Mat &first);
		void deinterlace(const cv::Mat &input, cv::Mat &output);
	private:
		int inputPosition;
		unsigned long frame;
		std::vector<cv::Mat> buf[4];
	};
};

#endif

