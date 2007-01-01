/*-------------------------------------------------------------------------*\
  <decomp.c> -- Decompress a binary file from BTCS

  Date      Programmer  Description
  12/07/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  Includes
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#include "compressor.h"
#include <getopt.h>

void ParseCommandLine(int argc, char *argv[], char *inname, char *outname);

int main(int argc, char *argv[])
{
 char inname[255];
 char outname[255];

 ParseCommandLine(argc, argv, inname, outname);
 BTCompressorReadFile file(inname);
 BinaryWriteFile file2(outname);
 IUByte b;
 try {
  while (true)
  {
   file.readUByte(b);
   file2.writeUByte(b);
  }
 }
 catch (FileException f)
 {
 }
 return 0;
}

void ParseCommandLine(int argc, char *argv[], char *inname, char *outname)
{
 char opt;
 static struct option long_options[] =
 {
  {0, 0, 0, 0}
 };

 while ((opt = getopt_long(argc, argv,"h?", long_options, NULL))!=EOF)
 {
  switch (opt)
  {
   case 'h':
   case '?':
    printf("\
Decompresses a compressed file from BTCS\n\
\n\
decomp inname outname\n\
\n\
  inname     Name of input file\n\
  outname    Name of output file\n\
");
    exit(0);
    break;
   default:
    break;
  }
 }
 if (argc - optind != 2)
 {
  printf("Incorrect number of parameters\n");
  exit(4);
 }
 strcpy(inname, argv[optind]);
 strcpy(outname, argv[optind + 1]);
}

