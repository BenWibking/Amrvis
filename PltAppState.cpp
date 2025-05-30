// ---------------------------------------------------------------
// PltAppState.cpp
// ---------------------------------------------------------------
#include <PltAppState.H>

#include <cctype>
#include <iostream>
#include <limits>
using std::cout;
using std::cerr;
using std::endl;

using namespace amrex;


// -------------------------------------------------------------------
CMinMax::CMinMax()
	: rMin( std::numeric_limits<Real>::max()),
	  rMax(-std::numeric_limits<Real>::max()),
          bMinMaxSet(false)
{
}


// -------------------------------------------------------------------
CMinMax::~CMinMax() {
}


// -------------------------------------------------------------------
void CMinMax::SetMinMax(const Real rmin, const Real rmax) {
  BL_ASSERT(rmax >= rmin);
  rMin = rmin;
  rMax = rmax;
  bMinMaxSet = true;
}


// -------------------------------------------------------------------
void CMinMax::GetMinMax(Real &rmin, Real &rmax) {
  BL_ASSERT(bMinMaxSet);
  rmin = rMin;
  rmax = rMax;
}


// -------------------------------------------------------------------
// -------------------------------------------------------------------
PltAppState::PltAppState(int numFrames, int numDerived)
            : currentScale(NOTSETYET),
	      maxScale(NOTSETYET),
	      currentFrame(0),
	      currentDerivedNumber(NOTSETYET),
	      currentContourType(Amrvis::INVALIDCONTOURTYPE),
	      nContours(NOTSETYET),
	      currentMinMaxType(Amrvis::INVALIDMINMAX),
	      minDrawnLevel(NOTSETYET),
	      maxDrawnLevel(NOTSETYET),
	      minAllowableLevel(NOTSETYET),
	      maxAllowableLevel(NOTSETYET),
	      finestLevel(NOTSETYET)
{
  minMax.resize(numFrames);
  for(int nf(0); nf < numFrames; ++nf) {
    minMax[nf].resize(numDerived);
    for(int nd(0); nd < numDerived; ++nd) {
      minMax[nf][nd].resize(Amrvis::NUMBEROFMINMAX);
    }
  }
  cgSmoothing = false;
  logScale = false;
}


// -------------------------------------------------------------------
PltAppState::~PltAppState() {
}


// -------------------------------------------------------------------
PltAppState &PltAppState::operator=(const PltAppState &rhs) {
  if(this == &rhs) {
    return *this;
  }
  currentScale = rhs.currentScale;
  maxScale = rhs.maxScale;
  currentFrame = rhs.currentFrame;
  currentDerived = rhs.currentDerived;
  currentDerivedNumber = rhs.currentDerivedNumber;
  showBoxes = rhs.showBoxes;
  cgSmoothing = rhs.cgSmoothing;
  logScale = rhs.logScale;
  currentContourType = rhs.currentContourType;
  nContours = rhs.nContours;
  currentMinMaxType = rhs.currentMinMaxType;

  // mins and maxes
  //Array<Vector<Array<CMinMax> > > minMax;   // minMax [frame] [derived] [RangeType]
  minMax = rhs.minMax;   // minMax [frame] [derived] [RangeType]

  //Array<Box> subDomains;
  subDomains = rhs.subDomains;

  minDrawnLevel = rhs.minDrawnLevel;
  maxDrawnLevel = rhs.maxDrawnLevel;
  minAllowableLevel = rhs.minAllowableLevel;
  maxAllowableLevel = rhs.maxAllowableLevel;
  finestLevel = rhs.finestLevel;
  contourNumString = rhs.contourNumString;
  formatString = rhs.formatString;
  fileName = rhs.fileName;
  palFilename = rhs.palFilename;

  return *this;
}


// -------------------------------------------------------------------
void PltAppState::SetMinMax(const Amrvis::MinMaxRangeType mmrangetype,
			    const int framenumber, const int derivednumber,
		            const Real rmin, const Real rmax)
{
  if (framenumber < 0 || framenumber >= minMax.size()) {
    return;
  }
  if (derivednumber < 0 || derivednumber >= minMax[framenumber].size()) {
    return;
  }
  if (mmrangetype < 0 || mmrangetype >= minMax[framenumber][derivednumber].size()) {
    return;
  }
  minMax[framenumber][derivednumber][mmrangetype].SetMinMax(rmin, rmax);
}


// -------------------------------------------------------------------
void PltAppState::GetMinMax(const Amrvis::MinMaxRangeType mmrangetype,
			    const int framenumber, const int derivednumber,
		            Real &rmin, Real &rmax)
{
  if (framenumber < 0 || framenumber >= minMax.size()) {
    rmin = rmax = 0.0;
    return;
  }
  if (derivednumber < 0 || derivednumber >= minMax[framenumber].size()) {
    rmin = rmax = 0.0;
    return;
  }
  if (mmrangetype < 0 || mmrangetype >= minMax[framenumber][derivednumber].size()) {
    rmin = rmax = 0.0;
    return;
  }
  minMax[framenumber][derivednumber][mmrangetype].GetMinMax(rmin, rmax);
}


// -------------------------------------------------------------------
void PltAppState::GetMinMax(Real &rmin, Real &rmax) {
  minMax[currentFrame][currentDerivedNumber][currentMinMaxType].
						      GetMinMax(rmin, rmax);
}


// -------------------------------------------------------------------
bool PltAppState::IsSet(const Amrvis::MinMaxRangeType mmrangetype,
			const int framenumber, const int derivednumber)
{
  if (framenumber < 0 || framenumber >= minMax.size()) {
    return false;
  }
  if (derivednumber < 0 || derivednumber >= minMax[framenumber].size()) {
    return false;
  }
  if (mmrangetype < 0 || mmrangetype >= minMax[framenumber][derivednumber].size()) {
    return false;
  }
  return (minMax[framenumber][derivednumber][mmrangetype].IsSet());
}


// -------------------------------------------------------------------
void PltAppState::PrintSetMap() {
  cout << "PltAppState::PrintSetMap(): minMax[frame] [derived] [rangetype]" << endl;
  for(int iframe(0); iframe < minMax.size(); ++iframe) {
    for(int ider(0); ider < minMax[iframe].size(); ++ider) {
      for(int immrt(0); immrt < minMax[iframe][ider].size(); ++immrt) {
	cout << "minMax[" << iframe << "][" << ider << "][" << immrt << "] = ";
	if(minMax[iframe][ider][immrt].IsSet()) {
	  cout << " set      =  ";
	} else {
	  cout << " not set  =  ";
	}
	cout << minMax[iframe][ider][immrt].Min() << "   "
	     << minMax[iframe][ider][immrt].Max() << endl;
      }
      cout << endl;
    }
    cout << endl;
  }
}


// -------------------------------------------------------------------
void PltAppState::ResizeMinMaxForDerived(int newNumDerived) {
  for(int iframe(0); iframe < minMax.size(); ++iframe) {
    int oldNumDerived = minMax[iframe].size();
    if(newNumDerived > oldNumDerived) {
      minMax[iframe].resize(newNumDerived);
      for(int ider(oldNumDerived); ider < newNumDerived; ++ider) {
        minMax[iframe][ider].resize(Amrvis::NUMBEROFMINMAX);
      }
    }
  }
}


// -------------------------------------------------------------------
void PltAppState::SetCurrentDerived(const string &newDerived, int cdnumber) {
  currentDerived = newDerived;
  currentDerivedNumber = cdnumber;
}
// -------------------------------------------------------------------
// -------------------------------------------------------------------
