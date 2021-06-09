#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/calib3d.hpp>
using namespace cv;

class CameraPose
{
public:
    Mat viewMatrix;
	bool using_markerless;
	bool is_mark;

	Mat getCamMatrix();

	void readCamParameters(String);
	void pose_estimate(cv::Mat &image);

	CameraPose(String);
	~CameraPose();
private:
    float markerLength = 1.75;
	int minHessian = 400;
    Mat camera_matrix;
	Mat dist_coeffs;

    Ptr<aruco::Dictionary> dictionary;
	Ptr<aruco::DetectorParameters> detectorParams;
	std::vector< int > markerIds;
	std::vector< std::vector<cv::Point2f> > markerCorners, rejectedCandidates;
};

Mat CameraPose::getCamMatrix()
{
	return this->camera_matrix;
}

void CameraPose::readCamParameters(String path)
{
	this->dictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME(0));

	cv::FileStorage fs(path, cv::FileStorage::READ);

	fs["camera_matrix"] >> this->camera_matrix;
	fs["distortion_coefficients"] >> dist_coeffs;

	std::cout << "camera_matrix\n"
		<< camera_matrix << std::endl;
	std::cout << "\ndist coeffs\n"
		<< dist_coeffs << std::endl;
}

void CameraPose::pose_estimate(cv::Mat &image)
{
    cv::aruco::detectMarkers(image, this->dictionary, this->markerCorners, this->markerIds, this->detectorParams, this->rejectedCandidates);
    //Draw maker
    cv::Mat outputImage = image.clone();
    cv::aruco::drawDetectedMarkers(outputImage, markerCorners, markerIds);

    if (markerIds.size() > 0) {
		std::vector< cv::Vec3d > rvecs, tvecs;

		cv::aruco::drawDetectedMarkers(image, markerCorners, markerIds);
		cv::aruco::estimatePoseSingleMarkers(markerCorners, this->markerLength, this->camera_matrix, this->dist_coeffs, rvecs, tvecs);

		for (unsigned int i = 0; i < markerIds.size(); i++) {
			cv::Mat rot;
			cv::Vec3d r = rvecs[i];
			cv::Vec3d t = tvecs[i];

			this->viewMatrix = cv::Mat::zeros(4, 4, CV_32F);

			Rodrigues(rvecs[i], rot);
			for (unsigned int row = 0; row < 3; ++row)
			{
				for (unsigned int col = 0; col < 3; ++col)
				{
					this->viewMatrix.at<float>(row, col) = (float)rot.at<double>(row, col);
				}
				this->viewMatrix.at<float>(row, 3) = (float)tvecs[i][row];
			}
			this->viewMatrix.at<float>(3, 3) = 1.0f;

			cv::Mat cvToGl = cv::Mat::zeros(4, 4, CV_32F);
			cvToGl.at<float>(0, 0) = 1.0f;
			cvToGl.at<float>(1, 1) = -1.0f;
			cvToGl.at<float>(2, 2) = -1.0f;
			cvToGl.at<float>(3, 3) = 1.0f;
			this->viewMatrix = cvToGl * this->viewMatrix;
			cv::transpose(this->viewMatrix, this->viewMatrix);
		}
		this->is_mark = true;
	}
	else
	{
		this->is_mark = false;
	}
}

CameraPose::CameraPose(String)
{
    
}

CameraPose::~CameraPose()
{
}
