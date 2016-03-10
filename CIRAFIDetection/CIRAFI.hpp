#pragma once
#include "stdafx.h"
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\features2d\features2d.hpp>
#include <iostream>
#include <numeric>
#include <opencv2\imgproc\imgproc.hpp>

using namespace cv;
using namespace std;

#ifndef _CIRAFI_H_
#define _CIRAFI_H_
namespace CIRAFI
{
	class CorrData
	{
	public:
		CorrData(int row, int col, int scale, int angle, double coefficient) :_row(row), _col(col), _scale(scale), _angle(angle), _coefficient(coefficient) {}
		double GetCoefficient() { return _coefficient; }
		int GetScale() { return _scale; }
		int GetAngle() { return _angle; }
		int GetRow() { return _row; }
		int GetCol() { return _col; }
	private:
		double _coefficient;
		int _scale;
		int _angle;
		int _row;
		int _col;
	};

	class CIRAFIData
	{
	public:
		CIRAFIData() :_scaleNum(5), _initialScale(0.5), _finalScale(1.0), _angleNum(36), _scaleThreshold(0.8), _angleThreshold(0.5), _nccThreshold(0.9)
			, _isMatchNegative(false), _circleNum(16), _initialRadius(0), _finalRadius(-1), _tefiTolerance(1) {}
		void CountParameter(cv::Mat& templateImage);
		double scale(double s) { return _initialScale*pow(_passoesc, s); }
		void SetScaleNum(int scaleNum) { _scaleNum = scaleNum; }
		void SetInitialScale(double initialScale) { _initialScale = initialScale; }
		void SetFinalScale(double finalScale) { _finalScale = finalScale; }
		void SetAngleNum(int angleNum) { _angleNum = angleNum; }
		void SetScaleThreshold(double scaleThreshold) { _scaleThreshold = scaleThreshold; }
		void SetAngleThreshold(double angleThreshold) { _angleThreshold = angleThreshold; }
		void SetNccThreshold(double nccThreshold) { _nccThreshold = nccThreshold; }
		void SetMatchNegative(bool isMatchNegative) { _isMatchNegative = isMatchNegative; }
		void SetCircleNum(int circleNum) { _circleNum = circleNum; }
		void SetInitialRadius(double initialRadius) { _initialRadius = initialRadius; }
		void SetTefiTolerance(int tefiTolerance) { _tefiTolerance = tefiTolerance; }

		template <class T>
		T clip(const T val, const T lower, const T upper) { return std::max(lower, std::min(val, upper)); }
		double CircularSample(cv::Mat& image, int y, int x, int radius);
		void Cisssa(cv::Mat& sourceImage);
		cv::Mat quadradaimpar(cv::Mat& image);
		void Cissq(cv::Mat& templateImage);
		void Cifi(cv::Mat& sourceImage);
		void CifiAnalysis(cv::Mat& sourceImage);

		double RadialSample(cv::Mat& image, int centerY, int centerX, double angle, double radius);
		void Rassq(cv::Mat& templateImage);
		void Rafi(cv::Mat& sourceImage);

		void CIRAFIData::Tefi(cv::Mat& sourceImage, cv::Mat& templateImage);
		cv::Mat DrawTefiResult(cv::Mat& sourceImage, double sampleRatio = 1);

		std::vector<CorrData> _cis;
		std::vector<CorrData> _ras;
		std::vector<CorrData> _tes;

	private:
		int _scaleNum;
		double _initialScale;
		double _finalScale;
		int _angleNum;
		double _scaleThreshold;
		double _angleThreshold;
		double _nccThreshold;
		bool _isMatchNegative;
		int _circleNum;
		double _initialRadius;
		double _finalRadius;
		int _tefiTolerance;
		double _circleDistance;
		double _passoesc;
		double _angleDegree;
		double _angleRadian;
		double _templateRadius;
		std::vector<double> _ca;
		std::vector<double> _cq;
		std::vector<double> _rq;
	};

	inline double round(double val, int precision = 0)
	{
		double mul = pow(10, (double)precision);
		val *= mul;
		val = (val<0.0) ? ceil(val - 0.5) : floor(val + 0.5);
		return val / mul;
	}
}


#endif