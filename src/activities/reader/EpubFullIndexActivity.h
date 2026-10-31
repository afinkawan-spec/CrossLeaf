#pragma once

#include <Epub.h>
#include <Epub/ReaderRenderSpec.h>
#include <Epub/Section.h>

#include <memory>
#include <utility>

#include "activities/Activity.h"

class EpubFullIndexActivity final : public Activity {
 public:
  EpubFullIndexActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, std::shared_ptr<Epub> epub,
                        ReaderRenderSpec renderSpec);

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;
  bool handleHomeGesture() override;
  bool preventAutoSleep() override { return state == State::INDEXING; }
  bool skipLoopDelay() override { return state == State::INDEXING; }

 private:
  enum class State : uint8_t { INDEXING, COMPLETE, FAILED };

  static constexpr int BUILD_PAGES_PER_TICK = 4;
  static constexpr int PROGRESS_REFRESH_STEP = 5;

  std::shared_ptr<Epub> epub;
  ReaderRenderSpec renderSpec;
  std::unique_ptr<Section> section;
  State state = State::INDEXING;
  int spineIndex = 0;
  int spineCount = 0;
  int progressPercent = 0;
  int lastDisplayedPercent = -PROGRESS_REFRESH_STEP;
  bool initialFrameShown = false;

  int calculateProgressPercent() const;
  void processBuildChunk();
  void close(bool cancelled);
};
