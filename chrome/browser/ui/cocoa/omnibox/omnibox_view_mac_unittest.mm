// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "chrome/browser/ui/cocoa/omnibox/omnibox_view_mac.h"

#include "base/string_util.h"
#include "base/sys_string_conversions.h"
#include "base/utf_string_conversions.h"
#include "chrome/browser/autocomplete/autocomplete.h"
#include "testing/platform_test.h"
#include "ui/base/clipboard/clipboard.h"
#include "ui/base/clipboard/scoped_clipboard_writer.h"
#include "ui/gfx/size.h"

namespace {

TEST(OmniboxViewMacTest, GetClipboardText) {
  ui::Clipboard clipboard;
  string16 text;

  // Does an empty clipboard get empty text?
  clipboard.WriteObjects(ui::Clipboard::BUFFER_STANDARD,
                         ui::Clipboard::ObjectMap());
  text = OmniboxViewMac::GetClipboardText(&clipboard);
  EXPECT_EQ(string16(), text);

  const string16 plainText(ASCIIToUTF16("test text"));
  const std::string url("http://www.example.com/");
  const string16 title(ASCIIToUTF16("The Example Company")), title_result;

  // Can we pull straight text off the clipboard?
  {
    ui::ScopedClipboardWriter clipboard_writer(&clipboard,
                                               ui::Clipboard::BUFFER_STANDARD);
    clipboard_writer.WriteText(plainText);
  }

  text = OmniboxViewMac::GetClipboardText(&clipboard);
  EXPECT_EQ(plainText, text);

  // Can we pull a bookmark off the clipboard?
  {
    ui::ScopedClipboardWriter clipboard_writer(&clipboard,
                                               ui::Clipboard::BUFFER_STANDARD);
    clipboard_writer.WriteBookmark(title, url);
  }

  text = OmniboxViewMac::GetClipboardText(&clipboard);
  EXPECT_EQ(ASCIIToUTF16(url), text);

  // Do we pull text in preference to a bookmark?
  {
    ui::ScopedClipboardWriter clipboard_writer(&clipboard,
                                               ui::Clipboard::BUFFER_STANDARD);
    clipboard_writer.WriteText(plainText);
    clipboard_writer.WriteBookmark(title, url);
  }

  text = OmniboxViewMac::GetClipboardText(&clipboard);
  EXPECT_EQ(plainText, text);

  // Do we get nothing if there is neither text nor a bookmark?
  {
    const string16 markup(ASCIIToUTF16("<strong>Hi!</string>"));
    ui::ScopedClipboardWriter clipboard_writer(&clipboard,
                                               ui::Clipboard::BUFFER_STANDARD);
    clipboard_writer.WriteHTML(markup, url);
  }

  text = OmniboxViewMac::GetClipboardText(&clipboard);
  EXPECT_TRUE(text.empty());
}

TEST(OmniboxViewMacTest, GetFieldFont) {
  EXPECT_TRUE(OmniboxViewMac::GetFieldFont());
}

}  // namespace
