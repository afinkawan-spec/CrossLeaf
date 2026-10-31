#include "EpubReaderFootnoteSelectActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include <algorithm>

#include "CrossPointSettings.h"
#include "MappedInputManager.h"
#include "components/UITheme.h"
#include "fontIds.h"

namespace {
constexpr unsigned long WORD_REPEAT_START_MS = 500;
constexpr unsigned long WORD_REPEAT_INTERVAL_MS = 500;
}  // namespace

void EpubReaderFootnoteSelectActivity::onEnter() {
  Activity::onEnter();
  snapshot = makeUniqueNoThrow<uint8_t[]>(SNAPSHOT_CAPACITY);
  buildFootnoteLinks();
  requestUpdate();
}

void EpubReaderFootnoteSelectActivity::buildFootnoteLinks() {
  footnoteLinks.clear();
  footnoteLinks.reserve(gameBookMode ? page->links.size() : page->footnotes.size());

  if (gameBookMode) {
    for (const auto& link : page->links) {
      const auto footnote = std::find_if(page->footnotes.begin(), page->footnotes.end(), [&link](const auto& entry) {
        return strcmp(entry.href, link.href) == 0;
      });
      footnoteLinks.push_back({&link, footnote != page->footnotes.end() ? &*footnote : nullptr});
    }
    if (canRestorePrevious) selected = 0;
    return;
  }

  // Match each footnote entry to its PageLink on this page. Footnote links
  // and footnote entries share the same href, so we can cross-reference them.
  for (const auto& footnote : page->footnotes) {
    const auto link = std::find_if(page->links.begin(), page->links.end(), [&footnote](const PageLink& candidate) {
      return strcmp(candidate.href, footnote.href) == 0;
    });
    if (link != page->links.end()) {
      footnoteLinks.push_back({&*link, &footnote});
    }
  }
}

void EpubReaderFootnoteSelectActivity::performJump() {
  ActivityResult result;
  if (gameBookMode && canRestorePrevious && selected == 0) {
    FootnoteResult selection;
    selection.restorePrevious = true;
    result.data = std::move(selection);
  } else {
    const int linkIndex = selected - ((gameBookMode && canRestorePrevious) ? 1 : 0);
    if (linkIndex < 0 || linkIndex >= static_cast<int>(footnoteLinks.size())) return;
    result.data = FootnoteResult{footnoteLinks[linkIndex].link->href, false};
  }
  setResult(std::move(result));
  finish();
}

void EpubReaderFootnoteSelectActivity::drawHints() const {
  const auto labels = mappedInput.mapDirectionalLabels(tr(STR_BACK), tr(STR_OPEN), tr(STR_DIR_LEFT), tr(STR_DIR_RIGHT),
                                                       tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
}

void EpubReaderFootnoteSelectActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    ActivityResult result;
    result.isCancelled = true;
    setResult(std::move(result));
    finish();
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm) &&
      (!footnoteLinks.empty() || (gameBookMode && canRestorePrevious))) {
    performJump();
    return;
  }

  if (footnoteLinks.empty() && !(gameBookMode && canRestorePrevious)) return;

  const int choiceCount = static_cast<int>(footnoteLinks.size()) + ((gameBookMode && canRestorePrevious) ? 1 : 0);
  const bool hasNext = selected + 1 < choiceCount;
  const unsigned long now = millis();
  const bool repeat =
      mappedInput.getHeldTime() >= WORD_REPEAT_START_MS && now - lastHorizontalMoveTime >= WORD_REPEAT_INTERVAL_MS;
  const bool moveLeft = mappedInput.wasPressed(MappedInputManager::Button::ScreenLeft) ||
                        (repeat && mappedInput.isPressed(MappedInputManager::Button::ScreenLeft));
  const bool moveRight = mappedInput.wasPressed(MappedInputManager::Button::ScreenRight) ||
                         (repeat && mappedInput.isPressed(MappedInputManager::Button::ScreenRight));
  if ((moveLeft || mappedInput.wasPressed(MappedInputManager::Button::ScreenUp)) && selected > 0) {
    selected--;
    if (moveLeft) lastHorizontalMoveTime = now;
    requestUpdate();
  } else if ((moveRight || mappedInput.wasPressed(MappedInputManager::Button::ScreenDown)) && hasNext) {
    selected++;
    if (moveRight) lastHorizontalMoveTime = now;
    requestUpdate();
  }
}

bool EpubReaderFootnoteSelectActivity::drawHighlightWithSnapshot() {
  if (gameBookMode && canRestorePrevious && selected == 0) {
    const int width = renderer.getScreenWidth();
    renderer.fillRect(4, 4, width - 8, 24, true);
    renderer.drawCenteredText(UI_10_FONT_ID, 10, tr(STR_GAME_BOOK_RETURN), false, EpdFontFamily::BOLD);
    snapshotIdx = -1;
    return false;
  }
  if (footnoteLinks.empty()) return false;
  const int linkIndex = selected - ((gameBookMode && canRestorePrevious) ? 1 : 0);
  if (linkIndex < 0 || linkIndex >= static_cast<int>(footnoteLinks.size())) return false;
  const PageLink* link = footnoteLinks[linkIndex].link;

  const int x = link->x + marginLeft;
  const int y = link->y + marginTop;
  int hx = x - 2;
  int hy = y - 2;
  int hw = link->width + 4;
  int hh = link->height + 4;

  if (hx < 0) {
    hw += hx;
    hx = 0;
  }
  if (hy < 0) {
    hh += hy;
    hy = 0;
  }

  bool saved = false;
  if (snapshot && hw > 0 && hh > 0) {
    saved = renderer.readFramebufferRegion(hx, hy, hw, hh, snapshot.get(), SNAPSHOT_CAPACITY) > 0;
  }
  snapshotX = static_cast<int16_t>(hx);
  snapshotY = static_cast<int16_t>(hy);
  snapshotW = static_cast<int16_t>(hw);
  snapshotH = static_cast<int16_t>(hh);
  snapshotIdx = saved ? selected : -1;

  renderer.fillRect(hx, hy, hw, hh, true);
  // Draw a small marker (the footnote number) at the highlight position
  const std::string choiceNumber = std::to_string(linkIndex + 1);
  const char* number = footnoteLinks[linkIndex].footnote ? footnoteLinks[linkIndex].footnote->number
                                                         : choiceNumber.c_str();
  if (number[0]) {
    renderer.drawText(SETTINGS.getReaderFontId(), x, y - 1, number, false);
  }
  return saved;
}

void EpubReaderFootnoteSelectActivity::render(RenderLock&&) {
  // Differential fast path: only the highlight moved and the framebuffer still
  // holds a clean page.
  if (snapshotIdx >= 0 && !footnoteLinks.empty() && selected != snapshotIdx) {
    renderer.writeFramebufferRegion(snapshotX, snapshotY, snapshotW, snapshotH, snapshot.get());
    if (drawHighlightWithSnapshot()) {
      drawHints();
      renderer.displayBuffer(HalDisplay::FAST_REFRESH);
      return;
    }
    // Snapshot failed — fall through to a full repaint.
  }

  renderer.clearScreen();

  // Render the page underneath
  if (page) {
    page->render(renderer, SETTINGS.getReaderFontId(), marginLeft, marginTop);
  }

  if (!footnoteLinks.empty()) {
    drawHighlightWithSnapshot();
  }

  drawHints();

  renderer.displayBuffer(HalDisplay::FAST_REFRESH);
}
