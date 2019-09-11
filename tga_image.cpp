// http://www.paulbourke.net/dataformats/tga/

#include "tga_image.h"

void tga_32bit_image::MergeBytes(pixel *pxl, unsigned char *p, int bytes)
{
    if (bytes == 4) {
        pxl->r = p[2];
        pxl->g = p[1];
        pxl->b = p[0];
        pxl->a = p[3];
    } else if (bytes == 3) {
        pxl->r = p[2];
        pxl->g = p[1];
        pxl->b = p[0];
        pxl->a = 0;
    } else if (bytes == 2) {
        pxl->r = (p[1] & 0x7c) << 1;
        pxl->g = ((p[1] & 0x03) << 6) | ((p[0] & 0xe0) >> 2);
        pxl->b = (p[0] & 0x1f) << 3;
        pxl->a = (p[1] & 0x80);
    }
}


void tga_32bit_image::save(const char *const filename)
{
    FILE *fptr;
    
    /* Write the result as a uncompressed TGA */
    if ((fptr = fopen(filename,"wb")) == NULL) {
        fprintf(stderr,"Failed to open outputfile\n");
        exit(-1);
    }
    putc(0,fptr);
    putc(0,fptr);
    putc(2,fptr);                         /* uncompressed RGB */
    putc(0,fptr); putc(0,fptr);
    putc(0,fptr); putc(0,fptr);
    putc(0,fptr);
    putc(0,fptr); putc(0,fptr);           /* X origin */
    putc(0,fptr); putc(0,fptr);           /* y origin */
    putc((hdr.width & 0x00FF),fptr);
    putc((hdr.width & 0xFF00) / 256,fptr);
    putc((hdr.height & 0x00FF),fptr);
    putc((hdr.height & 0xFF00) / 256,fptr);
    putc(32,fptr);
    putc(0,fptr);
    for (size_t i=0;i<hdr.height*hdr.width;i++) {
        putc(pixels[i].b,fptr);
        putc(pixels[i].g,fptr);
        putc(pixels[i].r,fptr);
        putc(pixels[i].a,fptr);
    }
    
    fclose(fptr);
}

void tga_32bit_image::load_single_channel_float(unsigned short width, unsigned short height, const vector<float>& buffer)
{
	hdr.datatypecode = 2;
	hdr.x_origin = 0;
	hdr.y_origin = 0;
	hdr.width = width;
	hdr.height = height;
	hdr.bitsperpixel = 32;

	pixels.resize(static_cast<size_t>(width) * static_cast<size_t>(height));

	float largest = 0;

	for (size_t i = 0; i < buffer.size(); i++)
	{
		if (buffer[i] > largest)
			largest = buffer[i];
	}

	for (size_t i = 0; i < buffer.size(); i++)
	{
		unsigned char c = static_cast<unsigned char>(buffer[i] / largest * 255.0f);

		pixels[i].r = c;
		pixels[i].g = c;
		pixels[i].b = c;
		pixels[i].a = 255;
	}
}


void tga_32bit_image::load(const char *const filename)
{
    int n=0,i,j;
    int bytes2read,skipover = 0;
    unsigned char p[5];
    FILE *fptr;
    
        
    
    
    /* Open the file */
    if ((fptr = fopen(filename,"rb")) == NULL) {
        fprintf(stderr,"File open failed %s\n", filename);
        return;
    }
    
    /* Display the header fields */
    hdr.idlength = fgetc(fptr);
    hdr.colourmaptype = fgetc(fptr);
    hdr.datatypecode = fgetc(fptr);
    fread(&hdr.colourmaporigin,2,1,fptr);
    fread(&hdr.colourmaplength,2,1,fptr);
    hdr.colourmapdepth = fgetc(fptr);
    fread(&hdr.x_origin,2,1,fptr);
    fread(&hdr.y_origin,2,1,fptr);
    fread(&hdr.width,2,1,fptr);
    fread(&hdr.height,2,1,fptr);
    hdr.bitsperpixel = fgetc(fptr);
    hdr.imagedescriptor = fgetc(fptr);
    
    pixels.resize(hdr.width*hdr.height);
    this->width = hdr.width;
    this->height = hdr.height;
    
    
    for (i=0;i<hdr.width*hdr.height;i++) {
        pixels[i].r = 0;
        pixels[i].g = 0;
        pixels[i].b = 0;
        pixels[i].a = 0;
    }
    
    /* Skip over unnecessary stuff */
    skipover += hdr.idlength;
    skipover += hdr.colourmaptype * hdr.colourmaplength;
    //    fprintf(stderr,"Skip over %d bytes\n",skipover);
    fseek(fptr,skipover,SEEK_CUR);
    
    /* Read the image */
    bytes2read = hdr.bitsperpixel / 8;
    while (n < hdr.width * hdr.height) {
        if (hdr.datatypecode == 2) {                     /* Uncompressed */
            if (fread(p,1,bytes2read,fptr) != bytes2read) {
                //              fprintf(stderr,"Unexpected end of file at pixel %d\n",i);
                return;
            }
            MergeBytes(&(pixels[n]),p,bytes2read);
            n++;
        } else if (hdr.datatypecode == 10) {             /* Compressed */
            if (fread(p,1,bytes2read+1,fptr) != bytes2read+1) {
                //              fprintf(stderr,"Unexpected end of file at pixel %d\n",i);
                return;
            }
            j = p[0] & 0x7f;
            MergeBytes(&(pixels[n]),&(p[1]),bytes2read);
            n++;
            if (p[0] & 0x80) {         /* RLE chunk */
                for (i=0;i<j;i++) {
                    MergeBytes(&(pixels[n]),&(p[1]),bytes2read);
                    n++;
                }
            } else {                   /* Normal chunk */
                for (i=0;i<j;i++) {
                    if (fread(p,1,bytes2read,fptr) != bytes2read) {
                        //                      fprintf(stderr,"Unexpected end of file at pixel %d\n",i);
                        return;
                    }
                    MergeBytes(&(pixels[n]),p,bytes2read);
                    n++;
                }
            }
        }
    }
    
    fclose(fptr);
}

