#include "EventFilter/StorageManager/interface/WebPageHelper.h"

#include "EventFilter/StorageManager/test/MockApplication.h"

int main()
{
    system("sleep 2 &");
    system("sleep 2 &");

    stor::MockApplicationStub* stub(new stor::MockApplicationStub());
    stor::WebPageHelper helper(stub->getDescriptor(), "");
    int count = helper.getProcessCount("sleep");

    //std::cout << helper.getProcessCount("CopyWorker.pl") << std::endl;
    //std::cout << helper.getProcessCount("InjectWorker.pl") << std::endl;

    exit (count != 2);
}
