#!/usr/bin/env python
##########################################################################
#
# Copyright 2008-2009 VMware, Inc.
# All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
##########################################################################/


import math
import time

import gobject
import gtk
import gtk.gdk
import gtk.keysyms
import cairo
import pango
import pangocairo


class View:

    def __init__(self):
       self.tx = 0.0 
       self.ty = 0.0 
       self.sx = 1.0 
       self.sy = 1.0 

    def transform(self, x):
        return self.tx + self.sx*x


ROW_HEIGHT = 40


class Label:

    def __init__(self, text):
        self.text = text
        self.fontsize = 14.0
        self.fontname = "Times-Roman"

    def draw(self, cr):
        try:
            layout = self.layout
        except AttributeError:
            layout = cr.create_layout()

            # set font options
            # see http://lists.freedesktop.org/archives/cairo/2007-February/009688.html
            context = layout.get_context()
            fo = cairo.FontOptions()
            fo.set_antialias(cairo.ANTIALIAS_DEFAULT)
            fo.set_hint_style(cairo.HINT_STYLE_NONE)
            fo.set_hint_metrics(cairo.HINT_METRICS_OFF)
            try:
                pangocairo.context_set_font_options(context, fo)
            except TypeError:
                # XXX: Some broken pangocairo bindings show the error
                # 'TypeError: font_options must be a cairo.FontOptions or None'
                pass

            # set font
            font = pango.FontDescription()
            font.set_family(self.fontname)
            font.set_absolute_size(self.fontsize*pango.SCALE)
            layout.set_font_description(font)

            # set text
            layout.set_text(self.text)

            # cache it
            self.layout = layout

        descent = 2 # XXX get descender from font metrics

        width, height = layout.get_size()
        width = float(width)/pango.SCALE
        height = float(height)/pango.SCALE

        cr.update_layout(layout)
        cr.show_layout(layout)


labels = {}


class Call:

    def __init__(self, no, name, starttsc, endtsc):
        self.no = no
        self.name = name
        self.starttsc = starttsc
        self.endtsc = endtsc

        try:
            self.label = labels[name]
        except KeyError:
            self.label = Label(name)
            labels[name] = self.label

    def draw(self, cr, view, row):
        x0 = view.transform(self.starttsc)
        x1 = view.transform(self.endtsc)

        if x1 - x0 > 1:
            x1 -= 0.5

        stride = 6*ROW_HEIGHT

        y0 = stride*(row + 1) - ROW_HEIGHT
        y1 = stride*(row + 1)

        cr.rectangle(x0, y0, x1 - x0, y1 - y0)

        cr.set_source_rgba(0.0, 0.0, 0.0, 1.0)
        cr.fill_preserve()
        cr.fill()

        if x1 - x0 < 20:
            return

        x = view.transform(self.starttsc)
        y = y0 - 0.5*ROW_HEIGHT

        cr.save()

        cr.translate(x, y)
        cr.rotate(-45*math.pi/180)
        
        cr.move_to(0, 0)

        self.label.draw(cr)

        cr.restore()


class Trace:

    def __init__(self):
        self.calls = []
        self.starttsc = None
        self.endtsc = None

    def add_call(self, call):
        self.calls.append(call)

        if self.starttsc is None:
            self.starttsc = call.starttsc
        else:
            self.starttsc = min(self.starttsc, call.starttsc)

        if self.endtsc is None:
            self.endtsc = call.endtsc
        else:
            self.endtsc = max(self.endtsc, call.endtsc)

    def draw(self, cr, view, row):
        for call in self.calls:
            if  call.endtsc < view.starttsc or call.starttsc > view.endtsc:
                continue

            call.draw(cr, view, row)


class TraceSet:

    def __init__(self):
        self.traces = []
        self.starttsc = None
        self.endtsc = None

    def add_trace(self, trace):
        self.traces.append(trace)

        if self.starttsc is None:
            self.starttsc = trace.starttsc
        else:
            self.starttsc = min(self.starttsc, trace.starttsc)

        if self.endtsc is None:
            self.endtsc = trace.endtsc
        else:
            self.endtsc = max(self.endtsc, trace.endtsc)

    def draw(self, cr, view):
        cr.set_source_rgba(0.0, 0.0, 0.0, 1.0)

        cr.set_line_cap(cairo.LINE_CAP_BUTT)
        cr.set_line_join(cairo.LINE_JOIN_MITER)

        for i in range(len(self.traces)):
            trace = self.traces[i]
            trace.draw(cr, view, i)

import xml2txt


class TraceParser(xml2txt.TraceParser):

    def __init__(self, stream, start, end):
        xml2txt.TraceParser.__init__(self, stream)
        self.trace = Trace()
        self.start = start
        self.end = end

    def parse(self):
        xml2txt.TraceParser.parse(self)
        return self.trace

    def handle_call(self, call):
        starttsc = call.attrs['starttsc']
        endtsc = call.attrs['endtsc']

        if self.end > self.start:
            if endtsc < self.start or starttsc > self.end:
                return

        self.trace.add_call(Call(call.no, call.name, starttsc, endtsc))


class DragAction(object):

    def __init__(self, widget):
        self.widget = widget

    def on_button_press(self, event):
        self.startmousex = self.prevmousex = event.x
        self.startmousey = self.prevmousey = event.y
        self.start()

    def on_motion_notify(self, event):
        if event.is_hint:
            x, y, state = event.window.get_pointer()
        else:
            x, y, state = event.x, event.y, event.state
        deltax = self.prevmousex - x
        deltay = self.prevmousey - y
        self.drag(deltax, deltay)
        self.prevmousex = x
        self.prevmousey = y

    def on_button_release(self, event):
        self.stopmousex = event.x
        self.stopmousey = event.y
        self.stop()

    def draw(self, cr):
        pass

    def start(self):
        pass

    def drag(self, deltax, deltay):
        pass

    def stop(self):
        pass

    def abort(self):
        pass


class NullAction(DragAction):

    def on_motion_notify(self, event):
        self.widget.window.set_cursor(gtk.gdk.Cursor(gtk.gdk.ARROW))


class PanAction(DragAction):

    def start(self):
        self.widget.window.set_cursor(gtk.gdk.Cursor(gtk.gdk.FLEUR))

    def drag(self, deltax, deltay):
        self.widget.x += deltax / self.widget.zoom_ratio
        self.widget.y += deltay / self.widget.zoom_ratio
        self.widget.queue_draw()

    def stop(self):
        self.widget.window.set_cursor(gtk.gdk.Cursor(gtk.gdk.ARROW))

    abort = stop


class ZoomAction(DragAction):

    def drag(self, deltax, deltay):
        self.widget.zoom_ratio *= 1.005 ** (deltax + deltay)
        self.widget.zoom_to_fit_on_resize = False
        self.widget.queue_draw()

    def stop(self):
        self.widget.queue_draw()


class ZoomAreaAction(DragAction):

    def drag(self, deltax, deltay):
        self.widget.queue_draw()

    def draw(self, cr):
        cr.save()
        cr.set_source_rgba(.5, .5, 1.0, 0.25)
        cr.rectangle(self.startmousex, self.startmousey,
                     self.prevmousex - self.startmousex,
                     self.prevmousey - self.startmousey)
        cr.fill()
        cr.set_source_rgba(.5, .5, 1.0, 1.0)
        cr.set_line_width(1)
        cr.rectangle(self.startmousex - .5, self.startmousey - .5,
                     self.prevmousex - self.startmousex + 1,
                     self.prevmousey - self.startmousey + 1)
        cr.stroke()
        cr.restore()

    def stop(self):
        x1, y1 = self.widget.window2graph(self.startmousex, self.startmousey)
        x2, y2 = self.widget.window2graph(self.stopmousex, self.stopmousey)
        self.widget.zoom_to_area(x1, y1, x2, y2)

    def abort(self):
        self.widget.queue_draw()


class Widget(gtk.DrawingArea):

    __gsignals__ = {
        'expose-event': 'override',
        'clicked' : (gobject.SIGNAL_RUN_LAST, gobject.TYPE_NONE, (gobject.TYPE_STRING, gtk.gdk.Event))
    }

    def __init__(self):
        gtk.DrawingArea.__init__(self)

        self.traces = TraceSet()

        self.set_flags(gtk.CAN_FOCUS)

        self.add_events(gtk.gdk.BUTTON_PRESS_MASK | gtk.gdk.BUTTON_RELEASE_MASK)
        self.connect("button-press-event", self.on_area_button_press)
        self.connect("button-release-event", self.on_area_button_release)
        self.add_events(gtk.gdk.POINTER_MOTION_MASK | gtk.gdk.POINTER_MOTION_HINT_MASK | gtk.gdk.BUTTON_RELEASE_MASK)
        self.connect("motion-notify-event", self.on_area_motion_notify)
        self.connect("scroll-event", self.on_area_scroll_event)

        self.connect('key-press-event', self.on_key_press_event)

        self.x, self.y = 0.0, 0.0
        self.zoom_ratio = 1.0
        self.zoom_to_fit_on_resize = False
        self.drag_action = NullAction(self)

    def add_trace(self, trace):
        self.traces.add_trace(trace)
        self.zoom_to_fit()

    def do_expose_event(self, event):
        cr = self.window.cairo_create()

        # set a clip region for the expose event
        cr.rectangle(
            event.area.x, event.area.y,
            event.area.width, event.area.height
        )
        cr.clip()

        cr.set_source_rgba(1.0, 1.0, 1.0, 1.0)
        cr.paint()

        cr.save()
        rect = self.get_allocation()

        #cr.translate(0.5*rect.width, 0.5*rect.height)
        #cr.scale(self.zoom_ratio, self.zoom_ratio)
        #cr.translate(-self.x, -self.y)

        view = View()
        view.starttsc = self.x - 0.5*rect.width/self.zoom_ratio
        view.endtsc = self.x + 0.5*rect.width/self.zoom_ratio
        view.tx = 0.5*rect.width - self.x*self.zoom_ratio
        view.ty = 0.5*rect.height
        view.sx = self.zoom_ratio
        view.sy = 1.0

        self.traces.draw(cr, view)
        cr.restore()

        self.drag_action.draw(cr)

        return False

    def zoom_image(self, zoom_ratio, center=False, pos=None):
        if center:
            self.x = (self.traces.starttsc + self.traces.endtsc)/2
            self.y = 0
        elif pos is not None:
            rect = self.get_allocation()
            x, y = pos
            x -= 0.5*rect.width
            y -= 0.5*rect.height
            self.x += x / self.zoom_ratio - x / zoom_ratio
            self.y += y / self.zoom_ratio - y / zoom_ratio
        self.zoom_ratio = zoom_ratio
        self.y = 0.0
        self.queue_draw()

    def zoom_to_area(self, x1, y1, x2, y2):
        rect = self.get_allocation()
        width = abs(x1 - x2)
        height = abs(y1 - y2)
        self.zoom_ratio = min(
            float(rect.width)/float(width),
            float(rect.height)/float(height)
        )
        self.x = (x1 + x2) / 2
        self.y = (y1 + y2) / 2
        self.y = 0.0
        self.queue_draw()

    def zoom_to_fit(self):
        rect = self.get_allocation()
        rect.x += self.ZOOM_TO_FIT_MARGIN
        rect.y += self.ZOOM_TO_FIT_MARGIN
        rect.width -= 2 * self.ZOOM_TO_FIT_MARGIN
        rect.height -= 2 * self.ZOOM_TO_FIT_MARGIN
        zoom_ratio = min(
            float(rect.width)/float(self.traces.endtsc - self.traces.starttsc),
            1.0
        )
        self.zoom_image(zoom_ratio, center=True)

    ZOOM_INCREMENT = 1.25
    ZOOM_TO_FIT_MARGIN = 12

    def on_zoom_in(self, action):
        self.zoom_image(self.zoom_ratio * self.ZOOM_INCREMENT)

    def on_zoom_out(self, action):
        self.zoom_image(self.zoom_ratio / self.ZOOM_INCREMENT)

    def on_zoom_fit(self, action):
        self.zoom_to_fit()

    def on_zoom_100(self, action):
        self.zoom_image(1.0)

    POS_INCREMENT = 100

    def on_key_press_event(self, widget, event):
        if event.keyval == gtk.keysyms.Left:
            self.x -= self.POS_INCREMENT/self.zoom_ratio
            self.queue_draw()
            return True
        if event.keyval == gtk.keysyms.Right:
            self.x += self.POS_INCREMENT/self.zoom_ratio
            self.queue_draw()
            return True
        if event.keyval == gtk.keysyms.Up:
            self.y -= self.POS_INCREMENT/self.zoom_ratio
            self.queue_draw()
            return True
        if event.keyval == gtk.keysyms.Down:
            self.y += self.POS_INCREMENT/self.zoom_ratio
            self.queue_draw()
            return True
        if event.keyval == gtk.keysyms.Page_Up:
            self.zoom_image(self.zoom_ratio * self.ZOOM_INCREMENT)
            self.queue_draw()
            return True
        if event.keyval == gtk.keysyms.Page_Down:
            self.zoom_image(self.zoom_ratio / self.ZOOM_INCREMENT)
            self.queue_draw()
            return True
        if event.keyval == gtk.keysyms.Escape:
            self.drag_action.abort()
            self.drag_action = NullAction(self)
            return True
        return False

    def get_drag_action(self, event):
        state = event.state
        if event.button in (1, 2): # left or middle button
            if state & gtk.gdk.CONTROL_MASK:
                return ZoomAction(self)
            elif state & gtk.gdk.SHIFT_MASK:
                return ZoomAreaAction(self)
            else:
                return PanAction(self)
        return NullAction

    def on_area_button_press(self, area, event):
        self.drag_action.abort()
        self.drag_action = self.get_drag_action(event)
        self.drag_action.on_button_press(event)
        self.pressx = event.x
        self.pressy = event.y
        return False

    def on_area_button_release(self, area, event):
        self.drag_action.on_button_release(event)
        self.drag_action = NullAction(self)
        if event.button == 1 or event.button == 2:
            return True
        return False

    def on_area_scroll_event(self, area, event):
        if event.direction == gtk.gdk.SCROLL_UP:
            self.zoom_image(self.zoom_ratio * self.ZOOM_INCREMENT,
                            pos=(event.x, event.y))
            return True
        if event.direction == gtk.gdk.SCROLL_DOWN:
            self.zoom_image(self.zoom_ratio / self.ZOOM_INCREMENT,
                            pos=(event.x, event.y))
            return True
        return False

    def on_area_motion_notify(self, area, event):
        self.drag_action.on_motion_notify(event)
        return True

    def window2graph(self, x, y):
        rect = self.get_allocation()
        x -= 0.5*rect.width
        y -= 0.5*rect.height
        x /= self.zoom_ratio
        y /= self.zoom_ratio
        x += self.x
        y += self.y
        return x, y


class Window(gtk.Window):

    ui = '''
    <ui>
        <toolbar name="ToolBar">
            <toolitem action="ZoomIn"/>
            <toolitem action="ZoomOut"/>
            <toolitem action="ZoomFit"/>
            <toolitem action="Zoom100"/>
        </toolbar>
    </ui>
    '''

    def __init__(self):
        gtk.Window.__init__(self)

        window = self

        window.set_title('Trace timeline')
        window.set_default_size(800, 600)
        vbox = gtk.VBox()
        window.add(vbox)

        self.widget = Widget()

        # Create a UIManager instance
        uimanager = self.uimanager = gtk.UIManager()

        # Add the accelerator group to the toplevel window
        accelgroup = uimanager.get_accel_group()
        window.add_accel_group(accelgroup)

        # Create an ActionGroup
        actiongroup = gtk.ActionGroup('Actions')
        self.actiongroup = actiongroup

        # Create actions
        actiongroup.add_actions((
            ('ZoomIn', gtk.STOCK_ZOOM_IN, None, None, None, self.widget.on_zoom_in),
            ('ZoomOut', gtk.STOCK_ZOOM_OUT, None, None, None, self.widget.on_zoom_out),
            ('ZoomFit', gtk.STOCK_ZOOM_FIT, None, None, None, self.widget.on_zoom_fit),
            ('Zoom100', gtk.STOCK_ZOOM_100, None, None, None, self.widget.on_zoom_100),
        ))

        # Add the actiongroup to the uimanager
        uimanager.insert_action_group(actiongroup, 0)

        # Add a UI descrption
        uimanager.add_ui_from_string(self.ui)

        # Create a Toolbar
        toolbar = uimanager.get_widget('/ToolBar')
        vbox.pack_start(toolbar, False)

        vbox.pack_start(self.widget)

        self.set_focus(self.widget)

        self.show_all()

    def add_trace(self, trace):
        self.widget.add_trace(trace)


class Main(xml2txt.Main):

    def __init__(self):
        xml2txt.Main.__init__(self)
        self.win = Window()
        self.win.connect('destroy', gtk.main_quit)

    def main(self):
        xml2txt.Main.main(self)
        gtk.main()

    def get_optparser(self):
        optparser = xml2txt.Main.get_optparser(self)
        optparser.add_option("-f", "--from", action="store", type="string", dest="start", default="0", help="from tsc")
        optparser.add_option("-t", "--to", action="store", type="string", dest="stop", default="0", help="until tsc")
        return optparser

    def process_arg(self, stream, options):

        start = int(options.start, 16)
        stop = int(options.stop, 16)

        parser = TraceParser(stream, start, stop)
        trace = parser.parse()

        self.win.add_trace(trace)


if __name__ == '__main__':
    Main().main()
