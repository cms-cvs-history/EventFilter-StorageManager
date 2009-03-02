#include "EventFilter/StorageManager/interface/WebPageHelper.h"

int main()
{
    system("sleep 2 &");
    system("sleep 2 &");

    int count = stor::WebPageHelper::getProcessCount("sleep");

    //std::cout << helper.getProcessCount("CopyWorker.pl") << std::endl;
    //std::cout << helper.getProcessCount("InjectWorker.pl") << std::endl;

    exit (count != 2);
}
