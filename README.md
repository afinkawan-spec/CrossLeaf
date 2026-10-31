# CrossLeaf

**CrossLeaf is a deliberately personal, vibe-coded firmware for one Xteink X4 reader.** It was built iteratively with AI coding assistance around one person's specific likes and dislikes: keep the reading experience and useful everyday tools, remove menu clutter, and make the remaining choices behave exactly as requested. It is not intended to be a universal configuration or a feature-for-feature replacement for CrossPoint.

CrossLeaf 1.0.0 is based on CrossPoint Reader 1.6.5. The source records the choices made for this particular device; some upstream implementation remains in the code even when its setting or menu entry is hidden. The release firmware has been tested on-device.

## What changed from CrossPoint

- **CrossLeaf identity:** Custom portrait logo on the boot and sleep screens, with CrossLeaf branding and version 1.0.0.
- **A simpler home screen:** The Lyra cover layout shows two larger recent-book covers, with the main actions below them.
- **A quieter menu system:** Settings are presented as one direct list. Library and OPDS entry points, KOReader Sync, status-bar customization, font management, and other unwanted controls are removed from the user-facing menus. Choices intended to stay fixed, such as portrait orientation, display image rendering, and list-style reader menus, no longer need a setting.
- **Reading-focused controls:** Text settings and font-family selection live in the in-reader menu. The Back button turns to the next page on a short press and returns Home on a long press while reading; its ordinary behavior outside a book is unchanged. Game Book Mode is available for linked-choice books.
- **Always-visible reading indicators:** Whole-book progress is shown along the bottom and battery charge along the top. Both bars show by default and have independent thickness settings; there is no battery icon or percentage display.
- **Useful tools kept:** Dictionary selection and lookup, EPUB illustrations, Book Metadata, manual Wi-Fi network joining for file transfer, reading-cache clearing, and SD-card firmware updates remain available.

The point is not to remove code just to make the firmware smaller. CrossLeaf favors a focused interface and fixed preferences over exposing every upstream option. Where an upstream feature is only hidden or fixed in the menus, this README does not imply that its implementation has been completely removed from the binary.

## Source and release

The source, build notes, changelog, and logo are in this repository. The ready-to-flash X4 image is attached to the [CrossLeaf 1.0.0 GitHub Release](https://github.com/afinkawan-spec/CrossLeaf/releases/tag/v1.0.0), not committed to Git history.

See [BUILDING.md](BUILDING.md) for the tested Windows build setup. CrossLeaf is derived from [CrossPoint Reader](https://github.com/crosspoint-reader/crosspoint-reader); see [LICENSE](LICENSE) and the vendored FreeInk SDK notices for licensing.
