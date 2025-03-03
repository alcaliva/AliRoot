//**************************************************************************\
//* This file is property of and copyright by the ALICE Project            *\
//* ALICE Experiment at CERN, All rights reserved.                         *\
//*                                                                        *\
//* Primary Authors: Matthias Richter <Matthias.Richter@ift.uib.no>        *\
//*                  for The ALICE HLT Project.                            *\
//*                                                                        *\
//* Permission to use, copy, modify and distribute this software and its   *\
//* documentation strictly for non-commercial purposes is hereby granted   *\
//* without fee, provided that the above copyright notice appears in all   *\
//* copies and that both the copyright notice and this permission notice   *\
//* appear in the supporting documentation. The authors make no claims     *\
//* about the suitability of this software for any purpose. It is          *\
//* provided "as is" without express or implied warranty.                  *\
//**************************************************************************

/// \file GPUTRDTrack.cxx
/// \author Ole Schmidt, Sergey Gorbunov

#include "GPUTRDTrack.h"
#include "GPUTRDTrackData.h"

using namespace GPUCA_NAMESPACE::gpu;

template <typename T>
GPUd() GPUTRDTrack_t<T>::GPUTRDTrack_t()
{
  //------------------------------------------------------------------
  // default constructor
  //------------------------------------------------------------------
  Initialize();
}

template <typename T>
GPUd() void GPUTRDTrack_t<T>::Initialize()
{
  // set all members to their default values (needed since in-class initialization not possible with AliRoot)
  mChi2 = 0.f;
  mMass = 0.f;
  mLabel = -1;
  mTPCTrackId = 0;
  mNTracklets = 0;
  mNMissingConsecLayers = 0;
  mLabelOffline = -1;
  mIsStopped = false;
  for (int i = 0; i < kNLayers; ++i) {
    mAttachedTracklets[i] = -1;
    mIsFindable[i] = 0;
  }
  for (int j = 0; j < 4; ++j) {
    mNTrackletsOffline[j] = 0;
  }
}

#ifdef GPUCA_ALIROOT_LIB
#include "AliHLTExternalTrackParam.h"
template <typename T>
GPUd() GPUTRDTrack_t<T>::GPUTRDTrack_t(const AliHLTExternalTrackParam& t) : T(t)
{
  Initialize();
}
#endif

#if defined(GPUCA_O2_LIB) && !defined(GPUCA_GPUCODE)
#include "ReconstructionDataFormats/TrackTPCITS.h"
#include "DataFormatsTPC/TrackTPC.h"
template <typename T>
GPUd() GPUTRDTrack_t<T>::GPUTRDTrack_t(const o2::dataformats::TrackTPCITS& t, float vDrift) : T(t, vDrift)
{
  Initialize();
}
template <typename T>
GPUd() GPUTRDTrack_t<T>::GPUTRDTrack_t(const o2::tpc::TrackTPC& t, float tbWidth, float vDrift, unsigned int iTrk) : T(t, tbWidth, vDrift, iTrk)
{
  Initialize();
}
#endif

template <typename T>
GPUd() GPUTRDTrack_t<T>::GPUTRDTrack_t(const GPUTRDTrack_t<T>& t)
  : T(t), mChi2(t.mChi2), mMass(t.mMass), mLabel(t.mLabel), mTPCTrackId(t.mTPCTrackId), mNTracklets(t.mNTracklets), mNMissingConsecLayers(t.mNMissingConsecLayers), mLabelOffline(t.mLabelOffline), mIsStopped(t.mIsStopped)
{
  //------------------------------------------------------------------
  // copy constructor
  //------------------------------------------------------------------
  for (int i = 0; i < kNLayers; ++i) {
    mAttachedTracklets[i] = t.mAttachedTracklets[i];
    mIsFindable[i] = t.mIsFindable[i];
  }
  for (int j = 0; j < 4; ++j) {
    mNTrackletsOffline[j] = t.mNTrackletsOffline[j];
  }
}

template <typename T>
GPUd() GPUTRDTrack_t<T>::GPUTRDTrack_t(const T& t) : T(t)
{
  //------------------------------------------------------------------
  // copy constructor from anything
  //------------------------------------------------------------------
  Initialize();
}

template <typename T>
GPUd() GPUTRDTrack_t<T>& GPUTRDTrack_t<T>::operator=(const GPUTRDTrack_t<T>& t)
{
  //------------------------------------------------------------------
  // assignment operator
  //------------------------------------------------------------------
  if (&t == this) {
    return *this;
  }
  *(T*)this = t;
  mChi2 = t.mChi2;
  mMass = t.mMass;
  mLabel = t.mLabel;
  mTPCTrackId = t.mTPCTrackId;
  mNTracklets = t.mNTracklets;
  mNMissingConsecLayers = t.mNMissingConsecLayers;
  mLabelOffline = t.mLabelOffline;
  mIsStopped = t.mIsStopped;
  for (int i = 0; i < kNLayers; ++i) {
    mAttachedTracklets[i] = t.mAttachedTracklets[i];
    mIsFindable[i] = t.mIsFindable[i];
  }
  for (int j = 0; j < 4; ++j) {
    mNTrackletsOffline[j] = t.mNTrackletsOffline[j];
  }
  return *this;
}

template <typename T>
GPUd() int GPUTRDTrack_t<T>::GetNlayers() const
{
  //------------------------------------------------------------------
  // returns number of layers in which the track is in active area of TRD
  //------------------------------------------------------------------
  int res = 0;
  for (int iLy = 0; iLy < kNLayers; iLy++) {
    if (mIsFindable[iLy]) {
      ++res;
    }
  }
  return res;
}

template <typename T>
GPUd() int GPUTRDTrack_t<T>::GetTracklet(int iLayer) const
{
  //------------------------------------------------------------------
  // returns index of attached tracklet in given layer
  //------------------------------------------------------------------
  if (iLayer < 0 || iLayer >= kNLayers) {
    return -1;
  }
  return mAttachedTracklets[iLayer];
}

template <typename T>
GPUd() int GPUTRDTrack_t<T>::GetNmissingConsecLayers(int iLayer) const
{
  //------------------------------------------------------------------
  // returns number of consecutive layers in which the track was
  // inside the deadzone up to (and including) the given layer
  //------------------------------------------------------------------
  int res = 0;
  while (!mIsFindable[iLayer]) {
    ++res;
    --iLayer;
    if (iLayer < 0) {
      break;
    }
  }
  return res;
}

template <typename T>
GPUd() void GPUTRDTrack_t<T>::ConvertTo(GPUTRDTrackDataRecord& t) const
{
  //------------------------------------------------------------------
  // convert to GPU structure
  //------------------------------------------------------------------
  t.mAlpha = T::getAlpha();
  t.fX = T::getX();
  t.fY = T::getY();
  t.fZ = T::getZ();
  t.fq1Pt = T::getQ2Pt();
  t.mSinPhi = T::getSnp();
  t.fTgl = T::getTgl();
  for (int i = 0; i < 15; i++) {
    t.fC[i] = T::getCov()[i];
  }
  t.fTPCTrackID = GetTPCtrackId();
  for (int i = 0; i < kNLayers; i++) {
    t.fAttachedTracklets[i] = GetTracklet(i);
  }
}

template <typename T>
GPUd() void GPUTRDTrack_t<T>::ConvertFrom(const GPUTRDTrackDataRecord& t)
{
  //------------------------------------------------------------------
  // convert from GPU structure
  //------------------------------------------------------------------
  T::set(t.fX, t.mAlpha, &(t.fY), t.fC);
  SetTPCtrackId(t.fTPCTrackID);
  mChi2 = 0.f;
  mMass = 0.13957f;
  mLabel = -1;
  mNTracklets = 0;
  mNMissingConsecLayers = 0;
  mLabelOffline = -1;
  mIsStopped = false;
  for (int iLayer = 0; iLayer < kNLayers; iLayer++) {
    mAttachedTracklets[iLayer] = t.fAttachedTracklets[iLayer];
    mIsFindable[iLayer] = 0;
    if (mAttachedTracklets[iLayer] >= 0) {
      mNTracklets++;
    }
  }
  for (int j = 0; j < 4; ++j) {
    mNTrackletsOffline[j] = 0;
  }
}

#ifndef GPUCA_GPUCODE
namespace GPUCA_NAMESPACE
{
namespace gpu
{
#ifdef GPUCA_ALIROOT_LIB // Instantiate AliRoot track version
template class GPUTRDTrack_t<trackInterface<AliExternalTrackParam>>;
#endif
#ifdef HAVE_O2HEADERS // Instantiate O2 track version
template class GPUTRDTrack_t<trackInterface<o2::gpu::GPUTRDO2BaseTrack>>;
#endif
template class GPUTRDTrack_t<trackInterface<GPUTPCGMTrackParam>>; // Always instatiate GM track version
} // namespace gpu
} // namespace GPUCA_NAMESPACE
#endif
