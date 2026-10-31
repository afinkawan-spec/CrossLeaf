#include "EpubFullIndexActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>
#include <Logging.h>

#include <algorithm>
#include <cstdint>
#include <string>

#include "MappedInputManager.h"
#include "components/UITheme.h"
#include "fontIds.h"

EpubFullIndexActivity::EpubFullIndexActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                                             std::shared_ptr<Epub> epub, ReaderRenderSpec renderSpec)
    : Activity("EpubFullIndex", renderer, mappedInput), epub(std::move(epub)), renderSpec(renderSpec) {}

void EpubFullIndexActivity::onEnter() {
  Activity::onEnter();
  spineCount = epub ? epub->getSpineItemsCount() : 0;
  if (!epub || spineCount <= 0 || renderSpec.viewportWidth == 0 || renderSpec.viewportHeight == 0) {
    state = State::FAILED;
  }
  requestUpdate();
}

void EpubFullIndexActivity::onExit() {
  section.reset();
  Activity::onExit();
}

void EpubFullIndexActivity::close(const bool cancelled) {
  {
    RenderLock lock(*this);
    section.reset();
  }
  ActivityResult result;
  result.isCancelled = cancelled;
  setResult(std::move(result));
  finish();
}

bool EpubFullIndexActivity::handleHomeGesture() {
  close(state == State::INDEXING);
  return true;
}

int EpubFullIndexActivity::calculateProgressPercent() const {
  if (!epub || spineCount <= 0) return 0;
  if (state == State::COMPLETE || spineIndex >= spineCount) return 100;

  const uint64_t bookBytes = epub->getBookSize();
  if (bookBytes == 0) {
    return std::clamp(spineIndex * 100 / spineCount, 0, 99);
  }

  uint64_t completedBytes = spineIndex > 0 ? epub->getCumulativeSpineItemSize(spineIndex - 1) : 0;
  if (section && section->isBuilding()) {
    const uint64_t spineEnd = epub->getCumulativeSpineItemSize(spineIndex);
    const uint64_t spineBytes = spineEnd > completedBytes ? spineEnd - completedBytes : 0;
    completedBytes += spineBytes * section->pageCount / std::max<uint16_t>(section->estimatedTotalPages(), 1);
  }

  return std::clamp(static_cast<int>(completedBytes * 100 / bookBytes), 0, 99);
}

void EpubFullIndexActivity::processBuildChunk() {
  if (state != State::INDEXING || RenderLock::peek()) return;

  bool refresh = false;
  {
    RenderLock lock(*this);
    if (state != State::INDEXING) return;

    if (spineIndex >= spineCount) {
      state = State::COMPLETE;
      progressPercent = 100;
      refresh = true;
    } else {
      if (!section) {
        section = std::make_unique<Section>(epub, spineIndex, renderer);
        const bool cacheLoaded = section->loadSectionFile(renderSpec);
        if (cacheLoaded && !section->isPartial()) {
          LOG_DBG("EFI", "Spine %d/%d already indexed", spineIndex + 1, spineCount);
          section.reset();
          spineIndex++;
        } else if (!section->startBuild(renderSpec)) {
          LOG_ERR("EFI", "Failed to start full-book index at spine %d", spineIndex);
          section.reset();
          state = State::FAILED;
          refresh = true;
        }
      }

      if (state == State::INDEXING && section && section->isBuilding()) {
        if (!section->buildSomeMore(BUILD_PAGES_PER_TICK)) {
          LOG_ERR("EFI", "Full-book index failed at spine %d", spineIndex);
          section.reset();
          state = State::FAILED;
          refresh = true;
        } else if (section->isBuildComplete()) {
          LOG_DBG("EFI", "Indexed spine %d/%d (%u pages)", spineIndex + 1, spineCount, section->pageCount);
          section.reset();
          spineIndex++;
        }
      }

      if (state == State::INDEXING && spineIndex >= spineCount) {
        state = State::COMPLETE;
        progressPercent = 100;
        refresh = true;
      } else if (state == State::INDEXING) {
        progressPercent = calculateProgressPercent();
        refresh = progressPercent >= lastDisplayedPercent + PROGRESS_REFRESH_STEP;
      }
    }
  }

  if (refresh) {
    lastDisplayedPercent = progressPercent;
    requestUpdateAndWait();
  }
}

void EpubFullIndexActivity::loop() {
  if (!initialFrameShown) {
    requestUpdateAndWait();
    initialFrameShown = true;
    return;
  }

  int touchX = 0;
  int touchY = 0;
  const bool tappedFooter = mappedInput.wasScreenTapped(touchX, touchY) &&
                            touchY >= renderer.getScreenHeight() - 90;
  if (mappedInput.wasReleased(MappedInputManager::Button::Back) || tappedFooter) {
    close(state == State::INDEXING);
    return;
  }

  if (state != State::INDEXING) {
    if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
      close(false);
    }
    return;
  }

  processBuildChunk();
}

void EpubFullIndexActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int pageWidth = renderer.getScreenWidth();
  const int pageHeight = renderer.getScreenHeight();
  const Rect screen = UITheme::getInstance().getScreenSafeArea(renderer, true, false);

  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{screen.x, screen.y + metrics.topPadding, screen.width, metrics.headerHeight},
                 tr(STR_INDEX_ENTIRE_BOOK));
  if (epub) {
    GUI.drawSubHeader(
        renderer,
        Rect{screen.x, screen.y + metrics.topPadding + metrics.headerHeight, screen.width, metrics.tabBarHeight},
        epub->getTitle().c_str());
  }

  const int centerY = pageHeight / 2 - metrics.progressBarHeight;
  if (state == State::INDEXING) {
    const int displayedSpine = std::min(spineIndex + 1, spineCount);
    const std::string chapterText = std::string(tr(STR_CHAPTER_PREFIX)) + std::to_string(displayedSpine) + "/" +
                                    std::to_string(spineCount);
    renderer.drawCenteredText(UI_10_FONT_ID, centerY - 35, chapterText.c_str(), true, EpdFontFamily::BOLD);
    GUI.drawProgressBar(
        renderer,
        Rect{metrics.contentSidePadding, centerY, pageWidth - metrics.contentSidePadding * 2, metrics.progressBarHeight},
        progressPercent, 100);
  } else if (state == State::COMPLETE) {
    renderer.drawCenteredText(UI_12_FONT_ID, centerY, tr(STR_DONE), true, EpdFontFamily::BOLD);
  } else {
    renderer.drawCenteredText(UI_10_FONT_ID, centerY, tr(STR_INDEX_FAILED), true, EpdFontFamily::BOLD);
  }

  const char* footerLabel = state == State::INDEXING ? tr(STR_CANCEL) : tr(STR_BACK);
  if (mappedInput.hasTouch()) {
    renderer.drawCenteredText(UI_10_FONT_ID, pageHeight - 45, footerLabel);
  } else {
    const auto labels = mappedInput.mapLabels(footerLabel, state == State::INDEXING ? "" : tr(STR_SELECT), "", "");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  }

  renderer.displayBuffer();
}
