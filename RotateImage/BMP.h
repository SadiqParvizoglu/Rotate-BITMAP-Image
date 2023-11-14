#pragma once
#include <bits/stdc++.h>

#pragma pack(push, 1)
struct BMPFileHeader
{
    uint16_t signature{0x4D42};
    uint32_t filesize{0};
    uint16_t reserved1{0};
    uint16_t reserved2{0};
    uint32_t offsetdata{0};
};

struct BMPInfoHeader
{
    uint32_t size{0};
    int32_t width{0};
    int32_t height{0};
    uint16_t planes{1};
    uint16_t bitcount{0};
    uint32_t compression{0};
    uint32_t imagesize{0};
    int32_t xpixels_perM{0};
    int32_t ypixels_perM{0};
    uint32_t colorsused{0};
    uint32_t colorsimportant{0};
};

struct BMPColorHeader
{
    uint32_t redmask{0x00ff0000};
    uint32_t greenmask{0x0000ff00};
    uint32_t bluemask{0x000000ff};
    uint32_t alphamask{0xff000000};
    uint32_t colorspacetype{0x73524742};
    uint32_t unused[16]{0};
};
#pragma pack(pop)

struct BMP
{
    BMPFileHeader bmp_fileheader;
    BMPInfoHeader bmp_infoheader;
    BMPColorHeader bmp_colorheader;
    std::vector<uint8_t> data;
    //Mövcud şəkli çağırmaq üçün konstruktor
    BMP (const char *fname) { read(fname); }
    //Şəkli oxumaq üçün funksiya
    void read(const char *fname)
    {
        std::ifstream inp{fname,std::ios_base::binary};
        if (inp)
        {
            inp.read((char*)&bmp_fileheader,sizeof(bmp_fileheader));
            if(bmp_fileheader.signature!=0x4D42)
                throw std::runtime_error("Fayl tanınmadı.");
            inp.read((char*)&bmp_infoheader,sizeof(bmp_infoheader));
            if(bmp_infoheader.bitcount==32)
            {
                if(bmp_infoheader.size>=(sizeof(BMPInfoHeader)+sizeof(BMPColorHeader)))
                {
                    inp.read((char*)&bmp_colorheader,sizeof(bmp_colorheader));
                    checkcolorheader(bmp_colorheader);
                }
                else
                {
                    std::cerr<<"Verilmiş \""<<fname<<"\" faylı bitmask informasiyası ehtiva etmir.\n";
                    throw std::runtime_error("Fayl tanınmadı.");
                }
            }
            inp.seekg(bmp_fileheader.offsetdata,inp.beg);
            if(bmp_infoheader.bitcount==32)
            {
                 bmp_infoheader.size=sizeof(BMPInfoHeader)+sizeof(BMPColorHeader);
                    bmp_fileheader.offsetdata=sizeof(BMPFileHeader)+sizeof(BMPInfoHeader)+sizeof(BMPColorHeader);
            }
            else
                bmp_infoheader.size=sizeof(BMPInfoHeader); bmp_fileheader.offsetdata=sizeof(BMPFileHeader)+sizeof(BMPInfoHeader);
            bmp_fileheader.filesize=bmp_fileheader.offsetdata;
            if (bmp_infoheader.height<0)
                throw std::runtime_error("Proqram yalnız sol alt küncdə başlanğıcı olan BMP şəkillərini emal edə bilər!");
            data.resize(bmp_infoheader.width*bmp_infoheader.height*bmp_infoheader.bitcount/8);
            if (bmp_infoheader.width%4==0)
            {
                inp.read((char*)data.data(),data.size());
                bmp_fileheader.filesize+=static_cast<uint32_t>(data.size());
            }
            else
            {
                rowstride=bmp_infoheader.width*bmp_infoheader.bitcount/8;
                uint32_t newstride=makestridealigned(4);
                std::vector<uint8_t> padding_row(newstride-rowstride);
                for (int y=0;y<bmp_infoheader.height;++y)
                {
                    inp.read((char*)(data.data()+rowstride*y),rowstride);
                    inp.read((char*)padding_row.data(),padding_row.size());
                }
                bmp_fileheader.filesize+=static_cast<uint32_t>(data.size())+bmp_infoheader.height*static_cast<uint32_t>(padding_row.size());
            }
        }
        else throw std::runtime_error("Faylı açmaq mümkün olmadı.");
    }
    //Verilmiş ölçüdə şəkil yaratmaq üçün konstruktor
    BMP(int32_t width,int32_t height,bool has_alpha=true)
    {
        if (width<=0 || height<=0)
            throw std::runtime_error("The image width and height must be positive numbers.");
        bmp_infoheader.width=width; bmp_infoheader.height=height;
        if (has_alpha)
        {
            bmp_infoheader.size=sizeof(BMPInfoHeader)+sizeof(BMPColorHeader);
            bmp_fileheader.offsetdata=sizeof(BMPFileHeader)+sizeof(BMPInfoHeader)+sizeof(BMPColorHeader);
            bmp_infoheader.bitcount=32;
            bmp_infoheader.compression=3;
            rowstride=width*4;
            data.resize(rowstride*height);
            bmp_fileheader.filesize=bmp_fileheader.offsetdata+data.size();
        }
        else
        {
            bmp_infoheader.size=sizeof(BMPInfoHeader);
            bmp_fileheader.offsetdata=sizeof(BMPFileHeader)+sizeof(BMPInfoHeader);
            bmp_infoheader.bitcount=24;
            bmp_infoheader.compression=0;
            rowstride=width*3;
            data.resize(rowstride*height);
            uint32_t newstride=makestridealigned(4);
            bmp_fileheader.filesize=bmp_fileheader.offsetdata+static_cast<uint32_t>(data.size())+bmp_infoheader.height*(newstride-rowstride);
        }
    }
    //Şəkili yaddaşda saxlayan funksiya
    void write(const char *fname)
    {
        std::ofstream of{fname,std::ios_base::binary};
        if (of)
        {
            if (bmp_infoheader.bitcount==32)
                writeheadersanddata(of);
            else if (bmp_infoheader.bitcount==24)
            {
                if (bmp_infoheader.width%4==0) writeheadersanddata(of);
                else
                {
                    uint32_t newstride=makestridealigned(4);
                    std::vector<uint8_t> padding_row(newstride-rowstride);
                    writeheaders(of);
                    for (int y=0;y<bmp_infoheader.height;++y)
                    {
                        of.write((const char*)(data.data()+rowstride*y),rowstride);
                        of.write((const char*)padding_row.data(),padding_row.size());
                    }
                }
            }
            else throw std::runtime_error("Proqram sadəcə 24 və 32 bitlik bitmapları emal edə bilir");
        }
        else throw std::runtime_error("Faylı yaratmaq mümkün olmadı");
    }
    //Şəkildə verilmiş düzbucaqlı sahəni verilmiş rəng ilə dolduran funksiya
    void setarea(uint32_t x0,uint32_t y0,uint32_t w,uint32_t h,uint8_t B,uint8_t G,uint8_t R,uint8_t A)
    {
        if (x0+w>(uint32_t)bmp_infoheader.width || y0+h>(uint32_t)bmp_infoheader.height)
            throw std::runtime_error("Verilmiş düzbucaqlı sahənin ölçüləri şəkilin ölçülərini aşır!");
        uint32_t channels=bmp_infoheader.bitcount/8;
        for (uint32_t y=y0;y<y0+h;++y)
        {
            for (uint32_t x=x0;x<x0+w;++x)
            {       
                data[channels*(y*bmp_infoheader.width+x)+0]=B;
                data[channels*(y*bmp_infoheader.width+x)+1]=G;
                data[channels*(y*bmp_infoheader.width+x)+2]=R;
                if (channels==4)
                    data[channels*(y*bmp_infoheader.width+x)+3]=A;
            }
        }
    }
    //Şəkildə verilmiş koordinatlardakı pikselə verilmiş rəngi mənimsədən funksiya
    void setpixel(uint32_t x0,uint32_t y0,uint8_t B,uint8_t G,uint8_t R,uint8_t A)
    {
        if (x0>=(uint32_t)bmp_infoheader.width || y0>=(uint32_t)bmp_infoheader.height || x0<0 || y0<0)
            throw std::runtime_error("Verilmiş nöqtə şəkil daxilində yerləşmir!");
        uint32_t channels = bmp_infoheader.bitcount/8;
        data[channels*(y0*bmp_infoheader.width+x0)+0]=B;
        data[channels*(y0*bmp_infoheader.width+x0)+1]=G;
        data[channels*(y0*bmp_infoheader.width+x0)+2]=R;
        if (channels==4)
            data[channels*(y0*bmp_infoheader.width+x0)+3]=A;
    }
    //Şəkildə verilmiş koordinatlardakı pikselin rəngini qaytaran funksiya
    std::vector<uint8_t> getpixel(uint32_t x0,uint32_t y0)
    {
        std::vector<uint8_t> pixel;
        if (x0>=(uint32_t)bmp_infoheader.width || y0>=(uint32_t)bmp_infoheader.height || x0<0 || y0<0)
            throw std::runtime_error("Verilmiş nöqtə şəkil daxilində yerləşmir!");
        uint32_t channels=bmp_infoheader.bitcount/8;
        pixel.push_back(data[channels*(y0*bmp_infoheader.width+x0)+0]);
        pixel.push_back(data[channels*(y0*bmp_infoheader.width+x0)+1]);
        pixel.push_back(data[channels*(y0*bmp_infoheader.width+x0)+2]);
        if (channels==4)
            pixel.push_back(data[channels*(y0*bmp_infoheader.width+x0)+3]);
        return pixel;
    }
    //Bağlı sahələr
private:
    uint32_t rowstride{0};
    //alignstride qiymətinə bölünənə qədər rowstride dəyişəninə 1 əlavə edilir
    uint32_t makestridealigned(uint32_t alignstride)
    {
        uint32_t newstride=rowstride;
        while (newstride%alignstride!=0)
            newstride++;
        return newstride;
    }
    void writeheaders(std::ofstream &of)
    {
        of.write((const char*)&bmp_fileheader,sizeof(bmp_fileheader));
        of.write((const char*)&bmp_infoheader,sizeof(bmp_infoheader));
        if(bmp_infoheader.bitcount==32)
            of.write((const char*)&bmp_colorheader,sizeof(bmp_colorheader));
    }
    void writeheadersanddata(std::ofstream &of)
    {
        writeheaders(of);
        of.write((const char*)data.data(),data.size());
    }
    void checkcolorheader(BMPColorHeader &bmp_colorheader)
    {
        BMPColorHeader expectedcolorheader;
        if(expectedcolorheader.redmask!=bmp_colorheader.redmask || expectedcolorheader.bluemask!=bmp_colorheader.bluemask || expectedcolorheader.greenmask!=bmp_colorheader.greenmask || expectedcolorheader.alphamask!=bmp_colorheader.alphamask)
            throw std::runtime_error("Piksel verilənləri BGRA formatında olmalıdır.");
        if(expectedcolorheader.colorspacetype!=bmp_colorheader.colorspacetype)
            throw std::runtime_error("Rəng məkanı sRGB olmalıdır.");
    }
};
