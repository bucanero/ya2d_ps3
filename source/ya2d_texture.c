#include "ya2d_texture.h"

//Simple draw
	//Non swizzled
		//GU_SPRITES: 28.54 FPS
		//GU_TRIANGLE_STRIP: 37.36 FPS
	//Swizzled
		//GU_SPRITES: 89.91 FPS
		//GU_TRIANGLE_STRIP: 110.34 FPS
//Fast draw
	//Non swizzled
		//GU_SPRITES: 148.26 FPS
		//GU_TRIANGLE_STRIP: 142.57 FPS
	//Swizzled
		//GU_SPRITES: 264.74 FPS
		//GU_TRIANGLE_STRIP: 263.21 FPS



	ya2d_Texture* ya2d_createTexture(int w, int h, int format)
	{
		ya2d_Texture* texp = (ya2d_Texture*)malloc(sizeof(ya2d_Texture));
		
		texp->imageWidth  = w;
		texp->imageHeight = h;
		texp->textureWidth  = w;//next_pow2(w);
		texp->textureHeight = h;//next_pow2(h);
		texp->centerX = (int)(w/2);
		texp->centerY = (int)(h/2);

		texp->format = format;
		texp->isSwizzled = 0;
		texp->hasAlpha   = 1;
		
		switch(format)
		{
		case TINY3D_TEX_FORMAT_L8:
			texp->rowBytes = texp->textureWidth;
			break;
		case TINY3D_TEX_FORMAT_A1R5G5B5:
		case TINY3D_TEX_FORMAT_A4R4G4B4:
		case TINY3D_TEX_FORMAT_R5G6B5:
			texp->rowBytes = texp->textureWidth * 2;
			break;
		case TINY3D_TEX_FORMAT_A8R8G8B8:
		default:
			texp->rowBytes = texp->textureWidth * 4;
			break;
		}
			
		texp->dataLength = texp->rowBytes * texp->textureHeight;
		texp->data = ya2d_texturePointer;
		texp->textureOffset = tiny3d_TextureOffset(texp->data);
		ya2d_texturePointer += (texp->dataLength + 3) & ~3;
		return texp;
	}
	
	void ya2d_swizzleTexture(ya2d_Texture *texp)
	{
		if(texp->isSwizzled || (texp->textureWidth < YA2D_TEXTURE_SLICE && texp->textureHeight < YA2D_TEXTURE_SLICE)) return;
		void *swizzledData = malloc(texp->dataLength);
		swizzle_fast(swizzledData, texp->data, texp->rowBytes, texp->textureHeight);
		memcpy(texp->data, swizzledData, texp->dataLength);
		free(swizzledData);
		texp->isSwizzled = 1;
	}
	
	inline void ya2d_setTexture(ya2d_Texture *texp)
	{
		tiny3d_SetTexture(0, texp->textureOffset, texp->imageWidth, texp->imageHeight, texp->rowBytes, texp->format, TEXTURE_LINEAR);
	}

    void ya2d_freeTexture(ya2d_Texture *texp)
    {
        if(texp->data != NULL)
        {
			free(texp);
			texp = NULL;
        }
    }

    void ya2d_drawTexture(ya2d_Texture *texp, int x, int y)
    {
		if(!texp->data) return;

		ya2d_setTexture(texp);
		
		tiny3d_SetPolygon(TINY3D_TRIANGLE_STRIP);
			tiny3d_VertexPos(x, y, 65535);
			tiny3d_VertexTexture(0.0f, 0.0f);
			
			tiny3d_VertexPos(x, y+texp->imageHeight, 65535);
			tiny3d_VertexTexture(0.0f, 1.0f);
			
			tiny3d_VertexPos(x+texp->imageWidth, y, 65535);
			tiny3d_VertexTexture(1.0f, 0.0f);
			
			tiny3d_VertexPos(x+texp->imageWidth, y+texp->imageHeight, 65535);
			tiny3d_VertexTexture(1.0f, 1.0f);
		tiny3d_End();
    }
     

	void ya2d_drawRotateTexture(ya2d_Texture *texp, int x, int y, float angle)
	{
		if(!texp->data) return;

		ya2d_setTexture(texp);
			
		MATRIX matrix = MatrixTranslation(x + (float)texp->centerX, y + (float)texp->centerY, 0.0f);
		matrix = MatrixMultiply(MatrixRotationZ(angle), matrix);
		
		tiny3d_SetMatrixModelView(&matrix);
		
		tiny3d_SetPolygon(TINY3D_TRIANGLE_STRIP);
			tiny3d_VertexPos((float)-texp->centerX, (float)-texp->centerY, 65535);
			tiny3d_VertexTexture(0.0f, 0.0f);
			
			tiny3d_VertexPos((float)-texp->centerX, (float)texp->centerY, 65535);
			tiny3d_VertexTexture(0.0f, 1.0f);
			
			tiny3d_VertexPos((float)texp->centerX, (float)-texp->centerY, 65535);
			tiny3d_VertexTexture(1.0f, 0.0f);
			
			tiny3d_VertexPos((float)texp->centerX, (float)texp->centerY, 65535);
			tiny3d_VertexTexture(1.0f, 1.0f);
		tiny3d_End();	
		
		tiny3d_SetMatrixModelView(NULL); // Identity
	}


	void ya2d_drawBlendTexture(ya2d_Texture *texp, int x, int y, u32 color)
	{

	}


	ya2d_Texture* ya2d_loadPNGfromFile(char* filename)
	{
		pngData png;
		pngLoadFromFile(filename, &png);
		if(png.bmp_out)
		{
			ya2d_Texture *texp = ya2d_createTexture(png.width, png.height, TINY3D_TEX_FORMAT_A8R8G8B8);
			memcpy(texp->data, png.bmp_out, texp->dataLength);
			free(png.bmp_out);	
			return texp;
		}
		return NULL;
	}

	ya2d_Texture* ya2d_loadPNGfromBuffer(void *buffer, u32 buf_size)
	{
		pngData png;
		pngLoadFromBuffer(buffer, buf_size, &png);
		if(png.bmp_out)
		{
			ya2d_Texture *texp = ya2d_createTexture(png.width, png.height, TINY3D_TEX_FORMAT_A8R8G8B8);
			memcpy(texp->data, png.bmp_out, texp->dataLength);
			free(png.bmp_out);	
			return texp;
		}
		return NULL;
	}

	ya2d_Texture* ya2d_loadJPGfromFile(char* filename)
	{
		jpgData jpg;
		jpgLoadFromFile(filename, &jpg);
		if(jpg.bmp_out)
		{
			ya2d_Texture *texp = ya2d_createTexture(jpg.width, jpg.height, TINY3D_TEX_FORMAT_A8R8G8B8);
			memcpy(texp->data, jpg.bmp_out, texp->dataLength);
			free(jpg.bmp_out);	
			return texp;
		}
		return NULL;
	}

	ya2d_Texture* ya2d_loadJPGfromBuffer(void *buffer, u32 buf_size)
	{
		jpgData jpg;
		jpgLoadFromBuffer(buffer, buf_size, &jpg);
		if(jpg.bmp_out)
		{
			ya2d_Texture *texp = ya2d_createTexture(jpg.width, jpg.height, TINY3D_TEX_FORMAT_A8R8G8B8);
			memcpy(texp->data, jpg.bmp_out, texp->dataLength);
			free(jpg.bmp_out);	
			return texp;
		}
		return NULL;
	}   
