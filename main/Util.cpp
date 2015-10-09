#include "Util.h"

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <cmath>
#include <cassert>
#include <limits>
using namespace std;
Util::Util()
{
	//ctor
}

Util::~Util()
{
	//dtor
}
double Util::stringToNum(const string& str)
{
	istringstream iss(str);
	double num;
	iss >> num;
	return num;
}
string Util::numToString(const double& num){
	int prec = numeric_limits<double>::max(); // 18
	ostringstream out;
	out.precision(prec);//����Ĭ�Ͼ���
	out << num;
	string str = out.str();
	return str;
}
vector<string> Util::split(const string& s, const char splitchar){
	vector<string> vec;
	if (vec.size()>0)//��֤vec�ǿյ�
		vec.clear();
	int length = s.length();
	int start = 0;
	for (int i = 0; i<length; i++)
	{
		if (s[i] == splitchar && i == 0)//��һ���������ָ��
		{
			start += 1;
		}
		else if (s[i] == splitchar)
		{
			vec.push_back(s.substr(start, i - start));
			start = i + 1;
		}
		else if (i == length - 1)//����β��
		{
			vec.push_back(s.substr(start, i + 1 - start));
		}
	}
	return vec;
}
map<string, double> Util::StringToMap(string content){
	map<string, double> smap;
	vector<string> vec = Util::split(content, ';');
	for (vector<string>::iterator it = vec.begin(); it != vec.end(); it++){
		vector<string> tmpv = Util::split(*(it), ',');
		smap.insert(map<string, double>::value_type(tmpv[0], Util::stringToNum(tmpv[1])));
	}
	return smap;
}

string Util::MapToString(const map<string, double>& smap){

	map<string, double>::const_iterator iter = smap.begin();
	string content = "";
	int count = 0;
	while (iter != smap.end()){
		string key = iter->first;
		string value = Util::numToString(iter->second);
		string tmp = key + ",";
		tmp += value;
		count++;
		if (count != smap.size()){
			content += tmp;
			content += ";";
		}
		else{
			content += tmp;
			content += "\n";
		}
		iter++;
	}
	return content;
}
//map1 map2�����ϵ��ƥ��
double Util::MapMatch(const map<string, double>& map1, const map<string, double>& map2,int m){
	map<string, double>::const_iterator iter = map1.begin();
	vector<double> xfinger;
	vector<double> yfinger;
	while (iter != map1.end()){  //遍历map1
		double X = 0.0;
		double Y = 0.0;
		string key = iter->first;  //取AP的MAC值
		map<string, double>::const_iterator iter2 = map2.find(key);  //查找map2里相同ap
		
		if (iter2 != map2.end()){   //找到相同ap后分别将RSSI值赋予X,Y
			X = iter->second;
			Y = iter2->second;
		}
		if (X < 0 && Y < 0){        //插入vector
			xfinger.push_back(X);
			yfinger.push_back(Y);
		}
		iter++;
	}
	if (xfinger.size()>(m-4)){		//
	return Util::corr(xfinger,yfinger);
	}
	else return 0.0;
//	return Util::cosine(xfinger,yfinger);
//	return Util::coefficient(xfinger,yfinger);
//	return Util::dice(xfinger,yfinger);
//	return Util::minkowsky(xfinger,yfinger);
//	return Util::euclidean(xfinger,yfinger);
	
}
double Util::MapMatchW(const map<string, double>& map1, const map<string, double>& map2){

	map<string, double>::const_iterator iter = map1.begin();
	vector<double> xfinger;
	vector<double> yfinger;

	while (iter != map1.end()){
		double X = 0.0;
		double Y = 0.0;
		string key = iter->first;
		map<string, double>::const_iterator iter2 = map2.find(key);
		if (iter2 != map2.end()){
			X = iter->second;
			Y = iter2->second;
		}
		if (X < 0 && Y < 0){
			xfinger.push_back(X);
			yfinger.push_back(Y);
		}
		iter++;
	}
	return Util::euclidean1(xfinger,yfinger);
}

double Util::mean(const vector<double>& v){
	double sum = 0.0;
	for (vector<double>::const_iterator iter = v.begin(); iter != v.end(); iter++){
		sum += *(iter);
	}
	return sum / v.size();
}

//����

double Util::cov(const vector<double>& v1, const vector<double>& v2){
	if (v1.size() != v2.size()) {
		return -1.0;
	}
	if (v1.size() == 0) {
		return 0.0;
	}
	else {
		double sum = 0.0;
		double avg1 = mean(v1);
		double avg2 = mean(v2);
		
		for (int i=0; i<v1.size(); i++) {
			sum += (v1[i] - avg1) * (v2[i] - avg2);
		}
		return sum / v1.size();
	}
}

 double Util::dotProduct(const vector<double>& v1, const vector<double>& v2)
 {
     assert(v1.size() == v2.size());
     double sum = 0.0;
     for (vector<double>::size_type i = 0; i != v1.size(); ++i)
     {
         sum += v1[i] * v2[i];
     }
     return sum;
 }
 double Util::module(const vector<double>& v)
 {
     double sum = 0.0;
    for (vector<double>::const_iterator iter = v.begin(); iter != v.end(); iter++){
		sum += (*(iter))*(*(iter));
	}
     return sqrt(sum);
 }

 
 
 
 // 夹角余弦
 double Util::cosine(const vector<double>& v1, const vector<double>& v2)
 {
     assert(v1.size() == v2.size());
     return dotProduct(v1, v2) / (module(v1) * module(v2));
 }

// 相关系数
 double Util::coefficient(const vector<double>& v1, const vector<double>& v2)
 {
     assert(v1.size() == v2.size());
     return cov(v1, v2) / sqrt(cov(v1, v1) * cov(v2, v2));
 }

// Dice 系数
 double Util::dice(const vector<double>& v1, const vector<double>& v2)
 {
     assert(v1.size() == v2.size());
     return 2.0 * dotProduct(v1, v2) / (dotProduct(v1, v1) + dotProduct(v2, v2));
 }
 
// 明氏距离 
 double Util::minkowsky(const vector<double>& v1, const vector<double>& v2, double n)
 {
     assert(v1.size() == v2.size());    
	 double sum = 0.0;
     for (vector<double>::size_type i = 0; i != v1.size(); ++i)
     {
        sum += pow(abs(v1[i] - v2[i]), n);
     }
     return pow(sum, 1.0 / n);
 }
 
// 欧式距离
 double Util::euclidean(const vector<double>& v1, const vector<double>& v2)
 {
     assert(v1.size() == v2.size());
     return minkowsky(v1, v2, 2.0);
 }
//  1/欧式距离
 double Util::euclidean1(const vector<double>&v1, const vector<double>& v2)
 {
	 assert(v1.size() == v2.size());
	 double sum = 0.0;
	 for (vector<double>::size_type i = 0; i != v1.size(); ++i )
	 {
		 sum += pow(abs(v1[i] -v2[i] ),2.0);
	 }
	 return  100 / pow(sum , 1.0 / 2.0)+ 0.001;
	 
 }

//皮尔逊相关系数
double Util::corr(const vector<double>& v1, const vector<double>& v2){
	double sum = cov(v1, v2);
	return sum / (sqrt(cov(v1,v1)) * sqrt(cov(v2,v2)));
}


