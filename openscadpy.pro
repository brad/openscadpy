# Auto-include config_<variant>.pri if the VARIANT variable is give on the
# command-line, e.g. qmake VARIANT=mybuild
!isEmpty(VARIANT) {
  message("Variant: $${VARIANT}")
  exists(config_$${VARIANT}.pri) {
    message("Including config_$${VARIANT}.pri")
    include(config_$${VARIANT}.pri)
  }
}

win32 {
  isEmpty(VERSION) VERSION = $$system(date /t)
} else {
  isEmpty(VERSION) VERSION = $$system(date "+%Y.%m.%d")
}


#configure additional directories
win32 {
    INCLUDEPATH += $$(MPIRDIR)
    INCLUDEPATH += $$(MPFRDIR)
}

DEFINES += OPENSCAD_VERSION=$$VERSION
win32:DEFINES += _USE_MATH_DEFINES NOMINMAX _CRT_SECURE_NO_WARNINGS YY_NO_UNISTD_H

#disable warning about too long decorated names
win32:QMAKE_CXXFLAGS += -wd4503


TEMPLATE = app
RESOURCES = openscad.qrc

OBJECTS_DIR = objects
MOC_DIR = objects
UI_DIR = objects
RCC_DIR = objects
INCLUDEPATH += src

macx {
  DEPLOYDIR = $$(MACOSX_DEPLOY_DIR)
  !isEmpty(DEPLOYDIR) {
    INCLUDEPATH += $$DEPLOYDIR/include
    LIBS += -L$$DEPLOYDIR/lib
  }
  # add CONFIG+=deploy to the qmake command-line to make a deployment build
  deploy {
    message("Building deployment version")
    CONFIG += x86 x86_64
  }

  TARGET = OpenSCADpy
  ICON = icons/OpenSCAD.icns
  QMAKE_INFO_PLIST = Info.plist
  APP_RESOURCES.path = Contents/Resources
  APP_RESOURCES.files = OpenSCAD.sdef
  QMAKE_BUNDLE_DATA += APP_RESOURCES
  LIBS += -framework Carbon
}
else {
  TARGET = openscadpy
}

win32 {
  RC_FILE = openscad_win32.rc
}

CONFIG += qt
QT += opengl

# Application configuration
macx:CONFIG += mdi
CONFIG += cgal
CONFIG += opencsg
CONFIG += progresswidget
CONFIG += boost
CONFIG += python

#Uncomment the following line to enable QCodeEdit
#CONFIG += qcodeedit

mdi {
  # MDI needs an OpenCSG library that is compiled with OpenCSG-Reset-Hack.patch applied
  DEFINES += ENABLE_MDI
}

progresswidget {
  DEFINES += USE_PROGRESSWIDGET
  FORMS   += src/ProgressWidget.ui
  HEADERS += src/ProgressWidget.h
  SOURCES += src/ProgressWidget.cc
}

include(cgal.pri)
include(opencsg.pri)
include(eigen2.pri)
include(boost.pri)
include(python.pri)

# Standard include path for misc external libs
#macx {
#  INCLUDEPATH += /opt/local/include
#}

# QMAKE_CFLAGS   += -pg
# QMAKE_CXXFLAGS += -pg
# QMAKE_LFLAGS   += -pg


FORMS   += src/MainWindow.ui \
           src/Preferences.ui

HEADERS += src/CGAL_renderer.h \
           src/GLView.h \
           src/MainWindow.h \
           src/Preferences.h \
           src/cgal.h \
           src/csgterm.h \
           src/csgops.h \
           src/dxfdata.h \
           src/dxfdim.h \
           src/dxftess.h \
           src/export.h \
           src/grid.h \
           src/highlighter.h \
           src/matrix.h \
           src/node.h \
           src/openscad.h \
           src/polyset.h \
           src/printutils.h \
           src/transform.h \
           src/primitives.h \
           src/progress.h \
           src/editor.h \
           src/accuracy.h \
           src/cgaladv.h \
           src/dxflinextrude.h \
           src/dxfrotextrude.h \
           src/import.h \
           src/projection.h \
           src/render.h \
           src/render-opencsg.h \
           src/surface.h \
           src/tostring.h \
           src/mathc99.h

SOURCES += src/openscad.cc \
           src/mainwin.cc \
           src/glview.cc \
           src/export.cc \
	   src/matrix.cc \
           src/node.cc \
           src/csgterm.cc \
           src/polyset.cc \
           src/csgops.cc \
           src/transform.cc \
           src/primitives.cc \
           src/projection.cc \
           src/cgaladv.cc \
	   src/cgaladv_convexhull2.cc \
           src/cgaladv_minkowski3.cc \
           src/cgaladv_minkowski2.cc \
           src/surface.cc \
           src/render.cc \
           src/import.cc \
           src/dxfdata.cc \
           src/dxftess.cc \
           src/dxftess-glu.cc \
           src/dxftess-cgal.cc \
           src/dxfdim.cc \
           src/dxflinextrude.cc \
           src/dxfrotextrude.cc \
           src/highlighter.cc \
           src/printutils.cc \
           src/nef2dxf.cc \
           src/Preferences.cc \
           src/progress.cc \
           src/editor.cc \
	   src/accuracy.cc \
	   src/mathc99.cc

macx {
  HEADERS += src/AppleEvents.h \
             src/EventFilter.h
  SOURCES += src/AppleEvents.cc
}

target.path = /usr/local/bin/
INSTALLS += target

examples.path = /usr/local/share/openscadpy/examples/
examples.files = examples/*
INSTALLS += examples

libraries.path = /usr/local/share/openscadpy/libraries/
libraries.files = libraries/*
INSTALLS += libraries
