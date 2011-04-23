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
#include "polyset.h"
#include "context.h"
#include "dxfdata.h"
#include "dxftess.h"
#include "builtin.h"
#include "printutils.h"
#include <assert.h>
#include <boost/concept_check.hpp>
#include <boost/foreach.hpp>

enum primitive_type_e {
	CUBE,
	SPHERE,
	CYLINDER,
	POLYHEDRON,
	SQUARE,
	CIRCLE,
	POLYGON
};

class PrimitiveModule : public AbstractModule
{
public:
	primitive_type_e type;
	PrimitiveModule(primitive_type_e type) : type(type) { }
	virtual AbstractNode *evaluate(const Context *ctx, const ModuleInstantiation *inst) const;
};

class PrimitiveNode : public AbstractPolyNode
{
public:
	bool center;
	double x, y, z, h, r1, r2;
	static const double F_MINIMUM = 0.01;
	double fn, fs, fa;
	primitive_type_e type;
	int convexity;
	Vec2D points2d;
	Vec3D points3d;
	VecPaths paths;
	VecTriangles triangles;
	PrimitiveNode(bool root, bool highlight, bool background, primitive_type_e type) : AbstractPolyNode(root, highlight, background), type(type) {}
	virtual PolySet *render_polyset(render_mode_e mode) const;
	virtual QString dump(QString indent) const;
};

AbstractNode *PrimitiveModule::evaluate(const Context *ctx, const ModuleInstantiation *inst) const
{
	PrimitiveNode *node = new PrimitiveNode(inst->tag_root, inst->tag_highlight, inst->tag_background, type);

	node->center = false;
	node->x = node->y = node->z = node->h = node->r1 = node->r2 = 1;

	QVector<QString> argnames;
	QVector<Expression*> argexpr;

	if (type == CUBE) {
		argnames = QVector<QString>() << "size" << "center";
	}
	if (type == SPHERE) {
		argnames = QVector<QString>() << "r";
	}
	if (type == CYLINDER) {
		argnames = QVector<QString>() << "h" << "r1" << "r2" << "center";
	}
	if (type == POLYHEDRON) {
		argnames = QVector<QString>() << "points" << "triangles" << "convexity";
	}
	if (type == SQUARE) {
		argnames = QVector<QString>() << "size" << "center";
	}
	if (type == CIRCLE) {
		argnames = QVector<QString>() << "r";
	}
	if (type == POLYGON) {
		argnames = QVector<QString>() << "points" << "paths" << "convexity";
	}

	Context c(ctx);
	c.args(argnames, argexpr, inst->argnames, inst->argvalues);

	node->fn = c.lookup_variable("$fn").num;
	node->fs = c.lookup_variable("$fs").num;
	node->fa = c.lookup_variable("$fa").num;

	if (node->fs < PrimitiveNode::F_MINIMUM) {
		PRINTF("WARNING: $fs too small - clamping to %f", PrimitiveNode::F_MINIMUM);
		node->fs = PrimitiveNode::F_MINIMUM;
	}
	if (node->fa < PrimitiveNode::F_MINIMUM) {
		PRINTF("WARNING: $fa too small - clamping to %f", PrimitiveNode::F_MINIMUM);
		node->fa = PrimitiveNode::F_MINIMUM;
	}


	if (type == CUBE) {
		Value size = c.lookup_variable("size");
		Value center = c.lookup_variable("center");
		size.getnum(node->x);
		size.getnum(node->y);
		size.getnum(node->z);
		size.getv3(node->x, node->y, node->z);
		if (center.type == Value::BOOL) {
			node->center = center.b;
		}
	}

	if (type == SPHERE) {
		Value r = c.lookup_variable("r");
		if (r.type == Value::NUMBER) {
			node->r1 = r.num;
		}
	}

	if (type == CYLINDER) {
		Value h = c.lookup_variable("h");
		Value r, r1, r2;
		r1 = c.lookup_variable("r1");
		r2 = c.lookup_variable("r2");
		if (r1.type != Value::NUMBER && r2.type != Value::NUMBER)
			r = c.lookup_variable("r", true); // silence warning since r has no default value
		Value center = c.lookup_variable("center");
		if (h.type == Value::NUMBER) {
			node->h = h.num;
		}
		if (r.type == Value::NUMBER) {
			node->r1 = r.num;
			node->r2 = r.num;
		}
		if (r1.type == Value::NUMBER) {
			node->r1 = r1.num;
		}
		if (r2.type == Value::NUMBER) {
			node->r2 = r2.num;
		}
		if (center.type == Value::BOOL) {
			node->center = center.b;
		}
	}

	if (type == POLYHEDRON) {
//		node->points = c.lookup_variable("points");
//		node->triangles = c.lookup_variable("triangles");
	}

	if (type == SQUARE) {
		Value size = c.lookup_variable("size");
		Value center = c.lookup_variable("center");
		size.getnum(node->x);
		size.getnum(node->y);
		size.getv2(node->x, node->y);
		if (center.type == Value::BOOL) {
			node->center = center.b;
		}
	}

	if (type == CIRCLE) {
		Value r = c.lookup_variable("r");
		if (r.type == Value::NUMBER) {
			node->r1 = r.num;
		}
	}

	if (type == POLYGON) {
//		node->points = c.lookup_variable("points");
//		node->paths = c.lookup_variable("paths");
	}

	node->convexity = c.lookup_variable("convexity", true).num;
	if (node->convexity < 1)
		node->convexity = 1;

	return node;
}

AbstractNode *builtinCube(double x, double y, double z, bool center, bool highlight, bool background, bool root) {
	PrimitiveNode *node = new PrimitiveNode(root, highlight, background, CUBE);
	node->center = center;
	node->x = x; node->y = y; node->z = z;
	node->convexity = 1;
	return node;
}
AbstractNode *builtinSphere(double r, double fn, double fs, double fa, bool center, bool highlight, bool background, bool root) {
	PrimitiveNode *node = new PrimitiveNode(root, highlight, background, SPHERE);
	node->center = center;
	node->r1 = r;
	node->fn = fn;
	node->fs = std::max(fs, PrimitiveNode::F_MINIMUM);
	node->fa = std::max(fa, PrimitiveNode::F_MINIMUM);
	node->convexity = 1;
	return node;
}
AbstractNode *builtinCylinder(double r1, double r2, double h, double fn, double fs, double fa, bool center, bool highlight, bool background, bool root) {
	PrimitiveNode *node = new PrimitiveNode(root, highlight, background, CYLINDER);
	node->center = center;
	node->r1 = r1;
	node->r2 = r2;
	node->h = h;
	node->fn = fn;
	node->fs = std::max(fs, PrimitiveNode::F_MINIMUM);
	node->fa = std::max(fa, PrimitiveNode::F_MINIMUM);
	node->convexity = 1;
	return node;
}
AbstractNode *builtinPolyhedron(const Vec3D &points, const VecTriangles &triangles, int convexity, bool highlight, bool background, bool root) {
	PrimitiveNode *node = new PrimitiveNode(root, highlight, background, POLYHEDRON);
	node->points3d = points;
	node->triangles = triangles;
	node->convexity = convexity;
	return node;
}
AbstractNode *builtinSquare(double x, double y, bool center, bool highlight, bool background, bool root) {
	PrimitiveNode *node = new PrimitiveNode(root, highlight, background, SQUARE);
	node->center = center;
	node->y = x;
	node->y = y;
	node->convexity = 1;
	return node;
}
AbstractNode *builtinCircle(double r, double fn, double fs, double fa, bool center, bool highlight, bool background, bool root) {
	PrimitiveNode *node = new PrimitiveNode(root, highlight, background, CIRCLE);
	node->center = center;
	node->r1 = r;
	node->fn = fn;
	node->fs = std::max(fs, PrimitiveNode::F_MINIMUM);
	node->fa = std::max(fa, PrimitiveNode::F_MINIMUM);
	node->convexity = 1;
	return node;
}
AbstractNode *builtinPolygon(const Vec2D &points, const VecPaths &paths, int convexity, bool highlight, bool background, bool root) {
	PrimitiveNode *node = new PrimitiveNode(root, highlight, background, POLYGON);
	node->points2d = points;
	node->paths = paths;
	node->convexity = convexity;
	return node;
}


void register_builtin_primitives()
{
	builtin_modules["cube"] = new PrimitiveModule(CUBE);
	builtin_modules["sphere"] = new PrimitiveModule(SPHERE);
	builtin_modules["cylinder"] = new PrimitiveModule(CYLINDER);
	builtin_modules["polyhedron"] = new PrimitiveModule(POLYHEDRON);
	builtin_modules["square"] = new PrimitiveModule(SQUARE);
	builtin_modules["circle"] = new PrimitiveModule(CIRCLE);
	builtin_modules["polygon"] = new PrimitiveModule(POLYGON);
}

/*!
	Returns the number of subdivision of a whole circle, given radius and
	the three special variables $fn, $fs and $fa
*/
int get_fragments_from_r(double r, double fn, double fs, double fa)
{
	if (r < GRID_FINE) return 0;
	if (fn > 0.0)
		return (int)fn;
	return (int)ceil(fmax(fmin(360.0 / fa, r*M_PI / fs), 5));
}

static void generate_circle(Vec2D &circle, double r)
{
	for (unsigned int i=0; i<circle.size(); i++) {
		double phi = (M_PI*2* (i + 0.5)) / circle.size();
		circle[i].x = r*cos(phi);
		circle[i].y = r*sin(phi);
	}
}

struct ring_s {
	Vec2D points;
	double z;
};

PolySet *PrimitiveNode::render_polyset(render_mode_e) const
{
	PolySet *p = new PolySet();

	if (type == CUBE && x > 0 && y > 0 && z > 0)
	{
		double x1, x2, y1, y2, z1, z2;
		if (center) {
			x1 = -x/2;
			x2 = +x/2;
			y1 = -y/2;
			y2 = +y/2;
			z1 = -z/2;
			z2 = +z/2;
		} else {
			x1 = y1 = z1 = 0;
			x2 = x;
			y2 = y;
			z2 = z;
		}

		p->append_poly(); // top
		p->append_vertex(x1, y1, z2);
		p->append_vertex(x2, y1, z2);
		p->append_vertex(x2, y2, z2);
		p->append_vertex(x1, y2, z2);

		p->append_poly(); // bottom
		p->append_vertex(x1, y2, z1);
		p->append_vertex(x2, y2, z1);
		p->append_vertex(x2, y1, z1);
		p->append_vertex(x1, y1, z1);

		p->append_poly(); // side1
		p->append_vertex(x1, y1, z1);
		p->append_vertex(x2, y1, z1);
		p->append_vertex(x2, y1, z2);
		p->append_vertex(x1, y1, z2);

		p->append_poly(); // side2
		p->append_vertex(x2, y1, z1);
		p->append_vertex(x2, y2, z1);
		p->append_vertex(x2, y2, z2);
		p->append_vertex(x2, y1, z2);

		p->append_poly(); // side3
		p->append_vertex(x2, y2, z1);
		p->append_vertex(x1, y2, z1);
		p->append_vertex(x1, y2, z2);
		p->append_vertex(x2, y2, z2);

		p->append_poly(); // side4
		p->append_vertex(x1, y2, z1);
		p->append_vertex(x1, y1, z1);
		p->append_vertex(x1, y1, z2);
		p->append_vertex(x1, y2, z2);
	}

	if (type == SPHERE && r1 > 0)
	{


		int fragments = get_fragments_from_r(r1, fn, fs, fa);
		int rings = fragments/2;
// Uncomment the following three lines to enable experimental sphere tesselation
//		if (rings % 2 == 0) rings++; // To ensure that the middle ring is at phi == 0 degrees
		typedef std::vector< ring_s > RingVector;
		RingVector ring(rings);

//		double offset = 0.5 * ((fragments / 2) % 2);
		for (int i = 0; i < rings; i++) {
//			double phi = (M_PI * (i + offset)) / (fragments/2);
			double phi = (M_PI * (i + 0.5)) / rings;
			double r = r1 * sin(phi);
			ring[i].z = r1 * cos(phi);
			ring[i].points.resize(fragments);
			generate_circle(ring[i].points, r);
		}

		p->append_poly();
		for (int i = 0; i < fragments; i++)
			p->append_vertex(ring[0].points[i].x, ring[0].points[i].y, ring[0].z);

		for (int i = 0; i < rings-1; i++) {
			ring_s &r1(ring[i]);
			ring_s &r2(ring[i+1]);
			int r1i = 0, r2i = 0;
			while (r1i < fragments || r2i < fragments)
			{
				if (r1i >= fragments)
					goto sphere_next_r2;
				if (r2i >= fragments)
					goto sphere_next_r1;
				if ((double)r1i / fragments <
						(double)r2i / fragments)
				{
sphere_next_r1:
					p->append_poly();
					int r1j = (r1i+1) % fragments;
					p->insert_vertex(r1.points[r1i].x, r1.points[r1i].y, r1.z);
					p->insert_vertex(r1.points[r1j].x, r1.points[r1j].y, r1.z);
					p->insert_vertex(r2.points[r2i % fragments].x, r2.points[r2i % fragments].y, r2.z);
					r1i++;
				} else {
sphere_next_r2:
					p->append_poly();
					int r2j = (r2i+1) % fragments;
					p->append_vertex(r2.points[r2i].x, r2.points[r2i].y, r2.z);
					p->append_vertex(r2.points[r2j].x, r2.points[r2j].y, r2.z);
					p->append_vertex(r1.points[r1i % fragments].x, r1.points[r1i % fragments].y, r1.z);
					r2i++;
				}
			}
		}
		p->append_poly();
		for (int i = 0; i < fragments; i++)
			p->insert_vertex(ring[rings-1].points[i].x, ring[rings-1].points[i].y, ring[rings-1].z);
	}

	if (type == CYLINDER && h > 0 && r1 >=0 && r2 >= 0 && (r1 > 0 || r2 > 0))
	{
		int fragments = get_fragments_from_r(fmax(r1, r2), fn, fs, fa);

		double z1, z2;
		if (center) {
			z1 = -h/2;
			z2 = +h/2;
		} else {
			z1 = 0;
			z2 = h;
		}

		Vec2D circle1(fragments);
		Vec2D circle2(fragments);

		generate_circle(circle1, r1);
		generate_circle(circle2, r2);
		
		for (int i=0; i<fragments; i++) {
			int j = (i+1) % fragments;
			if (r1 == r2) {
				p->append_poly();
				p->insert_vertex(circle1[i].x, circle1[i].y, z1);
				p->insert_vertex(circle2[i].x, circle2[i].y, z2);
				p->insert_vertex(circle2[j].x, circle2[j].y, z2);
				p->insert_vertex(circle1[j].x, circle1[j].y, z1);
			} else {
				if (r1 > 0) {
					p->append_poly();
					p->insert_vertex(circle1[i].x, circle1[i].y, z1);
					p->insert_vertex(circle2[i].x, circle2[i].y, z2);
					p->insert_vertex(circle1[j].x, circle1[j].y, z1);
				}
				if (r2 > 0) {
					p->append_poly();
					p->insert_vertex(circle2[i].x, circle2[i].y, z2);
					p->insert_vertex(circle2[j].x, circle2[j].y, z2);
					p->insert_vertex(circle1[j].x, circle1[j].y, z1);
				}
			}
		}

		if (r1 > 0) {
			p->append_poly();
			for (int i=0; i<fragments; i++)
				p->insert_vertex(circle1[i].x, circle1[i].y, z1);
		}

		if (r2 > 0) {
			p->append_poly();
			for (int i=0; i<fragments; i++)
				p->append_vertex(circle2[i].x, circle2[i].y, z2);
		}
	}

	if (type == POLYHEDRON)
	{
	  p->convexity = convexity;
	  BOOST_FOREACH( const Triangle &ti, triangles ) {
	    p->append_poly();
	    BOOST_FOREACH( unsigned int pt, ti ) {
	      if (pt < points3d.size()) {
		const Point3D &p3d(points3d[pt]);
		p->insert_vertex(p3d.x, p3d.y, p3d.z);
	      }
	    }
	  }
	}

	if (type == SQUARE)
	{
		double x1, x2, y1, y2;
		if (center) {
			x1 = -x/2;
			x2 = +x/2;
			y1 = -y/2;
			y2 = +y/2;
		} else {
			x1 = y1 = 0;
			x2 = x;
			y2 = y;
		}

		p->is2d = true;
		p->append_poly();
		p->append_vertex(x1, y1);
		p->append_vertex(x2, y1);
		p->append_vertex(x2, y2);
		p->append_vertex(x1, y2);
	}

	if (type == CIRCLE)
	{
		int fragments = get_fragments_from_r(r1, fn, fs, fa);

		p->is2d = true;
		p->append_poly();

		for (int i=0; i < fragments; i++) {
			double phi = (M_PI*2*i) / fragments;
			p->append_vertex(r1*cos(phi), r1*sin(phi));
		}
	}

	if (type == POLYGON) {
	  DxfData dd;
	  BOOST_FOREACH( const Point2D &p2d, points2d ) {
	    dd.points.append(DxfData::Point(p2d.x, p2d.y));
	  }

	  if (paths.size() == 0)
	  {
		  dd.paths.append(DxfData::Path());
		  for (unsigned int i=0; i<points2d.size(); i++) {
			  assert(i < static_cast<unsigned int>(dd.points.size())); // FIXME: Not needed, but this used to be an 'if'
			  DxfData::Point *p = &dd.points[i];
			  dd.paths.last().points.append(p);
		  }
		  if (dd.paths.last().points.size() > 0) {
			  dd.paths.last().points.append(dd.paths.last().points.first());
			  dd.paths.last().is_closed = true;
		  }
	  }
	  else
	  {
	    BOOST_FOREACH( const VecPoints &pi, paths ) {
	      dd.paths.append(DxfData::Path());
	      BOOST_FOREACH( unsigned int idx, pi ) {
		if (idx < static_cast<unsigned int>(dd.points.size())) {
		  DxfData::Point *p = &dd.points[idx];
		  dd.paths.last().points.append(p);
		}
	      }
	      if (dd.paths.last().points.isEmpty()) {
		dd.paths.removeLast();
	      } else {
		dd.paths.last().points.append(dd.paths.last().points.first());
		dd.paths.last().is_closed = true;
	      }
	    }
	  }
	  p->is2d = true;
	  p->convexity = convexity;
	  dxf_tesselate(p, &dd, 0, true, false, 0);
	  dxf_border_to_ps(p, &dd);
	}

	return p;
}

QString PrimitiveNode::dump(QString indent) const
{
	if (dump_cache.isEmpty()) {
		QString text;
		if (type == CUBE)
			text.sprintf("cube(size = [%g, %g, %g], center = %s);\n", x, y, z, center ? "true" : "false");
		if (type == SPHERE)
			text.sprintf("sphere($fn = %g, $fa = %g, $fs = %g, r = %g);\n", fn, fa, fs, r1);
		if (type == CYLINDER)
			text.sprintf("cylinder($fn = %g, $fa = %g, $fs = %g, h = %g, r1 = %g, r2 = %g, center = %s);\n", fn, fa, fs, h, r1, r2, center ? "true" : "false");
		if (type == POLYHEDRON)
			text.sprintf("polyhedron(%d points, %d triangles, convexity = %d);\n", int(points3d.size()), int(triangles.size()), convexity);
		if (type == SQUARE)
			text.sprintf("square(size = [%g, %g], center = %s);\n", x, y, center ? "true" : "false");
		if (type == CIRCLE)
			text.sprintf("circle($fn = %g, $fa = %g, $fs = %g, r = %g);\n", fn, fa, fs, r1);
		if (type == POLYGON)
			text.sprintf("polygon(%d points, %d paths, convexity = %d);\n", int(points2d.size()), int(paths.size()), convexity);
		((AbstractNode*)this)->dump_cache = indent + QString("n%1: ").arg(idx) + text;
	}
	return dump_cache;
}

