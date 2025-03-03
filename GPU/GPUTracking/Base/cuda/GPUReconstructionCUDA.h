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

/// \file GPUReconstructionCUDA.h
/// \author David Rohr

#ifndef GPURECONSTRUCTIONCUDA_H
#define GPURECONSTRUCTIONCUDA_H

#include "GPUReconstructionDeviceBase.h"

#ifdef _WIN32
extern "C" __declspec(dllexport) GPUCA_NAMESPACE::gpu::GPUReconstruction* GPUReconstruction_Create_CUDA(const GPUCA_NAMESPACE::gpu::GPUSettingsDeviceBackend& cfg);
#else
extern "C" GPUCA_NAMESPACE::gpu::GPUReconstruction* GPUReconstruction_Create_CUDA(const GPUCA_NAMESPACE::gpu::GPUSettingsDeviceBackend& cfg);
#endif

namespace GPUCA_NAMESPACE
{
namespace gpu
{
struct GPUReconstructionCUDAInternals;

class GPUReconstructionCUDABackend : public GPUReconstructionDeviceBase
{
 public:
  ~GPUReconstructionCUDABackend() override;

 protected:
  GPUReconstructionCUDABackend(const GPUSettingsDeviceBackend& cfg);

  int InitDevice_Runtime() override;
  int ExitDevice_Runtime() override;
  void UpdateSettings() override;

  class GPUThreadContextCUDA : public GPUThreadContext
  {
   public:
    GPUThreadContextCUDA(GPUReconstructionCUDAInternals* context);
    ~GPUThreadContextCUDA() override;

   private:
    GPUReconstructionCUDAInternals* mContext = nullptr;
  };

  std::unique_ptr<GPUThreadContext> GetThreadContext() override;
  bool CanQueryMaxMemory() override { return true; }
  void SynchronizeGPU() override;
  int GPUDebug(const char* state = "UNKNOWN", int stream = -1, bool force = false) override;
  void SynchronizeStream(int stream) override;
  void SynchronizeEvents(deviceEvent* evList, int nEvents = 1) override;
  void StreamWaitForEvents(int stream, deviceEvent* evList, int nEvents = 1) override;
  bool IsEventDone(deviceEvent* evList, int nEvents = 1) override;

  int PrepareTextures() override;
  int registerMemoryForGPU(const void* ptr, size_t size) override;
  int unregisterMemoryForGPU(const void* ptr) override;
  void startGPUProfiling() override;
  void endGPUProfiling() override;

  size_t WriteToConstantMemory(size_t offset, const void* src, size_t size, int stream = -1, deviceEvent* ev = nullptr) override;
  size_t TransferMemoryInternal(GPUMemoryResource* res, int stream, deviceEvent* ev, deviceEvent* evList, int nEvents, bool toGPU, const void* src, void* dst) override;
  size_t GPUMemCpy(void* dst, const void* src, size_t size, int stream, int toGPU, deviceEvent* ev = nullptr, deviceEvent* evList = nullptr, int nEvents = 1) override;
  void ReleaseEvent(deviceEvent* ev) override;
  void RecordMarker(deviceEvent* ev, int stream) override;

  void GetITSTraits(std::unique_ptr<o2::its::TrackerTraits>* trackerTraits, std::unique_ptr<o2::its::VertexerTraits>* vertexerTraits) override;

  void PrintKernelOccupancies() override;

  template <class T, int I = 0, typename... Args>
  int runKernelBackend(krnlSetup& _xyz, const Args&... args);
  template <class T, int I = 0, typename... Args>
  void runKernelBackendInternal(krnlSetup& _xyz, const Args&... args);
  template <class T, int I = 0>
  const krnlProperties getKernelPropertiesBackend();
  template <class T, int I>
  class backendInternal;

 private:
  GPUReconstructionCUDAInternals* mInternals;
};

using GPUReconstructionCUDA = GPUReconstructionKernels<GPUReconstructionCUDABackend>;
} // namespace gpu
} // namespace GPUCA_NAMESPACE

#endif
