// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/label.h"

#include "base/strings/sys_string_conversions.h"
#include "nativeui/app.h"
#include "nativeui/gfx/font.h"
#include "nativeui/gfx/geometry/size_conversions.h"
#include "nativeui/gfx/mac/painter_mac.h"
#include "nativeui/gfx/mac/text_mac.h"
#include "nativeui/mac/nu_private.h"
#include "nativeui/mac/nu_view.h"

@interface NULabel : NSView<NUView> {
 @private
  nu::NUPrivate private_;
  std::string text_;
  scoped_refptr<nu::Font> font_;
  nu::Color color_;
  nu::Color background_color_;
}
- (void)setText:(const std::string&)text;
- (std::string)text;
@end

@implementation NULabel

- (nu::NUPrivate*)nuPrivate {
  return &private_;
}

- (void)setNUFont:(nu::Font*)font {
  font_ = font;
  [self setNeedsDisplay:YES];
}

- (void)setNUColor:(nu::Color)color {
  color_ = color;
  [self setNeedsDisplay:YES];
}

- (void)setNUBackgroundColor:(nu::Color)color {
  background_color_ = color;
  [self setNeedsDisplay:YES];
}

- (void)setNUEnabled:(BOOL)enabled {
}

- (BOOL)isNUEnabled {
  return YES;
}

- (void)setText:(const std::string&)text {
  text_ = text;
  [self setNeedsDisplay:YES];
}

- (std::string)text {
  return text_;
}

- (void)drawRect:(NSRect)dirtyRect {
  nu::PainterMac painter;
  painter.SetColor(background_color_);
  painter.FillRect(nu::RectF(dirtyRect));
  nu::TextAttributes attributes(font_.get(), color_, nu::TextAlign::Center,
                                nu::TextAlign::Center);
  painter.DrawText(text_, nu::RectF(nu::SizeF([self frame].size)), attributes);
}

@end

namespace nu {

Label::Label(const std::string& text) {
  TakeOverView([[NULabel alloc] init]);
  SetText(text);
  // Default styles.
  App* app = App::GetCurrent();
  [GetNative() setNUFont:app->GetDefaultFont()];
  [GetNative() setNUColor:app->GetColor(App::ThemeColor::Text)];
}

Label::~Label() {
}

void Label::PlatformSetText(const std::string& text) {
  [static_cast<NULabel*>(GetNative()) setText:text];
}

std::string Label::GetText() const {
  return [static_cast<NULabel*>(GetNative()) text];
}

SizeF Label::GetMinimumSize() const {
  TextMetrics metrics = MeasureText(GetText(), -1, TextAttributes());
  metrics.size.Enlarge(1, 1);  // leave space for border
  return metrics.size;
}

}  // namespace nu
