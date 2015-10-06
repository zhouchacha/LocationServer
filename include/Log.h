#include "log4cpp/Category.hh"
#include "log4cpp/OstreamAppender.hh"
#include "log4cpp/BasicLayout.hh"
#include "log4cpp/Priority.hh"
#include "log4cpp/PropertyConfigurator.hh"
class Log
{
public:
    static Log* getInstance();
    static Log* instance;
    log4cpp::Category& getRootLog();
    log4cpp::Category& getRecvLog();
    log4cpp::Category& getSendLog();
    log4cpp::Category& getStatLog();
private:
    Log(void);
    ~Log(void);
};
