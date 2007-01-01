#include "compressor.h"
#include <igrimage.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

void ParseCommandLine(int argc, char *argv[], char *inname, char *outname,
  IUShort *width, IUShort *heigth);
IImage IImagePicPacLoad(const char *filename, IUShort width, IUShort height);

int main(int argc, char *argv[])
{
 char inname[255];
 char outname[255];
 IImage img = NULL;
 IUShort height, width;

 ParseCommandLine(argc, argv, inname, outname, &width, &height);
 img = IImagePicPacLoad(inname, width, height);
 IImagePCXSave(img, outname);
 IImageDestroy(img);
}

void ParseCommandLine(int argc, char *argv[], char *inname, char *outname,
  IUShort *width, IUShort *height)
{
 char opt;
 static struct option long_options[] =
 {
  {"width", 1, 0, 'w'},
  {"height", 1, 0, 'e'},
  {0, 0, 0, 0}
 };

 *height = 88;
 *width = 112;
 while ((opt = getopt_long(argc, argv,"h?e:w:", long_options, NULL))!=EOF)
 {
  switch (opt)
  {
   case 'h':
   case '?':
    printf("\
Converts the pic and pac files used by BTCS into pcx files\n\
\n\
pcx2piac [options] inname [outname[.pcx]]\n\
  Options:\n\
    -e, -height=PIXELS  Heigth of the image (defaults to 88)\n\
    -w, -width=PIXELS   Width of the image (defaults to 112)\n\
\n\
  inname                Name of input file\n\
  [outname[.pcx]]       Name of output file (defaults to input file name)\n\
");
    exit(0);
    break;
   case 'e':
    *height = atoi(optarg);
    break;
   case 'w':
    *width = atoi(optarg);
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
 if (argc - optind == 2)
 {
  strcpy(outname, argv[optind + 1]);
 }
 else
 {
  strcpy(outname, inname);
  outname[strlen(outname) - 4] = 0;
 }
 if (strcmp(outname + strlen(outname) - 4, ".pcx") != 0)
 {
  strcat(outname, ".pcx");
 }
}

IImage IImagePicPacLoad(const char *filename, IUShort width, IUShort height)
{
 IImage img;
 char tmpname[255];

 img = (IImage)IMalloc(sizeof(struct IImageStruct));
 if (img == NULL) {
  return img;
 }
 img->x = width;
 img->y = height;
 img->pic = (IPixel IFAR *)IMalloc(sizeof(IPixel) * img->x * img->y);
 img->pal = (IPalette)IMalloc(sizeof(IPaletteTable));
 if ((img->pic == NULL) || (img->pal == NULL))
 {
  if (img->pic != NULL) IFree(img->pic);
  if (img->pal != NULL) IFree(img->pal);
  IFree(img);
  return NULL;
 }
 try {
  {
   strcpy(tmpname, filename);
   strcat(tmpname, ".pic");
   BTCompressorReadFile file(tmpname);
   file.readUByteArray(img->x * img->y, (IUByte *)img->pic);
  }

  {
   memset((*img->pal), 0, sizeof(IPaletteTable));
   strcpy(tmpname, filename);
   strcat(tmpname, ".pac");
   BTCompressorReadFile file(tmpname);
   file.readUByteArray(256 * 3, (IUByte *)(*img->pal));
  }
 }
 catch (...)
 {
  IImageDestroy(img);
  throw;
 }
 return img;
}

