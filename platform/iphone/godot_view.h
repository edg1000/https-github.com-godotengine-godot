/*************************************************************************/
/*  godot_view.h                                                         */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#import <AVFoundation/AVFoundation.h>
#import <MediaPlayer/MediaPlayer.h>
#import <UIKit/UIKit.h>

class String;

@class GodotView;
@protocol DisplayLayer;
@protocol GodotViewRendererProtocol;

@protocol GodotViewDelegate

- (BOOL)godotViewFinishedSetup:(GodotView *)view;

@end

@interface GodotView : UIView

@property(assign, nonatomic) id<GodotViewRendererProtocol> renderer;
@property(assign, nonatomic) id<GodotViewDelegate> delegate;

@property(assign, readonly, nonatomic) BOOL isActive;

@property(strong, readonly, nonatomic) CALayer<DisplayLayer> *renderingLayer;
@property(assign, readonly, nonatomic) BOOL canRender;

@property(assign, nonatomic) NSTimeInterval renderingInterval;

- (CALayer<DisplayLayer> *)initializeRendering;
- (void)stopRendering;
- (void)startRendering;

@property(nonatomic, assign) BOOL useCADisplayLink;

@end
