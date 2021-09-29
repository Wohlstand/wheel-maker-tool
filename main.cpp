/*
 * Wheel maker tool
 *
 * Copyright © 2021 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the “Software”), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <cstdio>

#include <FreeImageLite.h>
#include <tclap/CmdLine.h>


static bool isPointInCircle(float xa, float ya, float xc, float yc, float r)
{
    return ((xa - xc) * (xa - xc) + (ya - yc) * (ya - yc)) < r * r;
}


static FIBITMAP *genhDIB8bpp(FIBITMAP *hDIB32bpp)
{
    FIBITMAP *hDIB8bpp = NULL;

    int w = FreeImage_GetWidth(hDIB32bpp);
    int h = FreeImage_GetHeight(hDIB32bpp);

    FIBITMAP *hDIB24bpp = FreeImage_Allocate(w, h, 24);
    for(int i = h - 1; i >= 0; i--)
    {
        for(int j = 0; j < w; j++)
        {
            RGBQUAD src_color;
            FreeImage_GetPixelColor(hDIB32bpp, j, i, &src_color);
            if(src_color.rgbReserved == 0xFF)
            {
                if(src_color.rgbBlue == 0xFF && src_color.rgbRed == 0 && src_color.rgbGreen == 0)
                    src_color.rgbBlue = 0xFE; // Aviod confusion
                FreeImage_SetPixelColor(hDIB24bpp, j, i, &src_color);
            }
            else
            {
                src_color.rgbRed = 0;
                src_color.rgbBlue = 0xFF;
                src_color.rgbGreen = 0;
                FreeImage_SetPixelColor(hDIB24bpp, j, i, &src_color);
            }
        }
    }

    BYTE Transparency[256];

    if(!hDIB8bpp)
        hDIB8bpp = FreeImage_ColorQuantize(hDIB32bpp, FIQ_WUQUANT);

    RGBQUAD *Palette = FreeImage_GetPalette(hDIB8bpp);
    for(int i = 0; i < 256; i++)
    {
        Transparency[i] = 0xFF;
        if(Palette[i].rgbRed == 0x00 && Palette[i].rgbBlue == 0xFF && Palette[i].rgbGreen == 0x00)
            Transparency[i] = 0x00;
    }

    FreeImage_SetTransparencyTable(hDIB8bpp, Transparency, 256);
    FreeImage_Unload(hDIB24bpp);

    return hDIB8bpp;
}


static void printDot()
{
    std::fprintf(stderr, ".");
    std::fflush(stderr);
}


int main(int argc, char **argv)
{
    double   angleStep;
    uint32_t gifDelay;
    uint32_t maxWidth;
    std::vector<std::string> files;

    try
    {
        // Define the command line object.
        TCLAP::CmdLine  cmd("Wheel maker tool\n"
                            "Copyright (c) 2017-2021 Vitaly Novichkov <admin@wohlnet.ru>\n"
                            "This program is distributed under the MIT license\n", ' ', "1.0");

        TCLAP::ValueArg<uint32_t> flagGifDelay("d", "delay",
                                               "Interframe delay",
                                               false, 40, "40");

        TCLAP::ValueArg<double> flagAngleStep("a", "angle-step",
                                              "Interframe delay",
                                              false, 18.0, "20");

        TCLAP::ValueArg<uint32_t> flagMaxWidth("w", "max-width",
                                               "Maximum width size",
                                               false, 512, "512");

        TCLAP::UnlabeledMultiArg<std::string> inputFileNames("filepaths",
                "Input image file(s)",
                true,
                "Input file path(s)");

        cmd.add(&flagGifDelay);
        cmd.add(&flagAngleStep);
        cmd.add(&flagMaxWidth);
        cmd.add(&inputFileNames);

        cmd.parse(argc, argv);

        gifDelay = flagGifDelay.getValue();
        angleStep = flagAngleStep.getValue();
        maxWidth = flagMaxWidth.getValue();

        for(const std::string &fpath : inputFileNames.getValue())
            files.push_back(fpath);

        if((argc <= 1) || files.empty())
        {
            std::fprintf(stderr, "\n"
                         "ERROR: Missing input files!\n"
                         "Type \"%s --help\" to display usage.\n\n", argv[0]);
            return 2;
        }
    }
    catch(TCLAP::ArgException &e)   // catch any exceptions
    {
        std::cerr << "Error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 2;
    }

    std::fprintf(stderr, "============================================================================\n"
                         "Wheel maker tool by Wohlstand Russiche Fuchs. Version 1.0\n"
                         "============================================================================\n"
                         "This program is distributed under the MIT license \n"
                         "============================================================================\n");
    std::fflush(stderr);

    FreeImage_Initialise();

    for(const std::string &file : files)
    {
        std::string newImage = file;
        size_t extSep = newImage.find_last_of('.');
        if(extSep != std::string::npos)
        {
            newImage.erase(extSep);
            newImage.append("-wheel.gif");
        }

        std::fprintf(stderr, "Processing %s: .", file.c_str());
        std::fflush(stderr);

        FREE_IMAGE_FORMAT formato = FreeImage_GetFileType(file.c_str(), 0);
        if(formato  == FIF_UNKNOWN)
        {
            std::fprintf(stderr, "ERROR: Unknown file format for %s\n", file.c_str());
            std::fflush(stderr);
            break;
        }
        FIBITMAP *img = FreeImage_Load(formato, file.c_str());
        if(!img)
        {
            std::fprintf(stderr, "ERROR: Failed load the file %s\n", file.c_str());
            std::fflush(stderr);
            break;
        }

        if(FreeImage_GetBPP(img) != 32)
        {
            printDot();
            FIBITMAP *n = FreeImage_ConvertTo32Bits(img);
            FreeImage_Unload(img);
            img = n;
        }

        FreeImage_SetTransparent(img, TRUE);

        uint32_t w = FreeImage_GetWidth(img), nw = w, l = 0, wh;
        uint32_t h = FreeImage_GetHeight(img), nh = h, t = 0, hh;

        if(w > h)
        {
            nw = h;
            l = (w / 2) - (nw / 2);
        }
        else if(w < h)
        {
            nh = w;
            t = (h / 2) - (nh / 2);
        }

        wh = nw / 2;
        hh = nh / 2;

        printDot();
        FIBITMAP *square = FreeImage_Copy(img, l, t, l + nw, t + nh);
        FreeImage_Unload(img);

        if(FreeImage_GetWidth(square) > maxWidth || FreeImage_GetHeight(square) > maxWidth)
        {
            printDot();
            FIBITMAP *neu = FreeImage_Rescale(square, maxWidth, maxWidth, FILTER_LANCZOS3);
            FreeImage_Unload(square);
            square = neu;
            wh = maxWidth / 2;
            hh = maxWidth / 2;
        }

        FIMULTIBITMAP *multi = FreeImage_OpenMultiBitmap(FIF_GIF, newImage.c_str(), TRUE, FALSE);

        double inv = -1;
        if(angleStep < 0)
        {
            inv = 1;
            angleStep *= -1;
        }

        if(angleStep <= 1.0)
            angleStep = 1.0;

        DWORD dwFrameTime = gifDelay;

        for(double angle = 0; angle < 360.0; angle += angleStep)
        {
            printDot();

            RGBQUAD q;
            q.rgbRed = 0;
            q.rgbBlue = 0xFF;
            q.rgbGreen = 0;
            q.rgbReserved = 0;

            FIBITMAP *rot = FreeImage_RotateEx(square, angle * inv,
                                               0, 0,
                                               wh, hh, FALSE);

            for(uint32_t x = 0; x < nw; ++x)
            {
                for(uint32_t y = 0; y < nh; ++y)
                {
                    if(isPointInCircle(x, y, wh, hh, hh))
                        continue;

                    RGBQUAD q;
                    q.rgbRed = 0;
                    q.rgbBlue = 0xFF;
                    q.rgbGreen = 0;
                    q.rgbReserved = 0;
                    FreeImage_SetPixelColor(rot, x, y, &q);
                }
            }

            FIBITMAP *image8 = genhDIB8bpp(rot);
            FreeImage_Unload(rot);

            FreeImage_SetMetadata(FIMD_ANIMATION, image8, NULL, NULL);
            FITAG *tag = FreeImage_CreateTag();
            if(tag)
            {
                FreeImage_SetTagKey(tag, "FrameTime");
                FreeImage_SetTagType(tag, FIDT_LONG);
                FreeImage_SetTagCount(tag, 1);
                FreeImage_SetTagLength(tag, 4);
                FreeImage_SetTagValue(tag, &dwFrameTime);
                FreeImage_SetMetadata(FIMD_ANIMATION, image8, FreeImage_GetTagKey(tag), tag);
                FreeImage_DeleteTag(tag);
            }
            FreeImage_AppendPage(multi, image8);
            FreeImage_Unload(image8);
        }

        std::fprintf(stderr, "DONE!\n");
        std::fflush(stderr);

        FreeImage_CloseMultiBitmap(multi);

        FreeImage_Unload(square);
    }

    FreeImage_DeInitialise();

    return 0;
}
