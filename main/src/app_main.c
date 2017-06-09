#include "inc/system/init.h"
#include "inc/system/tasks.h"

int app_main(void)
{
    device_init();
    driver_init(); 
     
    tasks_init();
    
    return 0;
}
