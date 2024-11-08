//YES you need to include all of these...
#include <stdio.h>
#include <string.h>
#include <math.h>
#define NANOSVG_ALL_COLOR_KEYWORDS	// Include full list of color keywords.
#define NANOSVG_IMPLEMENTATION		// Expands implementation
#include "nanosvg.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdexcept>
#include <filesystem>

#define DEFAULT -1

/**
 * Call this function to convert an SVG to OpenGL coordinates.
 * The format of the output is: x, y, r, g, b, a.
 * Complex SVGs utilizing new features may not be supported. Test first please.
 * 
 * @param SVGFilename filename of your SVG.
 * @param resultFilename filename of where you want the outputted vertices to be stored in. Must be a .txt file.
 * @param unit unit of your output. Should be one of: 'px', 'pt', 'pc' 'mm', 'cm', or 'in'
 * @param dpi dpi of the output. 96 is the recommended default of the nanosvg author/s.
 * @param xOffset xOffset of each vertices in OpenGL coordinates. A value of 1 will move all the vertices to the right by 1 unit
 * @param yOffset yOffset of each vertices in OpenGL coordinates. A value of 1 will move all the vertices up by 1 unit
 * @param scale the resulting x and y coordinates will be divided by this value. So a value of 1 will cover the entire OpenGL window, while a value of 0.5 will cover half of it.
 */
void SVGToGL(const char* SVGFilename, const char* resultFilename = "vertices.txt", const char* unit = "px", float dpi = 96, float xOffset = 0, float yOffset = 0, float scale = 1);


//This is literally how you use it:
int main() {
    SVGToGL("bird1_mapped.svg", "vertices.txt", "px", 96, 1.f, 1.f, .5f);

    return 0;
}


//no need to check, unless you want to understand how it works
void linkComponents(float xOffset, float yOffset, float scale, float* result, float x, float y, float maxWidth, float maxHeight, float r, float g, float b, float a) {
    result[0] = ((2 * (x/maxWidth) - 1) + xOffset) * scale;
    result[1] = ((1 - 2 * (y/maxHeight)) + yOffset) * scale;
    result[2] = r;
    result[3] = g;
    result[4] = b;
    result[5] = a;
}

void SVGToGL(const char* SVGFilename, const char* resultFilename, const char* unit, float dpi, float xOffset, float yOffset, float scale) {
    std::filesystem::path SVGPath = SVGFilename;
    if (SVGPath.extension() != ".svg") {
        throw std::invalid_argument("Argument: SVGFilename must have a .svg extension");
    }

    std::filesystem::path resultPath = resultFilename;
    if (resultPath.extension() != ".txt") {
        throw std::invalid_argument("Argument: resultFilename must have a .txt extension");
    }

    if (!std::filesystem::exists(SVGFilename)) {
        throw std::invalid_argument("Argument: SVGFilename must exist! (It doesn't exist)");
    }

    struct NSVGimage* image;
    image = nsvgParseFromFile(SVGFilename, unit, dpi);

    
    float maxWidth = image->width;
    float maxHeight = image->height;

    std::ofstream file(resultFilename);

    for (NSVGshape* shape = image->shapes; shape != NULL; shape = shape->next) {
        float a, r, g, b;
        if (shape->fill.type == NSVG_PAINT_COLOR) {
            //Normalize colors
            unsigned int color = shape->fill.color;

            a = ((color & 0xFF000000) >> 24) / 255.f;
            b = ((color & 0xFF0000) >> 16) / 255.f;
            g = ((color & 0xFF00) >> 8) / 255.f;
            r =  (color & 0xFF) / 255.f;
        }

        for (NSVGpath* path = shape->paths; path != NULL; path = path->next) {
            for (int i = 0; i < path->npts-1; i += 3) {
                float* p = &path->pts[i*2];

                float output[6]; 
                linkComponents(xOffset, yOffset, scale, output, p[0], p[1], maxWidth, maxHeight, r, g, b, a);

                file << output[0] << " " << output[1] << " " << output[2] << " " << output[3] << " " << output[4] << " " << output[5] << "\n";
            }
        }
    }
    file.close();
    
    
    nsvgDelete(image);
}