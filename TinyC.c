#include "stdio.h"

extern void SyntaxAnalysis();

FILE *sFile;
// char name[12];
char name[12] = "test.txt";
int main(int argc, char* argv[])
{
	// scanf("%s",name);
    sFile=fopen( name,"rt");
	SyntaxAnalysis();
	fclose(sFile);  //Free all memories
	return 0;
}
