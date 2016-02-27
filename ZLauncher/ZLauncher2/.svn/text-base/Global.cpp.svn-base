
#include "Global.h"
#include "stdafx.h"


void ClearDebugMSG( void)
{
#ifdef _DEBUG

	FILE *fstream;
	fstream = fopen( "log.txt", "w" );

	fclose( fstream );

#endif
}


void PutDebugMSG( const char* szText)
{
#ifdef _DEBUG

	FILE *fstream;
	fstream = fopen( "log.txt", "a+" );

	fprintf( fstream, "%05d : %s\n", (int)GetTickCount(), szText);

	fclose( fstream );

#endif
}