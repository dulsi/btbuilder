#include "compressor.h"
#include <igrimage.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

void ParseCommandLine(int argc, char *argv[], char *inname, char *outname);
void IImagePicPacSave(IImage img, const char *filename);

int main(int argc, char *argv[])
{
 char inname[255];
 char outname[255];
 IImage img = NULL;

 ParseCommandLine(argc, argv, inname, outname);
 img = IImagePCXLoad(inname);
 IImagePicPacSave(img, outname);
 IImageDestroy(img);
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
Converts a pcx file into the pic and pac files used by BTCS\n\
\n\
pcx2piac [options] inname[.pcx] [outname]\n\
\n\
  inname[.pcx]     Name of input file\n\
  [outname]  Name of output file (defaults to input file name)\n\
");
    exit(0);
    break;
   default:
    break;
  }
 }
 if ((argc - optind < 1) || (argc - optind > 2))
 {
  printf("Incorrect number of parameters\n");
  exit(4);
 }
 strcpy(inname, argv[optind]);
 if (strcmp(inname + strlen(inname) - 4, ".pcx") != 0)
 {
  strcat(inname, ".pcx");
 }
 if (argc - optind == 2)
 {
  strcpy(outname, argv[optind + 1]);
 }
 else
 {
  strcpy(outname, inname);
  outname[strlen(outname) - 4] = 0;
 }
}

void IImagePicPacSave(IImage img, const char *filename)
{
 static IUByte unknown[16] =
 {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
  0x0C, 0x0D, 0x0E, 0x0F
 };
 static IUByte unknown2[2] =
 {
  0x00, 0x00
 };
 static IUByte unknown3[4] =
 {
  0x00, 0x01, 0x02, 0x03
 };
 char tmpname[255];

 {
  strcpy(tmpname, filename);
  strcat(tmpname, ".pic");
  BTCompressorWriteFile file(tmpname);
  file.writeUByteArray(img->x * img->y, (IUByte *)img->pic);
 }

 {
  int i;
  strcpy(tmpname, filename);
  strcat(tmpname, ".pac");
  BTCompressorWriteFile file(tmpname);
  IUByte b(0x00);
  file.writeUByteArray(256 * 3, (IUByte *)img->pal);
  for (i = 0; i < 18; i++)
  {
   file.writeUByteArray(16, unknown);
  }
  file.writeUByteArray(2, unknown2);
  for (i = 0; i < 18 * 4; i++)
  {
   file.writeUByteArray(4, unknown3);
  }
 }
}

