#include <MsgQueue.h>
#include <define.h>
#include <json/json.h>
#include <DBAnaysisData.h>

void MsgQueue::addLocItem(int id,const Json::Value &value)
{
	LocateVec.push_back(std::make_pair(id,value));
	debug("%d request in LocateVec now",LocateVec.size());
}

int MsgQueue::startLoc(int &id, string &rssi)
{	
	if(LocateVec.size() == 0)
		return -1;

	//从待定位请求队列里取出
	auto req = LocateVec.front();
	id = req.first;
	rssi = (req.second)["rssi"].asString();
	debug("start Locating"); 
	return 0;
}

void MsgQueue::finishLoc(int id, std::tuple<int,int,int> loc)
{
	auto front = LocateVec.front();
	if(id != front.first)
		return;
	Json::Value value;
	value["x"] = std::get<0>(loc);
	value["y"] = std::get<1>(loc);
	value["z"] = std::get<2>(loc);

	for(auto len = LocatedVec.size();len <= id; len++)	//增加LocatedVec
	{
		std::queue<Json::Value> temp;
		LocatedVec.push_back(temp);
	}

	LocatedVec[id].push(value);
	LocateVec.erase(LocateVec.begin());
	debug("add an resolut to LocatedVec[%d]",id);
}

void MsgQueue::addGuideItem(int id,const Json::Value &value)
{
	GuideVec.push_back(std::make_pair(id,value));
	debug("%d request int GuideVec now", GuideVec.size());
}


int MsgQueue::startGuide(int &id, std::pair<int,int> &sour, std::pair<int,int> &dest)
{	
	if(GuideVec.size() == 0)
		return -1;

	auto req = GuideVec.front();
	id = req.first;
	Json::Value value = req.second;

	int sx = value["sour"]["x"].asInt();
	int sy = value["sour"]["y"].asInt();
	int dx = value["dest"]["x"].asInt();
	int dy = value["dest"]["y"].asInt();
	sour = std::make_pair(sx,sy);
	dest = std::make_pair(dx,dy);
	debug("start Guiding");
	return 0;
}

void MsgQueue::finishGuide(int id, const std::vector<std::pair<int,int> > &path)
{
	auto front = GuideVec.front();
	if(id != front.first)
		return;
	Json::Value jitem,jpath;
	for(auto item = path.begin();item != path.end();item++)
	{
		jitem["x"] = item->first;
		jitem["y"] = item->second;
		jpath.append(jitem);
	}

	for(auto len = GuidedVec.size();len <= id; len++)	//增加GuidedVec
	{
		std::queue<Json::Value> temp;
		GuidedVec.push_back(temp);
	}

	GuidedVec[id].push(jpath);
	GuideVec.erase(GuideVec.begin());
	debug("add an resolut to GuideeVec[%d]",id);
}


void MsgQueue::clearRqst(int id)
{
	// for(size_t len = 0;len<LocateVec.size();len++)
	// {
	// 	if(id == LocateVec[len] )
	// 		LocateVec.erase(len);
	// }
	// debug("clean LocateVec");

	// for(size_t len = 0;len<GuidedVec.size();len++)
	// {
	// 	if(id == GuidedVec[len] )
	// 		GuidedVec.erase(len);
	// }
	// debug("clean GuideVec");
	
	// LocatedVec[id] = NULL;
	// GuidedVec[id] = NULL;
	// debug("clean LocatedVec and GuideVec");
}

