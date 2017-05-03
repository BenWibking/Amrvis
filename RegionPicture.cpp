// ---------------------------------------------------------------
// RegionPicture.cpp
// ---------------------------------------------------------------
#include <RegionPicture.H>
#include <Palette.H>
#include <GraphicsAttributes.H>
#include <ProfDataServices.H>

using std::cout;
using std::cerr;
using std::endl;

using namespace amrex;

#include <ctime>

// ---------------------------------------------------------------------
RegionPicture::RegionPicture(GraphicsAttributes *gaptr,
                             ProfDataServices *pdsp)
           : gaPtr(gaptr),
	     profDataServicesPtr(pdsp),
	     currentScale(1)
{
  BL_ASSERT(gaptr != nullptr);
  BL_ASSERT(pdsp  != nullptr);
  int nRegions(profDataServicesPtr->GetRegionsProfStats().GetMaxRNumber() + 1);
  ++nRegions;
  dataSizeH = 600;
  dataSizeV = nRegions * 16;
  dataSize  = dataSizeH * dataSizeV;  // for a picture (slice).
  atiDataSizeH = dataSizeH;
  atiDataSizeV = 16;
  atiDataSize  = atiDataSizeH * atiDataSizeV;

  RegionPictureInit();
}


// ---------------------------------------------------------------------
void RegionPicture::RegionPictureInit() {

  Box sliceBox(IntVect(0, 0), IntVect(dataSizeH - 1, dataSizeV - 1));
  cout << "sliceBox = " << sliceBox << endl;
  sliceFab = new FArrayBox(sliceBox, 1);
  display = gaPtr->PDisplay();
  xgc = gaPtr->PGC();

  imageSizeH = currentScale * dataSizeH;
  imageSizeV = currentScale * dataSizeV;
  if(imageSizeH > 65528 || imageSizeV > 65528) {  // not quite sizeof(short)
    // XImages cannot be larger than this
    cerr << "*** imageSizeH = " << imageSizeH << endl;
    cerr << "*** imageSizeV = " << imageSizeV << endl;
    amrex::Abort("Error in RegionPicture:  Image size too large.  Exiting.");
  }
  int widthpad = gaPtr->PBitmapPaddedWidth(imageSizeH);
  imageSize = imageSizeV * widthpad * gaPtr->PBytesPerPixel();

  imageData = new unsigned char[dataSize];
  scaledImageData = new unsigned char[imageSize];

  atiImageSizeH = currentScale * dataSizeH;
  atiImageSizeV = currentScale * 16;
  int atiWidthpad = gaPtr->PBitmapPaddedWidth(imageSizeH);
  atiImageSize = imageSizeV * atiWidthpad * gaPtr->PBytesPerPixel();
  atiImageData = new unsigned char[atiDataSize];
  scaledATIImageData = new unsigned char[atiImageSize];

  pixMapCreated = false;
}


// ---------------------------------------------------------------------
RegionPicture::~RegionPicture() {
  delete [] imageData;
  delete [] scaledImageData;
  delete sliceFab;

  if(pixMapCreated) {
    XFreePixmap(display, pixMap);
  }
}


// ---------------------------------------------------------------------
void RegionPicture::APDraw(int fromLevel, int toLevel) {
  if( ! pixMapCreated) {
    pixMap = XCreatePixmap(display, pictureWindow,
			   imageSizeH, imageSizeV, gaPtr->PDepth());
    pixMapCreated = true;
  }  
 
  XPutImage(display, pixMap, xgc, xImage, 0, 0, 0, 0,
	    imageSizeH, imageSizeV);
  XPutImage(display, pixMap, xgc, atiXImage, 0, 0, 0, imageSizeV-1-16,
	    imageSizeH, imageSizeV);
           
  DoExposePicture();
}


// ---------------------------------------------------------------------
void RegionPicture::DoExposePicture() {
  XCopyArea(display, pixMap, pictureWindow, xgc, 0, 0,
            imageSizeH, imageSizeV, 0, 0); 
}


// ---------------------------------------------------------------------
void RegionPicture::CreatePicture(Window drawPictureHere, Palette *palptr) {
  palPtr = palptr;
  pictureWindow = drawPictureHere;
  APMakeImages(palptr);
}


// ---------------------------------------------------------------------
void RegionPicture::APMakeImages(Palette *palptr) {
  BL_ASSERT(palptr != NULL);
  palPtr = palptr;

  int nRegions(profDataServicesPtr->GetRegionsProfStats().GetMaxRNumber() + 1);
  cout << "nRegions = " << nRegions << endl;

  Box regionBox(IntVect(0, 0), IntVect(dataSizeH - 1, dataSizeV - 1));
  cout << "regionBox = " << regionBox << endl;
  FArrayBox tempSliceFab(regionBox, 1);

  profDataServicesPtr->GetRegionsProfStats().MakeRegionPlt(tempSliceFab, 0,
                                      dataSizeH, dataSizeV / (nRegions + 1));

  tempSliceFab.shift(1, 16);
  sliceFab->copy(tempSliceFab);
  Real minUsing(sliceFab->min(0)), maxUsing(sliceFab->max(0));

  CreateImage(*(sliceFab), imageData, dataSizeH, dataSizeV, minUsing, maxUsing, palPtr);
  CreateScaledImage(&(xImage), currentScale,
                imageData, scaledImageData, dataSizeH, dataSizeV,
                imageSizeH, imageSizeV);
  for(int j(0); j < atiDataSizeV; ++j) {
    //int value(j == 0 || j == atiDataSizeV - 1 ? 0 : 1);
    int value(j < 2 || j > atiDataSizeV - 3 ? 0 : 1);
    for(int i(0); i < atiDataSizeH; ++i) {
      if(i > atiDataSizeH / 4) {
        value = 0;
      }
      atiImageData[j * atiDataSizeH + i] = value;
    }
  }
  CreateScaledImage(&(atiXImage), currentScale,
                atiImageData, scaledATIImageData, atiDataSizeH, atiDataSizeV,
                atiImageSizeH, atiImageSizeV);

  //if( ! pltAppPtr->PaletteDrawn()) {
    //pltAppPtr->PaletteDrawn(true);
    palptr->DrawPalette(minUsing, maxUsing, "%8.2f");
  //}

  APDraw(0, 0);
}


// -------------------------------------------------------------------
// convert Real to char in imagedata from fab
void RegionPicture::CreateImage(const FArrayBox &fab, unsigned char *imagedata,
			        int datasizeh, int datasizev,
			        Real globalMin, Real globalMax, Palette *palptr)
{
  int jdsh, jtmp1;
  int dIndex, iIndex;
  Real oneOverGDiff;
  if((globalMax - globalMin) < FLT_MIN) {
    oneOverGDiff = 0.0;
  } else {
    oneOverGDiff = 1.0 / (globalMax - globalMin);
  }
  const Real *dataPoint = fab.dataPtr();

  // flips the image in Vert dir: j => datasizev-j-1
    Real dPoint;
    int paletteStart(palptr->PaletteStart());
    int paletteEnd(palptr->PaletteEnd());
    int colorSlots(palptr->ColorSlots());
    int csm1(colorSlots - 1);
      for(int j(0); j < datasizev; ++j) {
        jdsh = j * datasizeh;
        jtmp1 = (datasizev-j-1) * datasizeh;
        for(int i(0); i < datasizeh; ++i) {
          dIndex = i + jtmp1;
          dPoint = dataPoint[dIndex];
	  if(dIndex >= fab.nPts()) {
	    cout << "**** dIndex fab.nPts() = " << dIndex << "  " << fab.nPts() << endl;
	  }
          iIndex = i + jdsh;
          if(dPoint > globalMax) {  // clip
            imagedata[iIndex] = paletteEnd;
          } else if(dPoint < globalMin) {  // clip
            imagedata[iIndex] = paletteStart;
          } else {
            imagedata[iIndex] = (unsigned char)
              ((((dPoint - globalMin) * oneOverGDiff) * csm1) );
              //  ^^^^^^^^^^^^^^^^^^ Real data
            imagedata[iIndex] += paletteStart;
          } 
        }
      }
}


// ---------------------------------------------------------------------
void RegionPicture::CreateScaledImage(XImage **ximage, int scale,
				      unsigned char *imagedata,
				      unsigned char *scaledimagedata,
				      int datasizeh, int datasizev,
				      int imagesizeh, int imagesizev)
{ 
  int widthpad(gaPtr->PBitmapPaddedWidth(imagesizeh));
  *ximage = XCreateImage(display, gaPtr->PVisual(), gaPtr->PDepth(),
                         ZPixmap, 0, (char *) scaledimagedata,
		         widthpad, imagesizev,
		         XBitmapPad(display),
			 widthpad * gaPtr->PBytesPerPixel());

  for(int j(0); j < imagesizev; ++j) {
    int jtmp(datasizeh * (j / scale));
    for(int i(0); i < widthpad; ++i) {
      int itmp(i / scale);
      unsigned char imm1(imagedata[ itmp + jtmp ]);
      XPutPixel(*ximage, i, j, palPtr->makePixel(imm1));
    }
  }

}


// ---------------------------------------------------------------------
void RegionPicture::APChangeScale(int newScale, int previousScale) { 
  imageSizeH = newScale * dataSizeH;
  imageSizeV = newScale * dataSizeV;
  int widthpad = gaPtr->PBitmapPaddedWidth(imageSizeH);
  imageSize  = widthpad * imageSizeV * gaPtr->PBytesPerPixel();
  XClearWindow(display, pictureWindow);

  if(pixMapCreated) {
    XFreePixmap(display, pixMap);
  }  
  pixMap = XCreatePixmap(display, pictureWindow,
			 imageSizeH, imageSizeV, gaPtr->PDepth());
  pixMapCreated = true;

  delete [] scaledImageData;
  scaledImageData = new unsigned char[imageSize];
  CreateScaledImage(&xImage, newScale,
                    imageData, scaledImageData, dataSizeH, dataSizeV,
                    imageSizeH, imageSizeV);

  atiImageSizeH = newScale * dataSizeH;
  atiImageSizeV = newScale * 16;
  atiImageSize  = widthpad * 16 * gaPtr->PBytesPerPixel();
  delete [] scaledATIImageData;
  scaledATIImageData = new unsigned char[atiImageSize];
  //for(int i(0); i < atiDataSize; ++i) {
    //atiImageData[i] = i % 11;
  //}
  CreateScaledImage(&(atiXImage), currentScale,
                atiImageData, scaledATIImageData, atiDataSizeH, atiDataSizeV,
                atiImageSizeH, atiImageSizeV);

  APDraw(0, 0);
}


// ---------------------------------------------------------------------
XImage *RegionPicture::GetPictureXImage() {
  XImage *ximage;

  ximage = XGetImage(display, pixMap, 0, 0,
		imageSizeH, imageSizeV, AllPlanes, ZPixmap);

  if(pixMapCreated) {
    XFreePixmap(display, pixMap);
  }  
  pixMap = XCreatePixmap(display, pictureWindow,
			 imageSizeH, imageSizeV, gaPtr->PDepth());
  pixMapCreated = true;
  APDraw(0, 0);
  return ximage;
}
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------