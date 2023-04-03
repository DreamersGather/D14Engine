#include "Common/Precompile.h"

#include <iostream>

#include "UIKit/ColorUtils.h"

using namespace d14engine;
using namespace d14engine::uikit::color_utils;

std::ostream& operator<<(std::ostream& out, const iRGB& rgb)
{
    out << "RGB: ( " <<
        std::setw(3) << rgb.R << ", " <<
        std::setw(3) << rgb.G << ", " <<
        std::setw(3) << rgb.B << " )";
    return out;
}

std::ostream& operator<<(std::ostream& out, const iHSB& hsb)
{
    out << "HSB: ( " <<
        std::setw(3) << hsb.H << ", " <<
        std::setw(3) << hsb.S << ", " <<
        std::setw(3) << hsb.B << " )";
    return out;
}

void print(StrParam name, const iRGB& rgb)
{
    std::cout << name << std::endl <<
        rgb << std::endl << rgb2hsb(rgb) << std::endl;
}

void print(StrParam name, const iHSB& hsb)
{
    std::cout << name << std::endl <<
        hsb2rgb(hsb) << std::endl << hsb << std::endl;
}

int main()
{
    print("Black",          iRGB{   0,   0,   0 });
    print("Black",          iHSB{   0,   0,   0 });

    print("White",          iRGB{ 255, 255, 255 });
    print("White",          iHSB{   0,   0, 100 });

    print("Firebrick",      iRGB{ 178,  34,  34 });
    print("Firebrick",      iHSB{   0,  81,  70 });

    print("Navy",           iRGB{   0,   0, 128 });
    print("Navy",           iHSB{ 240, 100,  50 });

    print("Chocolate",      iRGB{ 210, 105,  30 });
    print("Chocolate",      iHSB{  25,  86,  82 });

    // .......... The world is colorful ..........
}
