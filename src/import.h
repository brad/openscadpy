#ifndef IMPORT_H
#define IMPORT_H
/*
 *  OpenSCAD (www.openscad.org)
 *  Copyright (C) 2009-2011 Clifford Wolf <clifford@clifford.at> and
 *                          Marius Kintel <marius@kintel.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  As a special exception, you have permission to link this program
 *  with the CGAL library and distribute executables, as long as you
 *  follow the requirements of the GNU GPL in regard to all of the
 *  software in the executable aside from CGAL.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "module.h"
#include "node.h"
#include "accuracy.h"



class ImportNode : public AbstractPolyNode
{
public:
	typedef shared_ptr< ImportNode > Pointer;
	QString filename;
	int convexity;
	ImportNode(const QString &filename, int convexity, const Props p=Props()) 
	  :AbstractPolyNode(p), filename(filename), convexity(convexity) { }
};

class ImportSTLNode : public ImportNode {
public:	
  ImportSTLNode(const QString &filename, int convexity, const Props p=Props()):ImportNode(filename, convexity, p) {}
  virtual PolySet *render_polyset(render_mode_e mode) const;
  virtual QString dump(QString indent) const;
};

class ImportDXFNode : public ImportNode {
  QString layername;
  Accuracy acc;
  double origin_x, origin_y, scale;
public:	
  ImportDXFNode(const QString &filename,const QString &layername, double origin_x, double origin_y, int convexity=5, double scale=1.0, const Accuracy &acc=Accuracy(), const Props p=Props())
    :ImportNode(filename, convexity, p), layername(layername), acc(acc), origin_x(origin_x), origin_y(origin_y), scale(scale) {}
  virtual PolySet *render_polyset(render_mode_e mode) const;
  virtual QString dump(QString indent) const;
};

class ImportOFFNode : public ImportNode {
public:	
  ImportOFFNode(const QString &filename, int convexity, const Props p=Props()):ImportNode(filename, convexity, p) {}
  virtual PolySet *render_polyset(render_mode_e mode) const;
  virtual QString dump(QString indent) const;
};

#endif