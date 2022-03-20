#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
int main(void)
{
    int fd = open("./init_threads.json",O_RDWR);
    if(fd < 0)
    {
        perror("open fail\n");
        return -1;
    }
    char buf[2048] = {0};
    int ret = read(fd,buf,sizeof(buf));
    if (ret == -1)
    {
        perror("read error\n");
    }
    close(fd);
    cJSON* root = cJSON_Parse(buf);
    if(root == NULL)
    {
        printf("parse error\n");
    }
    cJSON* value = cJSON_GetObjectItem(root, "Threads");
    int len = cJSON_GetArraySize(value);
    int i = 0;
    cJSON* name = NULL;
    cJSON* enfunction = NULL;
    cJSON* priority = NULL;
    cJSON* canmode = NULL;
    for(i = 0; i<len; i++)
    {
        name = cJSON_GetArrayItem(value,i);
        enfunction = cJSON_GetArrayItem(value,i);
        priority = cJSON_GetArrayItem(value,i);
        canmode = cJSON_GetArrayItem(value,i);

        name = cJSON_GetObjectItem(name,"name");
        enfunction = cJSON_GetObjectItem(enfunction,"entry function");
        priority = cJSON_GetObjectItem(priority,"priority");
        canmode = cJSON_GetObjectItem(canmode,"cancel mode");
        if(name == NULL||enfunction == NULL||priority == NULL||canmode == NULL)
        {
            printf("GETOBJECTITEM FAIL\n");
            return -1;
        }
        printf("\n%s\n%s\n%s\n%s\n",cJSON_Print(name),cJSON_Print(enfunction),cJSON_Print(priority),cJSON_Print(canmode));


    }
    return 0;
}
