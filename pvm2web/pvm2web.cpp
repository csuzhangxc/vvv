#include <string>
#include <iostream>
#include <sstream>
using namespace std;

// needed to create a directory
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include <viewer/ddsbase.h>
#include "texture.h"

#define MAXSTR 1000

// function to convert int to string
string int2str(int number)
{
   stringstream ss;
   ss << number;
   return ss.str();
}

int main(int argc, char *argv[])
{
   int numParams = 0;

   argv++;

   for (int i = 1; i<argc || argc<=1; i++)
   {
      unsigned char *volume0, *volume;
      unsigned int width, height, depth, components;
      float scalex, scaley, scalez;
      unsigned int nwidth, nheight, ndepth;

      unsigned char *texture;
      unsigned int length;

      char ch_inputfilename[MAXSTR];
      char ch_outputfilename[MAXSTR];
      char ch_outputfolder[MAXSTR];

      // no cmd line arguments given
      if (argc<=1)
      {
         printf("File to convert: \n");
         fgets(ch_inputfilename, MAXSTR, stdin);
      }
      else
      {
         strncpy(ch_inputfilename, *argv, MAXSTR);
      }

      // erase bracketing "
      string help = string(ch_inputfilename);
      if(help.find("\"")==0)
         help.erase(0,1);
      if(help.rfind("\"") == help.size()-1)
         help.erase(help.size()-1, help.size());
      strcpy(ch_inputfilename, help.c_str());

      // load the volume
      printf("reading PVM file\n");
      if (!(volume0=readPVMvolume(ch_inputfilename,
                                  &width, &height, &depth,
                                  &components, &scalex, &scaley, &scalez))) ERRORMSG();
      printf("found volume with width=%d height=%d depth=%d components=%d\n",
             width,height,depth,components);

      // convert to 8bit
      if (components==2)
         volume0=quantize(volume0, width, height, depth);

      // convert to nxnxn size
      if (!(volume = resampleVolume(volume0,
                                    width, height, depth,
                                    length))) ERRORMSG();
      free(volume0);

      // create the first part for the name of the output file
      string str_CompleteFilePath = string(ch_inputfilename);
      if (str_CompleteFilePath.rfind(".pvm") != string::npos)
         str_CompleteFilePath.erase(str_CompleteFilePath.rfind(".pvm"), str_CompleteFilePath.size());
      if (str_CompleteFilePath.size() == 0) ERRORMSG();

      // get the name of the directory and the name of the PVM file
      string str_Folder = str_CompleteFilePath;
      if (str_Folder.rfind("\\") != string::npos)
         str_Folder.erase(str_Folder.rfind("\\")+1, str_Folder.size());
      if (str_Folder.rfind("/") != string::npos)
         str_Folder.erase(str_Folder.rfind("/")+1, str_Folder.size());
      string str_File = str_CompleteFilePath;
      if (str_File.rfind("\\") != string::npos)
         str_File.erase(0, str_File.rfind("\\")+1);
      if (str_File.rfind("/") != string::npos)
         str_File.erase(0, str_File.rfind("/")+1);

      // create the basic file name of the resulting PGM files
      string str_CreatedFileName = int2str(length);
      str_CreatedFileName += "_";
      str_CreatedFileName += str_File;

      // create the folder to store the files
      str_Folder += str_CreatedFileName;
      strncpy(ch_outputfolder, str_Folder.c_str(), MAXSTR);
#ifdef _WIN32
      mkdir(ch_outputfolder);
#else
      mkdir(ch_outputfolder, 0777);
#endif
      str_Folder += "/";

      str_CreatedFileName += "_";

      // write the output files for each visible side
      char plane;
      for (int p=0; p<3; p++)
      {
         if (p==0) plane = 'x';
         else if (p==1) plane = 'y';
         else plane = 'z';

         for (int val = 0; val < length; val++)
         {
            // create the names of the individual planes
            string str_CreatedFileNamePGM = str_CreatedFileName;
            str_CreatedFileNamePGM += int2str(plane);
            str_CreatedFileNamePGM += "_";
            str_CreatedFileNamePGM += int2str(val);
            str_CreatedFileNamePGM += ".pgm";
            str_CreatedFileNamePGM = str_Folder + str_CreatedFileNamePGM;
            strncpy(ch_outputfilename, str_CreatedFileNamePGM.c_str(), MAXSTR);

            // get a trilinear interpolated 2D texture
            texture = getTextureFromVolume(volume, length, plane, val);

            // save the 2D texture as PGM file
            writePNMimage(ch_outputfilename,
                          texture, length, length,
                          1, // components
                          0); // dds compression

            free(texture);
         }
      }

      free(volume);
   }

   return(0);
}
