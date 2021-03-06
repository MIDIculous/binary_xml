#include "bin_xml.h"
#include "bin_xml_creator.h"
#include "bin_xml_packer.h"

#include <stdio.h>
#include <string.h> // strcpy...
#include <limits.h> // PATH_MAX
#include <cstddef>  // nullptr

int main(int argc,char **argv)
{
    printf("2xb\n");

    
    for(int i = 1;i < argc;i++)
    {
        char dst[PATH_MAX];
        strcpy(dst,argv[i]);
        char *dot = strrchr(dst,'.');
        if (dot == NULL) return 1;
        if (dot-dst+4 > PATH_MAX) return 1;
        dot++;
        *(dot++) = 'x';
        *(dot++) = 'b';
        *(dot++) = '\0';

        printf("Input:  %s\n",argv[i]);
        printf("Output: %s\n",dst);
        if (!pklib_xml::Bin_xml_packer::Convert(argv[i],dst))
            return 1;
    }
    
    return 0; // ok
}
