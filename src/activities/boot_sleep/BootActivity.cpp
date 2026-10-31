#include "BootActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>

#include "fontIds.h"
#include "images/CrossLeafLogo120Portrait.h"

void BootActivity::onEnter() {
  Activity::onEnter();

  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();

  constexpr int frameInset = 10;
  constexpr int logoSize = 120;
  const int logoTop = pageHeight / 2 - 145;

  renderer.clearScreen();
  renderer.drawRect(frameInset, frameInset, pageWidth - frameInset * 2, pageHeight - frameInset * 2, 2, true);
  renderer.drawImage(CrossLeafLogo120Portrait, (pageWidth - logoSize) / 2, logoTop, logoSize, logoSize);
  renderer.drawCenteredText(UI_10_FONT_ID, logoTop + logoSize + 24, "CROSSLEAF", true, EpdFontFamily::BOLD);
  renderer.drawCenteredText(SMALL_FONT_ID, logoTop + logoSize + 56, tr(STR_BOOTING));
  renderer.drawCenteredText(SMALL_FONT_ID, pageHeight - 30, "v" CROSSPOINT_VERSION);
  renderer.displayBuffer();
}
