/***************************************************************************
                               component.cpp
                              ---------------
    begin                : Sat Aug 23 2003
    copyright            : (C) 2003 by Michael Margraf
    email                : michael.margraf@alumni.tu-berlin.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "components.h"
#include "main.h"
#include "node.h"
#include "viewpainter.h"

#include <qpoint.h>
#include <qpainter.h>
#include <qstring.h>
#include <qpen.h>
#include <qmessagebox.h>
#include <qdir.h>


// ***********************************************************************
// **********                                                   **********
// **********                  class "Component"                **********
// **********                                                   **********
// ***********************************************************************
Component::Component()
{
  Type = isComponent;

  mirroredX = false;
  rotated = 0;
  isSelected = false;
  isActive   = true;

  cx = 0;
  cy = 0;

  Arcs.setAutoDelete(true);
  Lines.setAutoDelete(true);
  Rects.setAutoDelete(true);
  Ellips.setAutoDelete(true);
  Ports.setAutoDelete(true);
  Texts.setAutoDelete(true);
  Props.setAutoDelete(true);
}

Component::~Component()
{
}

Component* Component::newOne()
{
  return new Component();
}

// -------------------------------------------------------
void Component::Bounding(int& _x1, int& _y1, int& _x2, int& _y2)
{
  _x1 = x1+cx;
  _y1 = y1+cy;
  _x2 = x2+cx;
  _y2 = y2+cy;
}

// -------------------------------------------------------
// Size of component text.
void Component::TextSize(int& _dx, int& _dy)
{
  QFontMetrics  metrics(QucsSettings.font);   // get size of text
  int tmp;
  _dx = metrics.width(Name);
  _dy = metrics.height();    // for "Name"
  for(Property *pp = Props.first(); pp != 0; pp = Props.next())
    if(pp->display) {
      // get width of text
      tmp = metrics.width(pp->Name+"="+pp->Value);
      if(tmp > _dx)  _dx = tmp;
      _dy += metrics.height();
    }
}

// -------------------------------------------------------
// Boundings including the component text.
void Component::entireBounds(int& _x1, int& _y1, int& _x2, int& _y2)
{
  _x1 = x1+cx;
  _y1 = y1+cy;
  _x2 = x2+cx;
  _y2 = y2+cy;

  // text boundings
  if(tx < x1) _x1 = tx+cx;
  if(ty < y1) _y1 = ty+cy;

  QFontMetrics  metrics(QucsSettings.font);
  QSize r = metrics.size(0, Name);
  if((tx+r.width()) > x2) _x2 = tx+r.width()+cx;
  if((ty+r.height()) > y2) _y2 = ty+r.height()+cy;

  int dy=r.height(); // due to 'Name' text
  for(Property *pp = Props.first(); pp != 0; pp = Props.next())
    if(pp->display) {
      // get width of text
      r = metrics.size(0, pp->Name+"="+pp->Value);
      if((tx+r.width()) > x2) _x2 = tx+r.width()+cx;
      dy += r.height();
    }
  if((ty+dy) > y2) _y2 = ty+dy+cy;
}

// -------------------------------------------------------
void Component::setCenter(int x, int y, bool relative)
{
  if(relative) { cx += x;  cy += y; }
  else { cx = x;  cy = y; }
}

// -------------------------------------------------------
void Component::getCenter(int& x, int& y)
{
  x = cx;
  y = cy;
}

// -------------------------------------------------------
int Component::getTextSelected(int x_, int y_)
{
  x_ -= cx;
  y_ -= cy;
  if(x_ < tx) return -1;
  if(y_ < ty) return -1;

  x_ -= tx;
  y_ -= ty;
  QFontMetrics  metrics(QucsSettings.font);
  QSize r = metrics.size(0, Name);
  int dy = r.height();
  if(y_ < dy) {
    if(x_ < r.width()) return 0;
    return -1;
  }

  for(Property *pp = Props.first(); pp != 0; pp = Props.next())
    if(pp->display) {
      dy += r.height();   // height is always the same
      if(y_ > dy) continue;
      // get width of text
      r = metrics.size(0, pp->Name+"="+pp->Value);
      if(x_ > r.width()) return -1;
      return Props.at()+1;  // number the property
    }
  
  return -1;
}

// -------------------------------------------------------
bool Component::getSelected(int x_, int y_)
{
  x_ -= cx;
  y_ -= cy;
  if(x_ >= x1) if(x_ <= x2) if(y_ >= y1) if(y_ <= y2)
    return true;

  return false;
}

// -------------------------------------------------------
void Component::paint(ViewPainter *p)
{
  Text *pt;
  int x, y, a, b, xb, yb; //, Size;
  QFont f = p->Painter->font();   // save current font
  QFont newFont = f;

  if(Model.at(0) == '.') {   // is simulation component (dc, ac, ...)
    newFont.setPointSizeFloat(float(p->Scale) * QucsSettings.largeFontSize);
    newFont.setWeight(QFont::DemiBold);
    p->Painter->setFont(newFont);
    p->map(cx, cy, &x, &y);

    p->Painter->setPen(QPen(QPen::darkBlue,2));
    a = b = 0;
    QRect r;
    for(pt = Texts.first(); pt != 0; pt = Texts.next()) {
      p->Painter->drawText(x, y+b, 0, 0, Qt::DontClip, pt->s, -1, &r);
      b += r.height();
      if(a < r.width())  a = r.width();
    }
    xb = a + int(12.0*p->Scale);
    yb = b + int(10.0*p->Scale);
    x2 = x1+25 + int(float(a) / p->Scale);
    y2 = y1+23 + int(float(b) / p->Scale);
    if(ty < y2+1) if(ty > y1-r.height())  ty = y2 + 1;

    p->map(cx-1, cy,   &x, &y);
    p->map(cx-6, cy-5, &a, &b);
    p->Painter->drawRect(a, b, xb, yb);
    p->Painter->drawLine(x,      y+yb, a,      b+yb);
    p->Painter->drawLine(x+xb-1, y+yb, x,      y+yb);
    p->Painter->drawLine(x+xb-1, y+yb, a+xb,   b+yb);
    p->Painter->drawLine(x+xb-1, y+yb, x+xb-1, y);
    p->Painter->drawLine(x+xb-1, y,    a+xb,   b);
  }
  else {    // normal components go here

    // paint all lines
    for(Line *p1 = Lines.first(); p1 != 0; p1 = Lines.next()) {
      p->Painter->setPen(p1->style);
      p->drawLine(cx+p1->x1, cy+p1->y1, cx+p1->x2, cy+p1->y2);
    }

    // paint all arcs
    for(Arc *p3 = Arcs.first(); p3 != 0; p3 = Arcs.next()) {
      p->Painter->setPen(p3->style);
      p->drawArc(cx+p3->x, cy+p3->y, p3->w, p3->h, p3->angle, p3->arclen);
    }

    // paint all rectangles
    Area *pa;
    for(pa = Rects.first(); pa != 0; pa = Rects.next()) {
      p->Painter->setPen(pa->Pen);
      p->Painter->setBrush(pa->Brush);
      p->drawRect(cx+pa->x, cy+pa->y, pa->w, pa->h);
    }

    // paint all ellipses
    for(pa = Ellips.first(); pa != 0; pa = Ellips.next()) {
      p->Painter->setPen(pa->Pen);
      p->Painter->setBrush(pa->Brush);
      p->drawEllipse(cx+pa->x, cy+pa->y, pa->w, pa->h);
    }
    p->Painter->setBrush(QBrush::NoBrush);

    newFont.setWeight(QFont::Light);
    p->Painter->setPen(QPen(QPen::black,1));
    // write all text
    for(pt = Texts.first(); pt != 0; pt = Texts.next()) {
      newFont.setPointSizeFloat(p->Scale * pt->Size);
      p->Painter->setFont(newFont);
      p->Painter->setPen(pt->Color);
      p->drawText(pt->s, cx+pt->x, cy+pt->y);
    }
  }
  p->Painter->setFont(f);


  p->Painter->setPen(QPen(QPen::black,1));
  int Height = p->LineSpacing;
  p->map(cx+tx, cy+ty, &x, &y);
  p->Painter->drawText(x, y, 0, 0, Qt::DontClip, Name);
  // write all properties
  for(Property *p4 = Props.first(); p4 != 0; p4 = Props.next())
    if(p4->display) {
      y += Height;
      p->Painter->drawText(x, y, 0, 0, Qt::DontClip, p4->Name+"="+p4->Value);
    }

  if(!isActive) {
    p->Painter->setPen(QPen(QPen::red,0));
    p->drawRect(cx+x1, cy+y1, x2-x1+1, y2-y1+1);
    p->drawLine(cx+x1, cy+y1, cx+x2, cy+y2);
    p->drawLine(cx+x1, cy+y2, cx+x2, cy+y1);
  }

  if(isSelected) {
    p->Painter->setPen(QPen(QPen::darkGray,3));
    p->drawRoundRect(cx+x1, cy+y1, x2-x1, y2-y1);
  }
}

// -------------------------------------------------------
// Paints the component when moved with the mouse.
void Component::paintScheme(QPainter *p)
{
  if(Model.at(0) == '.') {   // is simulation component (dc, ac, ...)
    int xb = x2-x1-13;
    int yb = y2-y1-13;

    p->drawRect(cx-6, cy-5, xb, yb);
    p->drawLine(cx-1, cy+yb, cx-6, cy+yb-5);
    p->drawLine(cx+xb-2, cy+yb, cx-1, cy+yb);
    p->drawLine(cx+xb-2, cy+yb, cx+xb-6, cy+yb-5);
    p->drawLine(cx+xb-2, cy+yb, cx+xb-2, cy);
    p->drawLine(cx+xb-2, cy, cx+xb-6, cy-5);
    return;
  }

  // paint all lines
  for(Line *p1 = Lines.first(); p1 != 0; p1 = Lines.next())
    p->drawLine(cx+p1->x1, cy+p1->y1, cx+p1->x2, cy+p1->y2);

  // paint all ports
  for(Port *p2 = Ports.first(); p2 != 0; p2 = Ports.next())
    p->drawEllipse(cx+p2->x-4, cy+p2->y-4, 8, 8);

  for(Arc *p3 = Arcs.first(); p3 != 0; p3 = Arcs.next())   // paint all arcs
    p->drawArc(cx+p3->x, cy+p3->y, p3->w, p3->h, p3->angle, p3->arclen);

  Area *pa;
  for(pa = Rects.first(); pa != 0; pa = Rects.next()) // paint all rectangles
    p->drawRect(cx+pa->x, cy+pa->y, pa->w, pa->h);

  for(pa = Ellips.first(); pa != 0; pa = Ellips.next()) // paint all ellipses
    p->drawEllipse(cx+pa->x, cy+pa->y, pa->w, pa->h);
}

// -------------------------------------------------------
// For output on a printer device.
void Component::print(ViewPainter *p)
{
  Arc *pa;
  for(pa = Arcs.first(); pa != 0; pa = Arcs.next()) {
    pa->w -= 1;   // to look nice after printing, arcs have to be
    pa->h -= 1;   // made smaller
  }

  paint(p);

  for(pa = Arcs.first(); pa != 0; pa = Arcs.next()) {
    pa->w += 1;   // back to old size
    pa->h += 1;
  }
}

// -------------------------------------------------------
// Set position of property text to not interfere with component
// symbol.
void Component::recreate()
{
}

// -------------------------------------------------------
// Rotates the component 90� counter-clockwise around its center
void Component::rotate()
{
  if(Ports.count() < 1) return;  // do not rotate components without ports
  int tmp, dx, dy;

  // rotate all lines
  for(Line *p1 = Lines.first(); p1 != 0; p1 = Lines.next()) {
    tmp = -p1->x1;
    p1->x1 = p1->y1;
    p1->y1 = tmp;
    tmp = -p1->x2;
    p1->x2 = p1->y2;
    p1->y2 = tmp;
  }

  // rotate all ports
  for(Port *p2 = Ports.first(); p2 != 0; p2 = Ports.next()) {
    tmp = -p2->x;
    p2->x = p2->y;
    p2->y = tmp;
  }

  // rotate all arcs
  for(Arc *p3 = Arcs.first(); p3 != 0; p3 = Arcs.next()) {
    tmp = -p3->x;
    p3->x = p3->y;
    p3->y = tmp - p3->w +1; // +1 is beauty correction
    tmp = p3->w;
    p3->w = p3->h;
    p3->h = tmp;
    p3->angle += 16*90;
    if(p3->angle >= 16*360) p3->angle -= 16*360;;
  }

  Area *pa;
  // rotate all rectangles
  for(pa = Rects.first(); pa != 0; pa = Rects.next()) {
    tmp = -pa->x;
    pa->x = pa->y;
    pa->y = tmp - pa->w;
    tmp = pa->w;
    pa->w = pa->h;
    pa->h = tmp;
  }

  // rotate all ellipses
  for(pa = Ellips.first(); pa != 0; pa = Ellips.next()) {
    tmp = -pa->x;
    pa->x = pa->y;
    pa->y = tmp - pa->w;
    tmp = pa->w;
    pa->w = pa->h;
    pa->h = tmp;
  }

  // rotate all text
  QFont f = QucsSettings.font;
  for(Text *pt = Texts.first(); pt != 0; pt = Texts.next()) {
    f.setPointSizeFloat(pt->Size);
    QFontMetrics  smallMetrics(f);
    dx = smallMetrics.width(pt->s) >> 1;
    dy = smallMetrics.lineSpacing() >> 1;
    tmp = -pt->x;
    pt->x = pt->y + dy - dx;
    pt->y = tmp - dx - dy;
  }

  tmp = -x1;   // rotate boundings
  x1  = y1; y1 = -x2;
  x2  = y2; y2 = tmp;

  tmp = -tx;    // rotate text position
  tx  = ty;
  ty  = tmp;
  QFontMetrics  metrics(QucsSettings.font);   // get size of text
  dx = metrics.width(Name);
  dy = metrics.lineSpacing();
  for(Property *pp = Props.first(); pp != 0; pp = Props.next())
    if(pp->display) {
      // get width of text
      tmp = metrics.width(pp->Name+"="+pp->Value);
      if(tmp > dx) dx = tmp;
      dy += metrics.lineSpacing();
    }
  if(tx > x2) ty = y1-ty+y2;    // rotate text position
  else if(ty < y1) ty -= dy;
  else if(tx < x1) { tx += dy-dx;  ty = y1-ty+y2; }
  else ty -= dx;

  rotated++;  // keep track of what's done
  rotated &= 3;
}

// -------------------------------------------------------
// Mirrors the component about the x-axis.
void Component::mirrorX()
{
  if(Ports.count() < 1) return;  // do not rotate components without ports

  // mirror all lines
  for(Line *p1 = Lines.first(); p1 != 0; p1 = Lines.next()) {
    p1->y1 = -p1->y1;
    p1->y2 = -p1->y2;
  }

  // mirror all ports
  for(Port *p2 = Ports.first(); p2 != 0; p2 = Ports.next())
    p2->y = -p2->y;

  // mirror all arcs
  for(Arc *p3 = Arcs.first(); p3 != 0; p3 = Arcs.next()) {
    p3->y = -p3->y - p3->h + 1;   // +1 is beauty correction
    if(p3->angle > 16*180) p3->angle -= 16*360;
    p3->angle  = -p3->angle;    // mirror
    p3->angle -= p3->arclen;    // go back to end of arc
    if(p3->angle < 0) p3->angle += 16*360;  // angle has to be > 0
  }

  Area *pa;
  // mirror all rectangles
  for(pa = Rects.first(); pa != 0; pa = Rects.next())
    pa->y = -pa->y - pa->h;

  // mirror all ellipses
  for(pa = Ellips.first(); pa != 0; pa = Ellips.next())
    pa->y = -pa->y - pa->h;

  QFont f = QucsSettings.font;
  int dy;
  // mirror all text
  for(Text *pt = Texts.first(); pt != 0; pt = Texts.next()) {
    f.setPointSizeFloat(pt->Size);
    QFontMetrics  smallMetrics(f);
    dy = smallMetrics.lineSpacing();   // height of text
//    pt->y = -pt->y - (dy & 0xFFFFFFFE); // erase LSB to be rotate consistent
    pt->y = -pt->y - dy;
  }

  int tmp = y1;
  y1  = -y2; y2 = -tmp;   // mirror boundings

  QFontMetrics  metrics(QucsSettings.font);   // get size of text
  dy = metrics.lineSpacing();    // for "Name"
  for(Property *pp = Props.first(); pp != 0; pp = Props.next())
    if(pp->display)  dy += metrics.lineSpacing();
  if((tx > x1) && (tx < x2)) ty = -ty-dy;     // mirror text position
  else ty = y1+ty+y2;

  mirroredX = !mirroredX;    // keep track of what's done
  rotated += rotated << 1;
  rotated &= 3;
}

// -------------------------------------------------------
// Mirrors the component about the y-axis.
void Component::mirrorY()
{
  if(Ports.count() < 1) return;  // do not rotate components without ports

  // mirror all lines
  for(Line *p1 = Lines.first(); p1 != 0; p1 = Lines.next()) {
    p1->x1 = -p1->x1;
    p1->x2 = -p1->x2;
  }

  // mirror all ports
  for(Port *p2 = Ports.first(); p2 != 0; p2 = Ports.next())
    p2->x = -p2->x;

  // mirror all arcs
  for(Arc *p3 = Arcs.first(); p3 != 0; p3 = Arcs.next()) {
    p3->x = -p3->x - p3->w + 1;   // +1 is beauty correction
    p3->angle = 16*180 - p3->angle - p3->arclen;  // mirror
    if(p3->angle < 0) p3->angle += 16*360;   // angle has to be > 0
  }

  Area *pa;
  // mirror all rectangles
  for(pa = Rects.first(); pa != 0; pa = Rects.next())
    pa->x = -pa->x - pa->w;

  // mirror all ellipses
  for(pa = Ellips.first(); pa != 0; pa = Ellips.next())
    pa->x = -pa->x - pa->w;

  int tmp;
  QFont f = QucsSettings.font;
  // mirror all text
  for(Text *pt = Texts.first(); pt != 0; pt = Texts.next()) {
    f.setPointSizeFloat(pt->Size);
    QFontMetrics  smallMetrics(f);
    tmp = smallMetrics.width(pt->s);   // width of text
//    pt->x = -pt->x - (tmp & 0xFFFFFFFE); // erase LSB to be rotate consistent
    pt->x = -pt->x - tmp;
  }

  tmp = x1;
  x1  = -x2; x2 = -tmp;   // mirror boundings

  QFontMetrics  metrics(QucsSettings.font);   // get size of text
  int dx = metrics.width(Name);
  for(Property *pp = Props.first(); pp != 0; pp = Props.next())
    if(pp->display) {
      // get width of text
      tmp = metrics.width(pp->Name+"="+pp->Value);
      if(tmp > dx)  dx = tmp;
    }
  if((ty > y1) && (ty < y2)) tx = -tx-dx;     // mirror text position
  else tx = x1+tx+x2;

  mirroredX = !mirroredX;   // keep track of what's done
  rotated += rotated << 1;
  rotated += 2;
  rotated &= 3;
}

// -------------------------------------------------------
QString Component::NetList()
{
  if(!isActive) return QString("");       // should it be simulated ?

  QString s = Model+":"+Name;

  // output all node names
  for(Port *p1 = Ports.first(); p1 != 0; p1 = Ports.next())
    s += " "+p1->Connection->Name;   // node names
  if(Model.at(0) == '_') {    // add port ? (e.g. BJT without substrate)
    if(Model == "_BJT")
      s += " "+Ports.at(1)->Connection->Name;
    else
      s += " "+Ports.at(2)->Connection->Name;
    s.remove(0,1);  // remove leading '_'
  }
  else if(Model == "MVIA")
    s += " gnd";   // add ground node

  // output all properties
  for(Property *p2 = Props.first(); p2 != 0; p2 = Props.next())
    if(p2->Name != "Symbol")
      s += " "+p2->Name+"=\""+p2->Value+"\"";

  return s;
}

// -------------------------------------------------------
QString Component::save()
{
  QString s = "<";
  if(Model.at(0) == '#')
    s  += Model.mid(1) + QString::number(Ports.count()-1);
  else s += Model;

  if(Name.isEmpty()) s += " *";
  else s += " "+Name;

  if(isActive) s += " 1";
  else s += " 0";
  s += " "+QString::number(cx)+" "+QString::number(cy);
  s += " "+QString::number(tx)+" "+QString::number(ty);
  if(mirroredX) s += " 1";
  else s += " 0";
  s += " "+QString::number(rotated);

  // write all properties
  for(Property *p1 = Props.first(); p1 != 0; p1 = Props.next()) {
    if(p1->Description.isEmpty())
      s += " \""+p1->Name+"="+p1->Value+"\"";   // e.g. for equations
    else s += " \""+p1->Value+"\"";
    if(p1->display) s += " 1";
    else s += " 0";
  }

  return s+">";
}

// -------------------------------------------------------
bool Component::load(const QString& _s)
{
  bool ok;
  int  ttx, tty, tmp;
  QString s = _s;

  if(s.at(0) != '<') return false;
  if(s.at(s.length()-1) != '>') return false;
  s = s.mid(1, s.length()-2);   // cut off start and end character

  QString n;
  Name = s.section(' ',1,1);    // Name
  if(Name == "*") Name = "";

  n  = s.section(' ',2,2);      // isActive
  if(n.toInt(&ok) == 1) isActive = true;
  else isActive = false;
  if(!ok) return false;

  n  = s.section(' ',3,3);    // cx
  cx = n.toInt(&ok);
  if(!ok) return false;

  n  = s.section(' ',4,4);    // cy
  cy = n.toInt(&ok);
  if(!ok) return false;

  n  = s.section(' ',5,5);    // tx
  ttx = n.toInt(&ok);
  if(!ok) return false;

  n  = s.section(' ',6,6);    // ty
  tty = n.toInt(&ok);
  if(!ok) return false;

if(Model.at(0) != '.') {  // is simulation component (dc, ac, ...) ?

  n  = s.section(' ',7,7);    // mirroredX
  if(n.toInt(&ok) == 1) mirrorX();  // mirror component
  if(!ok) return false;

  n  = s.section(' ',8,8);    // rotated
  tmp = n.toInt(&ok);
  if(!ok) return false;
  for(int z=0; z<tmp; z++) rotate();   // rotate component

}

  tx = ttx; ty = tty; // restore text position (was changed by rotate/mirror)

  int z=0;
  unsigned int counts = s.contains('"') >> 1;
  // load all properties
  for(Property *p1 = Props.first(); p1 != 0; p1 = Props.next()) {
    z++;
    n = s.section('"',z,z);    // property value
    // not all properties have to be mentioned (backward compatible)
    if(n.isEmpty()) {
      if(p1->Description.isEmpty())
        Props.remove();    // remove if allocated in vain
      return true;
    }

    z++;
    if(p1->Description.isEmpty()) {  // unknown number of properties ?
      p1->Name = n.section('=',0,0);
      n = n.section('=',1,1);
      // allocate memory for a new property (e.g. for equations)
      if(Props.count() < counts) {
	Props.insert(z >> 1, new Property("y", "1", true));
	Props.prev();
      }
    }
    p1->Value = n;

    n  = s.section('"',z,z);    // display
    if(n.toInt(&ok) == 1) p1->display = true;
    else p1->display = false;
    if(!ok) return false;
  }

//  if(Model.at(0) != '.') {  // is simulation component (dc, ac, ...) ?
//  }

  return true;
}



// ***********************************************************************
// ********                                                       ********
// ******** The following function does not below to any class.   ********
// ******** It creates a component by getting the identification  ********
// ******** string used in the schematic file and for copy/paste. ********
// ********                                                       ********
// ***********************************************************************

Component* getComponentFromName(QString& Line)
{
  Component *c = 0;

  Line = Line.stripWhiteSpace();
  if(Line.at(0) != '<') {
    QMessageBox::critical(0, QObject::tr("Error"),
			QObject::tr("Format Error:\nWrong line start!"));
    return 0;
  }

  QString cstr = Line.section(' ',0,0); // component type
  char first = Line.at(1).latin1();     // first letter of component name
  cstr.remove(0,2);    // remove leading "<" and first letter

  // to speed up the string comparision, they are ordered by the first
  // letter of their name
  switch(first) {
  case 'R' : if(cstr.isEmpty()) c = new Resistor();
	else if(cstr == "us") c = new Resistor(false);
	break;
  case 'C' : if(cstr.isEmpty()) c = new Capacitor();
	else if(cstr == "CCS") c = new CCCS();
	else if(cstr == "CVS") c = new CCVS();
	else if(cstr == "irculator") c = new Circulator();
	else if(cstr == "LIN") c = new Coplanar();
	else if(cstr == "OPEN") c = new CPWopen();
	else if(cstr == "SHORT") c = new CPWshort();
	else if(cstr == "GAP") c = new CPWgap();
	break;
  case 'L' : if(cstr.isEmpty()) c = new Inductor();
	break;
  case 'G' : if(cstr == "ND") c = new Ground();
        else if(cstr == "yrator") c = new Gyrator();
        break;
  case 'I' : if(cstr == "Probe") c = new iProbe();
        else if(cstr == "dc") c = new Ampere_dc();
        else if(cstr == "ac") c = new Ampere_ac();
        else if(cstr == "noise") c = new Ampere_noise();
        else if(cstr == "solator") c = new Isolator();
        else if(cstr == "pulse") c = new iPulse();
        else if(cstr == "rect") c = new iRect();
        break;
  case 'J' : if(cstr == "FET") c = new JFET();
        break;
  case 'V' : if(cstr == "dc") c = new Volt_dc();
        else if(cstr == "ac") c = new Volt_ac();
        else if(cstr == "CCS") c = new VCCS();
        else if(cstr == "CVS") c = new VCVS();
        else if(cstr == "noise") c = new Volt_noise();
        else if(cstr == "pulse") c = new vPulse();
        else if(cstr == "rect") c = new vRect();
        break;
  case 'T' : if(cstr == "r") c = new Transformer();
        else if(cstr == "LIN") c = new TLine();
        break;
  case 's' : if(cstr == "Tr") c = new symTrafo();
        break;
  case 'P' : if(cstr == "ac") c = new Source_ac();
        else if(cstr == "ort") c = new SubCirPort();
        else if(cstr == "Shift") c = new Phaseshifter();
        break;
  case 'S' : if(cstr.left(5) == "Pfile") {
	       c = new SParamFile(cstr.mid(5).toInt()); }
        else if(cstr == "ub")   c = new Subcircuit();
        else if(cstr == "UBST") c = new Substrate();
        else if(cstr == "PICE") c = new SpiceFile();
        break;
  case 'D' : if(cstr == "CBlock") c = new dcBlock();
	else if(cstr == "CFeed") c = new dcFeed();
	else if(cstr == "iode") c = new Diode();
	break;
  case 'B' : if(cstr == "iasT") c = new BiasT();
	else if(cstr == "JT") c = new BJTsub();
	break;
  case 'A' : if(cstr == "ttenuator") c = new Attenuator();
	else if(cstr == "mp") c = new Amplifier();
        break;
  case 'M' : if(cstr == "LIN") c = new MSline();
	else if(cstr == "OSFET") c = new MOSFET_sub();
	else if(cstr == "STEP") c = new MSstep();
	else if(cstr == "CORN") c = new MScorner();
	else if(cstr == "TEE") c = new MStee();
	else if(cstr == "CROSS") c = new MScross();
	else if(cstr == "MBEND") c = new MSmbend();
	else if(cstr == "OPEN") c = new MSopen();
	else if(cstr == "GAP") c = new MSgap();
	else if(cstr == "COUPLED") c = new MScoupled();
	else if(cstr == "VIA") c = new MSvia();
	break;
  case 'E' : if(cstr == "qn") c = new Equation();
        break;
  case 'O' : if(cstr == "pAmp") c = new OpAmp();
        break;
  case '.' : if(cstr == "DC") c = new DC_Sim();
        else if(cstr == "AC") c = new AC_Sim();
        else if(cstr == "TR") c = new TR_Sim();
        else if(cstr == "SP") c = new SP_Sim();
        else if(cstr == "HB") c = new HB_Sim();
        else if(cstr == "SW") c = new Param_Sweep();
        break;
  case '_' : if(cstr == "BJT") c = new BJT();
	else if(cstr == "MOSFET") c = new MOSFET();
        break;
  }
  if(!c) {
    QMessageBox::critical(0, QObject::tr("Error"),
	QObject::tr("Format Error:\nUnknown component!"));
    return 0;
  }

  if(!c->load(Line)) {
    QMessageBox::critical(0, QObject::tr("Error"),
	QObject::tr("Format Error:\nWrong 'component' line format!"));
    delete c;
    return 0;
  }

  cstr = c->Name;   // is perhaps changed in "recreate" (z.B. subcircuit)
  int x = c->tx, y = c->ty;
  c->recreate();
  c->Name = cstr;
  c->tx = x;  c->ty = y;
  return c;
}
