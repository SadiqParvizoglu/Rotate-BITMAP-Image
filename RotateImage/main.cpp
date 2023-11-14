#include <bits/stdc++.h>
#include "BMP.h"
using namespace std;

double min(double a,double b,double c,double d)
{
    return min(min(a,b),min(c,d));
}

double max(double a,double b,double c,double d)
{
    return max(max(a,b),max(c,d));
}

BMP rotateImage(BMP bmp,double alpha)
{
    alpha*=M_PI/180;
    int width=bmp.bmp_infoheader.width,height=bmp.bmp_infoheader.height;
    vector<uint8_t> data=bmp.data;
    double xm=width/2.0,ym=height/2.0;
    pair<double,double> p1,p2,p3,p4,c1,c2,c3,c4;
    p1={(0-xm)*cos(alpha)-(0-ym)*sin(alpha)+xm,(0-xm)*sin(alpha)+(0-ym)*cos(alpha)+ym};
    p2={(width-xm)*cos(alpha)-(0-ym)*sin(alpha)+xm,(width-xm)*sin(alpha)+(0-ym)*cos(alpha)+ym};
    p3={(0-xm)*cos(alpha)-(height-ym)*sin(alpha)+xm,(0-xm)*sin(alpha)+(height-ym)*cos(alpha)+ym};
    p4={(width-xm)*cos(alpha)-(height-ym)*sin(alpha)+xm,(width-xm)*sin(alpha)+(height-ym)*cos(alpha)+ym};
    c1={min(p1.first,p2.first,p3.first,p4.first),min(p1.second,p2.second,p3.second,p4.second)};
    c2={max(p1.first,p2.first,p3.first,p4.first),min(p1.second,p2.second,p3.second,p4.second)};
    c3={min(p1.first,p2.first,p3.first,p4.first),max(p1.second,p2.second,p3.second,p4.second)};
    c4={max(p1.first,p2.first,p3.first,p4.first),max(p1.second,p2.second,p3.second,p4.second)};
    double sx=-c1.first,sy=-c1.second;
    int newwidth=int(c4.first+sx+1),newheight=int(c4.second+sy+1);
    BMP newbmp(newwidth,newheight,false);
    for (int i=0;i<newheight;i++)
    {
        for (int j=0;j<newwidth;j++)
        {
            int xs=j-sx,ys=i-sy;
            int x=(xs-xm)*cos(alpha)+(ys-ym)*sin(alpha)+xm;
            int y=(ys-ym)*cos(alpha)-(xs-xm)*sin(alpha)+ym;
            if (x>0 && x<width && y>0 && y<height)
            {
                newbmp.data[3*(i*newwidth+j)]=bmp.data[3*(x*width+y)];
                newbmp.data[3*(i*newwidth+j)+1]=bmp.data[3*(x*width+y)+1];
                newbmp.data[3*(i*newwidth+j)+2]=bmp.data[3*(x*width+y)+2];
            }
        }
    }
    return newbmp;
}

int main()
{
    BMP bmp("image.bmp");
    BMP rotated=rotateImage(bmp,30);
    rotated.write("result.bmp");
    return 0;
}
