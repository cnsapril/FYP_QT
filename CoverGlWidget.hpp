#pragma once
/* ---------- include glew (has to be the first) ---------- */
#include <GL\glew.h>
/* ---------- include Qt-related libraries ---------- */
#include <QWidget>
#include <QGLWidget>
#include <qtimer.h>
#include <qdebug.h>
#include <QKeyEvent>
/* ---------- include glm libraries ---------- */
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
/* ---------- include helper functions ---------- */
#include <Helpers\ShapeGenerator.h>
#include <Helpers\Path.h>
#include <Helpers\Constants.h>
/* ---------- include standard libraries ---------- */
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <time.h>
#include <assert.h>
/* ---------- include haptic libraries ---------- */
#include <HD/hd.h>
#include <HDU/hduError.h>
#include <HDU\hduVector.h>

using glm::vec3;
using glm::mat4;

class CoverGlWidget : public QGLWidget {
	Q_OBJECT // Macro included for Qt signals and slots

public:	
	explicit CoverGlWidget(QWidget * parent = Q_NULLPTR);
	~CoverGlWidget();
	void updateSelectionStatus();			// Emits the signal to represent the status of selection mode (on/off)
	void selectionUp();						// Emits the signal that upper button has been pressed within selection mode
	void selectionDown();					// Emits the signal that lower button has been pressed within selection mode

	QImage img_front;
	QImage img_back;
	QImage img_weight;
protected:
	void initializeGL();					// Initialization of OpenGL and OpenHaptics
	void paintGL();							// Main function to draw the graphics scene and initiate communication between graphics thread and haptics thread
	void resizeGL(int w, int h);			// Called when the window is resized
private:
	void sendDataToOpenGL();				// Sends the vertex/color/index data down to OpenGL
	std::string readShaderCode(const char * fileName);
	void installShaders();
	bool checkStatus(GLuint objectID, PFNGLGETSHADERIVPROC objectPropertyGetterFunc, PFNGLGETSHADERINFOLOGPROC getInfoLogFunc, GLenum statusType);
	bool checkShaderStatus(GLuint shaderID);
	bool checkProgramStatus(GLuint programID);
	void initHD();
	void updateWorkspace(hduVector3Dd LLB, hduVector3Dd TRF);
	void DefineForceField();
	void loadTexture(const char *imgPathFront, const char *imgPathBack, const char *imgPathWeight);
	void attachTexture(QImage currentImg);

	QTimer* myTimer;


public slots:
	void coverChanged(int currentIndex);

signals:
	void selectionModeChanged(bool status);
	void selectionButtonChanged(bool upButtonPressed);
};
