/****************************************************************************

Copyright (C) 2002-2013 Gilles Debunne. All rights reserved.

This file is part of the QGLViewer library version 2.4.0.

http://www.libqglviewer.com - contact@libqglviewer.com

This file may be used under the terms of the GNU General Public License 
versions 2.0 or 3.0 as published by the Free Software Foundation and
appearing in the LICENSE file included in the packaging of this file.
In addition, as a special exception, Gilles Debunne gives you certain 
additional rights, described in the file GPL_EXCEPTION in this package.

libQGLViewer uses dual licensing. Commercial/proprietary software must
purchase a libQGLViewer Commercial License.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

*****************************************************************************/

#ifndef _SAVE_SNAPSHOT_H_
#define _SAVE_SNAPSHOT_H_

#include "qglviewer.h"
#include "ui_ImageInterface.h"


class QGLVIEWER_EXPORT ImageInterface: public QDialog, public Ui::ImageInterface
{
	Q_OBJECT
public: 
	ImageInterface(QWidget *parent) ;

private Q_SLOTS:
	void scaleChanged(int s);

private:
	QGLViewer*  viewer_;

	int scale_;
};

#endif

