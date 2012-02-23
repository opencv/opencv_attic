#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>

#define LEN 1024

char src[LEN];
char dst[LEN];

int callBack(const char * file, const struct stat * sb, int flag)
{
    int len = strlen(file);
    FILE *fp;
    if(len > 3 && (*(file+len-3) == '.') 
               && ( (*(file+len-2) == 'c') || (*(file+len-2) == 'C') )
               && ( (*(file+len-1) == 'l') || (*(file+len-1) == 'L') ))
    {
        int i,j;
        char * s;
        strcpy(src,"const char *");
        for(i=len-1;i>0;i--)
            if(*(file+i) == '/')
               break;
        if(i<=0)
            s = (char*)file;
        else
            s = (char*)&file[i+1];
        j = 0;
        while(i<len-4)
            dst[j++] = file[(i++)+1];
        fp = fopen(file,"r");
        if(!fp)
            return -1;       
        fprintf(stdout,"%s\n",strcat(strcat(src,dst)," ="));
        while(!feof(fp))
        {
            int i,len;
            memset(src,0,sizeof(src));
            memset(dst,0,sizeof(dst));
            fgets(src,LEN,fp);
            len = strlen(src);
            if(len >= LEN-3)
                return -1;
            else if(len>1)
            {
                dst[0] = '\"';
                memcpy(&dst[1],src,len);
                dst[len] = '\\';
                dst[len+1] = 'n';
                dst[len+2] = '\"';
                dst[len+3] = 0;
                fprintf(stdout,"%s\n",dst);            
            }
        }
        fprintf(stdout,";\n");
        fclose(fp);
    }
    return 0;
}

int main()
{
    fprintf(stdout,"namespace cv { namespace ocl {\n");
    ftw("./src/kernel/",callBack,16);
    fprintf(stdout,"}}\n");
}
