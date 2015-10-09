#pragma once
#ifndef UTIL_H
#define UTIL_H
#include <vector>
#include <map>
#include <string>
using namespace std;

class Util
{
public:
	Util();
	virtual ~Util();
	//字符串转换成double
	static double stringToNum(const string& str);
	static string numToString(const double& a);

	static vector<string> split(const string& content, const char splitchar);
	static map<string, double> StringToMap(string content);
	// static map<string,double> StringToMap(string content);
	static string MapToString(const map<string, double>& smap);
	//map1 map2的相关系数匹配
	static double MapMatch(const map<string, double>& map1, const map<string, double>& map2,int m);
	static double MapMatchW(const map<string, double>& map1, const map<string, double>& map2);
	//map1,map2 方差匹配
	static double MapMathErr2(const map<string, double>& map1, const map<string, double>& map2);
	//均值
	static double mean(const vector<double>& v);
	//方差
	static double var(const vector<double>& v);
	//协方差
	static double cov(const vector<double>& v1, const vector<double>& v2);
	//乘积
	static double module(const vector<double>& v);
	//sum += v1 * v2
	static double dotProduct(const vector<double>& v1, const vector<double>& v2);
	
	
	
    //皮尔逊系数
	static double corr(const vector<double>& v1, const vector<double>& v2);
	//夹角余弦
	static double cosine(const vector<double>& v1, const vector<double>& v2);
	
	//相关系数
	static double coefficient(const vector<double>& v1, const vector<double>& v2);
    //Dice 系数
	static double dice(const vector<double>& v1, const vector<double>& v2);
	//明氏距离
	static double minkowsky(const vector<double>& v1, const vector<double>& v2, double n);
	//欧式距离
	static double euclidean(const vector<double>& v1, const vector<double>& v2);
	static double euclidean1(const vector<double>& v1, const vector<double>& v2);
protected:
private:
};

#endif // UTIL_H

