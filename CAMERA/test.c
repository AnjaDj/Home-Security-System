#include "kamera.c"
#include <stdio.h>


int main (void) 
{
    //Unesite zeljenu putanju za fotografiju
	char filename[] = "/home/raso/image1.jpg";
	takePic(filename);
	printf("Image successful..\n");

	return 0;
}
