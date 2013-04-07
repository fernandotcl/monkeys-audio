/***************************************************************************************
MAC Console Frontend (MAC.exe)

Pretty simple and straightforward console front end.  If somebody ever wants to add 
more functionality like tagging, auto-verify, etc., that'd be excellent.

Copyrighted (c) 2000 - 2003 Matthew T. Ashland.  All Rights Reserved.
***************************************************************************************/
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "All.h"
#include "GlobalFunctions.h"
#include "MACLib.h"
#include "CharacterHelper.h"

// defines
#define COMPRESS_MODE		0
#define DECOMPRESS_MODE		1
#define VERIFY_MODE			2
#define CONVERT_MODE		3
#define UNDEFINED_MODE		-1

// global variables
static TICK_COUNT_TYPE g_nInitialTickCount = 0;
static int g_bStdoutIsTerminal = 0;
static int g_bOutputIsStdout = 0;

/***************************************************************************************
Displays the proper usage for MAC.exe
***************************************************************************************/
static void DisplayProperUsage(FILE * pFile)
{
	fprintf(pFile, "Usage: [Input File] [Output File] [Mode]\n\n");

	fprintf(pFile, "Modes: \n");
	fprintf(pFile, "    Compress (fast): '-c1000'\n");
	fprintf(pFile, "    Compress (normal): '-c2000'\n");
	fprintf(pFile, "    Compress (high): '-c3000'\n");
	fprintf(pFile, "    Compress (extra high): '-c4000'\n");
	fprintf(pFile, "    Compress (insane): '-c5000'\n");
	fprintf(pFile, "    Decompress: '-d'\n");
	fprintf(pFile, "    Verify: '-v'\n");
	fprintf(pFile, "    Convert: '-nXXXX'\n\n");

	fprintf(pFile, "Examples:\n");
	fprintf(pFile, "    Compress: mac \"Metallica - One.wav\" \"Metallica - One.ape\" -c2000\n");
	fprintf(pFile, "    Decompress: mac \"Metallica - One.ape\" \"Metallica - One.wav\" -d\n");
	fprintf(pFile, "    Verify: mac \"Metallica - One.ape\" -v\n");
}

/***************************************************************************************
Progress callback
***************************************************************************************/
static void CALLBACK ProgressCallback(int nPercentageDone)
{
	if (!g_bStdoutIsTerminal || g_bOutputIsStdout)
	{
		return;
	}

	// get the current tick count
	TICK_COUNT_TYPE  nTickCount;
	TICK_COUNT_READ(nTickCount);

	// calculate the progress
	double dProgress = nPercentageDone / 1.e5;											// [0...1]
	double dElapsed = (double) (nTickCount - g_nInitialTickCount) / TICK_COUNT_FREQ;	// seconds
	double dRemaining = dElapsed * ((1.0 / dProgress) - 1.0);							// seconds

	// output the progress
	printf("Progress: %.1f%% (%.1f seconds remaining, %.1f seconds total)		  \r",
		dProgress * 100, dRemaining, dElapsed);
	fflush(stdout);
}

/***************************************************************************************
Main (the main function)
***************************************************************************************/
int main(int argc, char * argv[])
{
	// variable declares
	CSmartPtr<wchar_t> spInputFilename; CSmartPtr<wchar_t> spOutputFilename;
	int nRetVal = ERROR_UNDEFINED;
	int nMode = UNDEFINED_MODE;
	int nCompressionLevel = COMPRESSION_LEVEL_NORMAL;
	int nPercentageDone;

	// parse the command line arguments
	int opt;
	while ((opt = getopt_long(argc, argv, "c:dhvn:", NULL, NULL)) != -1) {
		switch (opt) {
			case 'c':
				nMode = COMPRESS_MODE;
				nCompressionLevel = atoi(optarg);
				break;
			case 'd':
				nMode = DECOMPRESS_MODE;
				break;
			case 'h':
				DisplayProperUsage(stdout);
				return EXIT_SUCCESS;
			case 'v':
				nMode = VERIFY_MODE;
				break;
			case 'n':
				nMode = CONVERT_MODE;
				nCompressionLevel = atoi(optarg);
				break;
			default:
				DisplayProperUsage(stderr);
				return EXIT_FAILURE;
		}
	}

	// error check the mode
	if (nMode == UNDEFINED_MODE)
	{
		DisplayProperUsage(stderr);
		return EXIT_FAILURE;
	}

	// make sure we have enough arguments
	int nNumFilenames = nMode == VERIFY_MODE ? 1 : 2;
	int nNumProvided = argc - optind;
	if (nNumFilenames != nNumProvided)
	{
		DisplayProperUsage(stderr);
		return EXIT_FAILURE;
	}

	// store the file names
	spInputFilename.Assign(GetUTF16FromANSI(argv[optind]), TRUE);
	if (nNumFilenames == 2)
	{
		spOutputFilename.Assign(GetUTF16FromANSI(argv[optind + 1]), TRUE);
	}

	// verify that the input file exists
	if (!FileExists(spInputFilename))
	{
		fprintf(stderr, "The input filename does not seem to exist\n");
		return EXIT_FAILURE;
	}

	// check if we are outputting to stdout
	if (nNumFilenames == 2 && !strcmp(argv[optind + 1], "-"))
	{
		g_bOutputIsStdout = 1;
	}

	// get and error check the compression level
	if (nMode == COMPRESS_MODE || nMode == CONVERT_MODE) 
	{
		if (nCompressionLevel != 1000 && nCompressionLevel != 2000 && 
			nCompressionLevel != 3000 && nCompressionLevel != 4000 &&
			nCompressionLevel != 5000) 
		{
			DisplayProperUsage(stderr);
			return EXIT_FAILURE;
		}
	}

	// set the initial tick count
	TICK_COUNT_READ(g_nInitialTickCount);

	// check if the output is a terminal
	g_bStdoutIsTerminal = isatty(fileno(stdout));

	// process
	int nKillFlag = 0;
	if (nMode == COMPRESS_MODE) 
	{
		char cCompressionLevel[16];
		if (nCompressionLevel == 1000) { strcpy(cCompressionLevel, "fast"); }
		if (nCompressionLevel == 2000) { strcpy(cCompressionLevel, "normal"); }
		if (nCompressionLevel == 3000) { strcpy(cCompressionLevel, "high"); }
		if (nCompressionLevel == 4000) { strcpy(cCompressionLevel, "extra high"); }
		if (nCompressionLevel == 5000) { strcpy(cCompressionLevel, "insane"); }

		if (!g_bOutputIsStdout)
		{
			printf("Compressing (%s)...\n", cCompressionLevel);
			fflush(stdout);
		}

		nRetVal = CompressFileW(spInputFilename, spOutputFilename, nCompressionLevel, &nPercentageDone, ProgressCallback, &nKillFlag);
	}
	else if (nMode == DECOMPRESS_MODE) 
	{
		if (!g_bOutputIsStdout)
		{
			printf("Decompressing...\n");
			fflush(stdout);
		}

		nRetVal = DecompressFileW(spInputFilename, spOutputFilename, &nPercentageDone, ProgressCallback, &nKillFlag);
	}	
	else if (nMode == VERIFY_MODE) 
	{
		if (!g_bOutputIsStdout)
		{
			printf("Verifying...\n");
			fflush(stdout);
		}

		nRetVal = VerifyFileW(spInputFilename, &nPercentageDone, ProgressCallback, &nKillFlag);
	}	
	else if (nMode == CONVERT_MODE) 
	{
		if (!g_bOutputIsStdout)
		{
			printf("Converting...\n");
			fflush(stdout);
		}

		nRetVal = ConvertFileW(spInputFilename, spOutputFilename, nCompressionLevel, &nPercentageDone, ProgressCallback, &nKillFlag);
	}

	if (g_bStdoutIsTerminal && !g_bOutputIsStdout)
	{
		printf("\n");
	}

	if (nRetVal == ERROR_SUCCESS)
	{
		if (!g_bOutputIsStdout)
		{
			printf("Success!\n");
		}
		return EXIT_SUCCESS;
	}
	else
	{
		if (!g_bOutputIsStdout)
		{
			printf("Error: %i\n", nRetVal);
		}
		return EXIT_FAILURE;
	}
}
