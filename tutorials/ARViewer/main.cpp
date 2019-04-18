
#include "simpleViewer.h"
#include <qapplication.h>

#include <iostream>

int main(int argc, char** argv) {

	// Read command lines arguments.
	QApplication application(argc, argv);

	std::cout << "running...\n";

	// Instantiate the viewer.
	Viewer viewer;

	viewer.setWindowTitle("Simple AR QGLViewer");

	// Make the viewer window visible on screen.
	viewer.show();

	// Run main loop.
	return application.exec();

}
