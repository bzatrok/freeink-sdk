#pragma once
// TEMPORARY latency instrumentation for the input -> render -> panel pipeline.
//
// Compiled in only with -DFREEINK_LATENCY_TRACE=1. The SDK records marks through a
// weak C hook, freeink_latency_mark(id); a host that wants the data defines that
// symbol (CrossInk: src/util/LatencyTrace.cpp). Without a definition, or without
// the flag, every macro below is a no-op and nothing links in.
//
// Mark ids are the shared contract between the SDK and the host, so they live here.
#include <stdint.h>

namespace freeink {
namespace lat {
enum Mark : uint8_t {
  InputEdge = 0,     // host: raw button/touch edge seen by the input loop
  PageTurn = 1,      // host: page-turn decision made
  RenderStart = 2,   // host: render task starts drawing the frame
  DisplayCall = 3,   // SDK: facade display entry (framebuffer complete)
  DriverStart = 4,   // SDK: about to hand the frame to the panel driver (SPI starts)
  WaveformWait = 5,  // SDK: refresh command issued, waiting for BUSY (SPI done)
  BusyReleased = 6,  // SDK: BUSY released, waveform finished
  DriverReturn = 7,  // SDK: facade display call returns (post-waveform uploads done)
  RenderDone = 8,    // host: render task finished the frame
  BusyWait = 9,      // SDK: any EpdBus::waitBusy() entered (command/PON/POF/UC DRF waits)
  BusyDone = 10,     // SDK: that waitBusy() returned
  Count = 11,
};
}  // namespace lat
}  // namespace freeink

#if defined(FREEINK_LATENCY_TRACE) && FREEINK_LATENCY_TRACE
extern "C" void freeink_latency_mark(uint8_t id) __attribute__((weak));
#define FREEINK_LAT_MARK(id)                                                  \
  do {                                                                        \
    if (freeink_latency_mark) freeink_latency_mark(static_cast<uint8_t>(id)); \
  } while (0)
namespace freeink {
namespace lat {
struct DisplayScope {
  DisplayScope() { FREEINK_LAT_MARK(DisplayCall); }
  ~DisplayScope() { FREEINK_LAT_MARK(DriverReturn); }
};
struct WaitScope {
  WaitScope() { FREEINK_LAT_MARK(WaveformWait); }
  ~WaitScope() { FREEINK_LAT_MARK(BusyReleased); }
};
struct BusyScope {
  BusyScope() { FREEINK_LAT_MARK(BusyWait); }
  ~BusyScope() { FREEINK_LAT_MARK(BusyDone); }
};
}  // namespace lat
}  // namespace freeink
#define FREEINK_LAT_DISPLAY_SCOPE() ::freeink::lat::DisplayScope freeinkLatDisplayScope_
#define FREEINK_LAT_WAIT_SCOPE() ::freeink::lat::WaitScope freeinkLatWaitScope_
#define FREEINK_LAT_BUSY_SCOPE() ::freeink::lat::BusyScope freeinkLatBusyScope_
#else
#define FREEINK_LAT_MARK(id) \
  do {                       \
  } while (0)
#define FREEINK_LAT_DISPLAY_SCOPE() \
  do {                              \
  } while (0)
#define FREEINK_LAT_WAIT_SCOPE() \
  do {                           \
  } while (0)
#define FREEINK_LAT_BUSY_SCOPE() \
  do {                           \
  } while (0)
#endif
