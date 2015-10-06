#ifndef LOCATE_H
#define LOCATE_H

#include <string>
using std::string;

class Locate {
public:
    Locate(){}

    void setter(string uuid, int apid, float positionx, float positiony, string fingerdetail)
    {
        this->uuid = uuid;
        this->apid = apid;
        this->positionx = positionx;
        this->positiony = positiony;
        this->fingerdetail = fingerdetail;
    }
    int getApid()
    {
        return this->apid;
    }
    float getPositionx()
    {
        return this->positionx;
    }
    float getPositiony()
    {
        return this->positiony;
    }
    string getFingerdetail()
    {
        return this->fingerdetail;
    }

private:
    string uuid;
    int apid;   //id 为301-12
    float positionx;
    float positiony;
    string fingerdetail;
};

#endif
