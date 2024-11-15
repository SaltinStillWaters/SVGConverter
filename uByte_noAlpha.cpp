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
float getGLWidth(const char* sourceFilename);

//This is literally how you use it:
int main() {
    SVGToGL("lowerPipe.svg", "botPipe.txt", "px", 96, 1.169068);

    std::cout << getGLWidth("upperPipe.txt");
    return 0;
}

float getGLWidth(const char* sourceFilename) {
    std::ifstream file(sourceFilename);

    std::string line;
    float minX = 9999;
    float maxX = -9999;

    while(std::getline(file, line)) {
        std::stringstream temp(line);
        std::string xTemp;
        std::getline(temp, xTemp, ' ');
        
        float x = std::stof(xTemp);
        if (x < minX) {
            minX = x;
        }
        if (x > maxX) {
            maxX = x;
        }
    }

    std::cout << minX << ' ' << maxX;
    return maxX - minX;
}

//no need to check, unless you want to understand how it works
void linkComponents(float xOffset, float yOffset, float scale, float* result, unsigned int* color, float x, float y, float maxWidth, float maxHeight, float r, float g, float b, float a) {
    result[0] = ((2 * (x/maxWidth) - 1) + xOffset) * scale;
    result[1] = ((1 - 2 * (y/maxHeight)) + yOffset) * scale;
    color[0] = r;
    color[1] = g;
    color[2] = b;
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
        unsigned int a, r, g, b;
        if (shape->fill.type == NSVG_PAINT_COLOR) {
            //Normalize colors
            unsigned int color = shape->fill.color;

            a = ((color & 0xFF000000) >> 24);
            b = ((color & 0xFF0000) >> 16);
            g = ((color & 0xFF00) >> 8);
            r =  (color & 0xFF);
        }

        for (NSVGpath* path = shape->paths; path != NULL; path = path->next) {
            for (int i = 0; i < path->npts-1; i += 3) {
                float* p = &path->pts[i*2];

                float output[2];
                unsigned int color[3]; 
                linkComponents(xOffset, yOffset, scale, output, color, p[0], p[1], maxWidth, maxHeight, r, g, b, a);

                file << output[0] << " " << output[1] << " " << color[0] << " " << color[1] << " " << color[2] << "\n";
            }
        }
    }
    file.close();
    
    
    nsvgDelete(image);
}