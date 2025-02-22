#include "briskextractor.hpp"
#include <cpluslogger.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

defuse::BRISKXtractor::BRISKXtractor()
{
	mKeypointDetector = cv::BRISK::create();
	mDescriptorExtactor = cv::BRISK::create();
}

defuse::FeaturesBase* defuse::BRISKXtractor::xtract(VideoBase* _videobase)
{
	cv::VideoCapture stream(_videobase->mFile->getFile());

	if (!stream.isOpened())
	{
	LOG_FATAL("Error: Video Stream cannot be opened: " << _videobase->mFile->getFile());
	}

	int frameNumbers = (int) stream.get(CV_CAP_PROP_FRAME_COUNT);
	int keyframe = getKeyframeNumber(frameNumbers);

 
	// inputs
	cv::Mat color, gray;
	// outputs
	std::vector<cv::KeyPoint> keypoints;
	cv::Mat descriptors;

	// frame A
	stream.set(CV_CAP_PROP_POS_FRAMES, keyframe);
	stream >> color;
	// image to grayscale
	cv::cvtColor(color, gray, cv::COLOR_BGR2GRAY);

	// compute brisk
	//Measure extraction time
	double e1Start = double(cv::getTickCount());
	computeBriskKeypoints(gray, keypoints);
	computeBriskDescriptors(gray, keypoints, descriptors);
	double e1End = double(cv::getTickCount());
	double elapsedSecs = (e1End - e1Start) / double(cv::getTickFrequency());


	// create features
	FeaturesBase* features = new FeaturesBase(_videobase->mFile->getFilename(), descriptors);
	features->mExtractionTime = float(elapsedSecs);

	stream.release();
	return features;

}

void defuse::BRISKXtractor::testBrisk(VideoBase* _videobase, int _framedistance) const
{
	cv::VideoCapture stream(_videobase->mFile->getFile());

	if (!stream.isOpened())
	{
	LOG_FATAL("Error: Video Stream cannot be opened: " << _videobase->mFile->getFile());
	}

	int frameNumbers = (int) stream.get(CV_CAP_PROP_FRAME_COUNT);
	int keyframeA = frameNumbers / 2;
	int keyframeB = keyframeA + _framedistance;

	LOG_INFO("Length of input Video: " << frameNumbers);
	LOG_INFO("Keyframe: " << keyframeA);

	// inputs
	cv::Mat colorA, colorB, grayA, grayB;
	// outputs
	std::vector<cv::KeyPoint> keypointsA, keypointsB;
	cv::Mat descriptorsA, descriptorsB;

	// frame A
	stream.set(CV_CAP_PROP_POS_FRAMES, keyframeA);
	stream >> colorA;
	// image to grayscale
	cv::cvtColor(colorA, grayA, cv::COLOR_BGR2GRAY);
	// cv::cvtColor(colorA, grayA, cv::COLOR_BGR2GRAY);

	// frame B
	stream.set(CV_CAP_PROP_POS_FRAMES, keyframeB);
	stream >> colorB;
	cv::cvtColor(colorB, grayB, cv::COLOR_BGR2GRAY);

	// no functions
	// cv::Ptr<cv::BRISK> detector = cv::BRISK::create();
	// detector->detect(grayA, keypointsA);
	// detector->detect(grayB, keypointsB);
	// cv::Ptr<cv::BRISK> descriptorExtractor = cv::BRISK::create();
	// descriptorExtractor->compute(grayA, keypointsA, descriptorsA);
	// descriptorExtractor->compute(grayB, keypointsB, descriptorsB);

	// functions
	computeBriskKeypoints(grayA, keypointsA);
	computeBriskKeypoints(grayB, keypointsB);
	computeBriskDescriptors(grayA, keypointsA, descriptorsA);
	computeBriskDescriptors(grayB, keypointsB, descriptorsB);

	// LOG_INFO("TEST DIMS: " << descriptorsA.rows << " x " << descriptorsA.cols);

	// draw result
	// Declare one type off matcher
	cv::BFMatcher matcher(cv::NORM_HAMMING);
	std::vector<cv::DMatch> matches;
	matcher.match(descriptorsA, descriptorsB, matches);
	cv::Mat all_matches;
	cv::drawMatches( grayA, keypointsA, grayB, keypointsB,
						matches, all_matches, cv::Scalar::all(-1), cv::Scalar::all(-1),
						std::vector<char>(),cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
	cv::imshow( "BRISK All Matches", all_matches );
	cv::waitKey(0);

	// imshow("frame", grayA);
	// cv::waitKey(0);

}

std::string defuse::BRISKXtractor::toString()
{
	std::string result;
	result += "xtractor: cvBRISK, ";
	result += getKeyframeSelectionAsString();
	return result;
}

std::string defuse::BRISKXtractor::getXtractorID() const
{
	std::stringstream st;
	st << "cvBRISK_";
	st << mKeyFrameSelection;
	return st.str();
}

bool defuse::BRISKXtractor::computeBriskKeypoints(cv::Mat& _grayimage, std::vector<cv::KeyPoint>& _keypoints) const
{
	mKeypointDetector->detect(_grayimage, _keypoints);
	return true;
}

bool defuse::BRISKXtractor::computeBriskDescriptors(cv::Mat& _grayimage, std::vector<cv::KeyPoint>& _keypoints, cv::OutputArray& _signatures) const
{
	mDescriptorExtactor->compute(_grayimage, _keypoints, _signatures);
	return true;
}
