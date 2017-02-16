#include "CoverGlWidget.hpp"

#include <Helpers\shared.h>
#include <Helpers\sharedInit.h>
#include <Helpers\AccSynthHashMatrix.h>
#include <Helpers\autoGenHashMatrix.h>
#include <Helpers\ContactModel.h>
#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>


// iPhone 7 Plus specifications
#define IPHONE_WIDTH 77.9
#define IPHONE_HEIGHT 158.2
#define IPHONE_DEPTH 7.3

// Scaling factors
#define SCALE_IPHONE_WIDTH() 1.0f / IPHONE_HEIGHT * IPHONE_WIDTH * 100
#define SCALE_IPHONE_HEIGHT() 1.0f * 100
#define SCALE_IPHONE_DEPTH() 1.0f / IPHONE_HEIGHT * IPHONE_DEPTH * 100
#define VISITOR_SPHERE_RADIUS 1.6

// OpenGL vertex/index/uv buffers
GLuint cubeVertexBuffer;
GLuint cubeIndexBuffer;
GLuint cubeUVBuffer;
GLuint sphereVertexBuffer;
GLuint sphereIndexBuffer;

// OpenGL number of indices
GLuint cubeNumIndices;
GLuint sphereNumIndices;

// OpenGL program IDs
GLuint programID;
GLuint sphereProgramID;

// OpenGL textures
GLuint texture2D_front;
GLuint texture2D_back;
std::string frontCoverPath = "./Resources/Images/front.png";
std::string backCoverPath = "./Resources/Images/back.png";

// OpenGL transform matrices
mat4 projectionMatrix;
mat4 viewMatrix;
mat4 fullTransformMatrixLeft, fullTransformMatrixRight;

// OpenHaptics weight variables
GLuint weightArray[] = { 188, 198, 198, 198, 198, 198, 198, 198, 198, 198, 208, 208, 208, 208, 208, 208, 208 };
GLuint weight = weightArray[0];
std::string weightText = weightTextPath[0];

// OpenHaptics texture model file numbers
GLuint textureArray[] = { 84, 23, 23, 23, 23, 23, 23, 23, 23, 23, 47, 47, 47, 47, 47, 47, 47 };

// OpenHaptics variables
HHD ghHD = HD_INVALID_HANDLE;
HDSchedulerHandle gSchedulerCallback = HD_INVALID_HANDLE;
ContactModel* gContactModelHS;

hduVector3Dd priorPosition(0, 0, 0);
hduVector3Dd priorVelocity(0, 0, 0);
hduVector3Dd priorpriorVelocity(0, 0, 0);

hduVector3Dd visitorPos;

AccSynthHashMatrix myMatrix;
boost::mt19937 rng;

// Precise timing variables
char clockResult[100];
LARGE_INTEGER ticksPerSecond;
LARGE_INTEGER thisTime;
LARGE_INTEGER lastTime;
LARGE_INTEGER deltaTime;
double deltaTimeS;
LARGE_INTEGER trialTime;
double trialTimeS;

//pAudio variables
std::vector <float> outputHist;
std::vector <float> excitationHist;

// Mode control variables
bool text_mode = true; // true - texture mode; false - weight mode
bool selection_mode = false;
bool selection_lock = false;

struct Wrapper
{
	CoverGlWidget* coverGlWidget;
	ContactModel* gContactModelHS;
};

struct Synchronizer
{
	HHD m_hHD;

	// Will be assigned to global pointer modified by haptics thread
	ContactModel *pContactModelHS;

	// pClientData will be coped and assigned into these, thread safe
	hduVector3Dd visitorPosition;
	hduVector3Dd forceOnVisitor;
};

struct DeviceDisplayState
{
	HHD m_hHD;
	double position[3];
	double force[3];
};

HDCallbackCode HDCALLBACK GetStateCB(void *pUserData)
{
	Synchronizer *pSynchronizer = static_cast<Synchronizer*>(pUserData);

	// pSynchronizer->pContactModelHS points to same data structure passed
	// in the haptics thread callback. So it is updated every servoloop tick
	// and is not thread safe to access directly.

	pSynchronizer->visitorPosition =
		pSynchronizer->pContactModelHS->GetCurrentContactPoint();

	pSynchronizer->forceOnVisitor =
		pSynchronizer->pContactModelHS->GetCurrentForceOnVisitor();

	return HD_CALLBACK_DONE;
}

HDCallbackCode HDCALLBACK DeviceStateCallback(void *pUserData)
{
	DeviceDisplayState *pDisplayState =
		static_cast<DeviceDisplayState *>(pUserData);

	HHD hHD = hdGetCurrentDevice();
	hdBeginFrame(hHD);
	hdGetDoublev(HD_CURRENT_POSITION, pDisplayState->position);
	hdGetDoublev(HD_CURRENT_FORCE, pDisplayState->force);
	hdEndFrame(hHD);

	return HD_CALLBACK_DONE;
}

double vibrations()
{
	double output = 0.0;
	double excitation = 0.0;
	double rgen_mean = 0.;
	boost::mt19937 generator;

	//Double buffered, if buffer 1:
	if (SynthesisFlag_Buffer1) {
		//generate Gaussian random number with power equal to interpolation model variance
		boost::normal_distribution<> nd(rgen_mean, sqrt(filtVariance_buf1));
		boost::variate_generator<boost::mt19937&,
			boost::normal_distribution<> > var_nor(rng, nd);
		excitation = var_nor();
		output = 0.0;

		//if the size of output history is less than the number of AR coefficients, append zeros
		if (outputHist.size()<(unsigned int)MAX_COEFF) {
			int subt = MAX_COEFF - outputHist.size();
			for (int j = 0; j < subt; j++) {
				outputHist.push_back(0.0);
			}
		}
		//if the size of excitation history is less than the number of MA coefficients, append zeros
		if (excitationHist.size()<(unsigned int)MAX_MACOEFF) {
			int subt = MAX_MACOEFF - excitationHist.size();
			for (int j = 0; j < subt; j++) {
				excitationHist.push_back(0.0);
			}
		}

		//apply AR coefficients to history of output values
		for (int i = 0; i < coeffNum; i++) {
			output += outputHist.at(i) * (-filtCoeff_buf1[i]);
		}
		//if applicable, also apply MA coefficients to history of excitation values
		if (isARMA){
			output += excitation*filtGain_buf1;
			for (int i = 0; i < MAcoeffNum; i++) {
				output += excitationHist.at(i) * (filtMACoeff_buf1[i])*filtGain_buf1;
			}

		}
		else{
			output += excitation;
		}

		//if the size of output history is greater than the number of AR coefficients, make the extra values zero so we're not storing junk
		if (outputHist.size()>(unsigned int) coeffNum) {
			for (unsigned int kk = coeffNum; kk < outputHist.size(); kk++)
				outputHist.at(kk) = 0.0;
		}
		//if the size of excitation history is greater than the number of MA coefficients, make the extra values zero so we're not storing junk
		if (excitationHist.size()>(unsigned int) MAcoeffNum) {
			for (unsigned int kk = MAcoeffNum; kk < excitationHist.size(); kk++)
				excitationHist.at(kk) = 0.0;
		}

	}
	else {//if buffer 2
		//generate Gaussian random number with power equal to interpolation model variance
		boost::normal_distribution<> nd(rgen_mean, sqrt(filtVariance_buf2));
		boost::variate_generator<boost::mt19937&,
			boost::normal_distribution<> > var_nor(rng, nd);
		excitation = var_nor();
		output = 0.0;

		//if the size of output history is less than the number of AR coefficients, append zeros
		if (outputHist.size()<(unsigned int)MAX_COEFF) {
			int subt = MAX_COEFF - outputHist.size();
			for (int j = 0; j < subt; j++) {
				outputHist.push_back(0.0);
			}
		}
		//if the size of excitation history is less than the number of MA coefficients, append zeros
		if (excitationHist.size()<(unsigned int)MAX_MACOEFF) {
			int subt = MAX_MACOEFF - excitationHist.size();
			for (int j = 0; j < subt; j++) {
				excitationHist.push_back(0.0);
			}
		}

		//apply AR coefficients to history of output values
		for (int i = 0; i < coeffNum; i++) {
			output += outputHist.at(i) * (-filtCoeff_buf2[i]);
		}
		//if applicable, also apply MA coefficients to history of excitation values
		if (isARMA){
			output += excitation*filtGain_buf2;
			for (int i = 0; i < MAcoeffNum; i++) {
				output += excitationHist.at(i) * (filtMACoeff_buf2[i])*filtGain_buf2;
			}

		}
		else{
			output += excitation;
		}

		//if the size of output history is greater than the number of AR coefficients, make the extra values zero so we're not storing junk
		if (outputHist.size()>(unsigned int) coeffNum) {
			for (unsigned int kk = coeffNum; kk < outputHist.size(); kk++) {
				outputHist.at(kk) = 0.0;
			}
		}
		//if the size of excitation history is greater than the number of MA coefficients, make the extra values zero so we're not storing junk
		if (excitationHist.size()>(unsigned int) MAcoeffNum) {
			for (unsigned int kk = MAcoeffNum; kk < excitationHist.size(); kk++)
				excitationHist.at(kk) = 0.0;
		}
	}

	// remove the last element of our output vector
	outputHist.pop_back();
	excitationHist.pop_back();
	// push our new ouput value onto the front of our vector stack
	outputHist.insert(outputHist.begin(), output);
	excitationHist.insert(excitationHist.begin(), excitation);

	return output; //this is the output vibration value (in m/s^2)
}

HDCallbackCode HDCALLBACK ContactCB(void *data)
{
	static GLuint buttonHoldCount = 0;

	double output;

	float slopeFric = 0.004;
	float vThresh; // threshold velocity for friction calculation
	vThresh = mu_k / slopeFric;

	static const hduVector3Dd direction(0, 1, 0); //direction of the vibration - now y-axis
	static HDdouble timer = 0; // timer, as defined by OH

	HDdouble instRate; //instantaneous rate of update of device

	hduVector3Dd forceTex; //the calculated texture force
	hduVector3Dd forceFric; //the friction force
	double forceNorm; //magnitude of the normal force
	hduVector3Dd currPosition; //Current position vector
	hduVector3Dd currVelocity; //current velocity vector in mm/s
	hduVector3Dd normalVelocity; //normal vector of the velocity
	hduVector3Dd normalVec; //vector normal to the sphere
	hduVector3Dd tanVelocity;//tangential velocity vector

	hduVector3Dd tanVec; //vector tangent to direction of travel
	hduVector3Dd textureVec; //direction to display texture force (binormal to normalVec and tanVec)

	//ContactModel* pContactModelHS = static_cast<ContactModel*>(data);
	Wrapper* wrapper = static_cast<Wrapper*>(data);
	ContactModel* pContactModelHS = static_cast<ContactModel*>(wrapper->gContactModelHS);
	CoverGlWidget* coverGlWidget = wrapper->coverGlWidget;

	HHD hHD = hdGetCurrentDevice();

	hdBeginFrame(hHD); //haptic frame begins here <~~~~~~~~~~~~

	int currentButtons, lastButtons;

	hduVector3Dd forceVec;

	hdGetIntegerv(HD_CURRENT_BUTTONS, &currentButtons);
	hdGetIntegerv(HD_LAST_BUTTONS, &lastButtons);

	if ((currentButtons & HD_DEVICE_BUTTON_2) != 0)
	{
		buttonHoldCount++;

		if (buttonHoldCount > HOLD_ITERATION && !selection_lock)
		{
			selection_mode = !selection_mode;
			selection_lock = true;
			text_mode = true;
			coverGlWidget->updateSelectionStatus();
		}
	}

	if (selection_mode && buttonHoldCount <= HOLD_ITERATION)
	{
		if ((currentButtons & HD_DEVICE_BUTTON_1) == 0 && (lastButtons & HD_DEVICE_BUTTON_1) != 0)
		{
			coverGlWidget->selectionUp();
		}

		if ((currentButtons & HD_DEVICE_BUTTON_2) == 0 && (lastButtons & HD_DEVICE_BUTTON_2) != 0)
		{
			coverGlWidget->selectionDown();
		}
	}

	if ((currentButtons & HD_DEVICE_BUTTON_2) == 0 && (lastButtons & HD_DEVICE_BUTTON_2) != 0)
	{
		buttonHoldCount = 0;
		selection_lock = false;
	}

	if ((currentButtons & HD_DEVICE_BUTTON_1) != 0 && (lastButtons & HD_DEVICE_BUTTON_1) == 0 && !selection_mode)
	{
		text_mode = !text_mode;
	}

	if (text_mode)
	{
		hdGetDoublev(HD_INSTANTANEOUS_UPDATE_RATE, &instRate);
		hdGetDoublev(HD_CURRENT_POSITION, currPosition);

		timer += 1.0 / instRate;

		// Cache the time of the previous haptic function call.
		lastTime = thisTime;

		// Find out what time it is now.  This information facilitates accurate velocity calculation.
		QueryPerformanceCounter(&thisTime);

		// Calculate time since last call in clock cycles and then convert to seconds.
		deltaTime.QuadPart = (thisTime.QuadPart - lastTime.QuadPart);
		deltaTimeS = (float)deltaTime.LowPart / (float)ticksPerSecond.QuadPart;

		//Second order lowpass filter parameters
		float lambda = 125.0; //cutoff frequency of lowpass filter in rad/s
		float T = deltaTimeS; //current sampling time
		float w0 = lambda*lambda*T*T / ((1.0 + T*lambda)*(1.0 + T*lambda)); //weight for z^0 term
		float w1 = 2.0 / (1.0 + lambda*T); //weight for z^-1 term
		float w2 = -1.0 / ((1.0 + T*lambda)*(1.0 + T*lambda)); //weight for z^-2 term

		currVelocity = w0*((currPosition - priorPosition) / T) + w1*priorVelocity + w2*priorpriorVelocity; //second order low-pass filter at 20 Hz

		priorpriorVelocity = priorVelocity; //store new values for filter in next loop
		priorVelocity = currVelocity;

		hduVector3Dd newEffectorPosition;
		hdGetDoublev(HD_CURRENT_POSITION, newEffectorPosition);
		//std::cout << newEffectorPosition[0] << "     " << newEffectorPosition[1] << "      " << newEffectorPosition[2] << std::endl;
		// Given the new master position, run the dynamics simulation and
		// update the slave position.
		pContactModelHS->UpdateEffectorPosition(newEffectorPosition);

		normalVec = pContactModelHS->gradient; //unit vector normal to surface

		forceVec = pContactModelHS->GetCurrentForceOnVisitor(); //get current force being applied to user
		if (pContactModelHS->isCollided)
		{
			normalVelocity = normalVec.dotProduct(currVelocity) * normalVec; // velocity in normal direction
			forceNorm = normalVec.dotProduct(forceVec); // normal force being applied
			tanVelocity = currVelocity - normalVelocity; // velocity in tangential direction
			myMatrix.HashAndInterp2(textNum, tanVelocity.magnitude(), forceNorm); // interpolate between models using the current normal force and tangential velocity
			tanVec = tanVelocity / tanVelocity.magnitude();
			textureVec = tanVec.crossProduct(normalVec);
			output = vibrations();
			forceTex = textureVec * output * 0.05;

			if (tanVelocity.magnitude() < vThresh)
			{
				forceFric = -mu_k * forceNorm * slopeFric * tanVelocity;
			}
			else
			{
				forceFric = -mu_k * forceNorm * tanVec;
			}

			forceVec = forceNorm * normalVec;

			if (visitorPos[0] < 0 && visitorPos[2] < 0)
				forceVec = forceNorm * normalVec + forceTex + forceFric;

			if (visitorPos[0] > 0 && visitorPos[2] > 0)
				forceVec = forceNorm * normalVec + forceTex + forceFric;
		}

		priorPosition = currPosition; //store new values for filter in next loop
	}

	else
	{
		forceVec.set(0.0, -(weight * ADJUST_FACTOR - ADJUST_OFFSET) / 1000.0 * GRAVITY, 0.0);
	}
	hdSetDoublev(HD_CURRENT_FORCE, forceVec);

	hdEndFrame(hHD); //haptic frame ends here <~~~~~~~~~~~~~~~

	HDErrorInfo error;
	if (HD_DEVICE_ERROR(error = hdGetError()))
	{
		hduPrintError(stderr, &error, "Error during scheduler callback");

		if (hduIsSchedulerError(&error))
		{
			return HD_CALLBACK_DONE;
		}
	}
	return HD_CALLBACK_CONTINUE;
}

CoverGlWidget::CoverGlWidget(QWidget *parent)
{
	setMinimumSize(800, 800);
	setMaximumSize(800, 800);

	setFocusPolicy(Qt::FocusPolicy::StrongFocus);
	setFocus();

	myTimer = new QTimer();
	connect(myTimer, SIGNAL(timeout()), this, SLOT(update()));
	myTimer->start(30);
}

CoverGlWidget::~CoverGlWidget()
{
	glUseProgram(0);
	glDeleteProgram(programID);
	glDeleteProgram(sphereProgramID);
	delete myTimer;

	hdStopScheduler();

	if (ghHD != HD_INVALID_HANDLE)
	{
		hdDisableDevice(ghHD);
		ghHD = HD_INVALID_HANDLE;
	}
}

void CoverGlWidget::updateSelectionStatus()
{
	if (selection_mode)
	{
		emit selectionModeChanged(true);
		//qDebug() << "Selection mode on";
	}
	else
	{
		emit selectionModeChanged(false);
		//qDebug() << "Selection mode off";
	}
}

void CoverGlWidget::selectionUp()
{
	emit selectionButtonChanged(true);
}

void CoverGlWidget::selectionDown()
{
	emit selectionButtonChanged(false);
}

void CoverGlWidget::initializeGL()
{
	QueryPerformanceFrequency(&ticksPerSecond);
	QueryPerformanceCounter(&thisTime);
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	sendDataToOpenGL();
	loadTexture(frontCoverPath.c_str(), backCoverPath.c_str(), weightText.c_str());
	installShaders();
	initHD();
}

void CoverGlWidget::paintGL()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	// Common matrices (projection and scaling)
	mat4 scalingMatrixCube = glm::scale(vec3(SCALE_IPHONE_WIDTH(), SCALE_IPHONE_HEIGHT(), SCALE_IPHONE_DEPTH()));

	/* ------------------------ front of the phone ------------------------ */
	// Use the shader for cube rendering with texture
	glUseProgram(programID);

	// Individual matrices (translation matrix)
	mat4 translationMatrixLeft = glm::translate(vec3(-SCALE_IPHONE_WIDTH() - 10.0f, 0.0f, 0.0f));

	// Send full matrix to vertex shader
	fullTransformMatrixLeft = projectionMatrix * viewMatrix * translationMatrixLeft * scalingMatrixCube;
	GLint fullTransformMatrixUniformLocation = glGetUniformLocation(programID, "fullTransformMatrix");
	glUniformMatrix4fv(fullTransformMatrixUniformLocation, 1, GL_FALSE, &fullTransformMatrixLeft[0][0]);

	// Load texture for the front of the phone
	attachTexture(img_front);

	// Set up array buffer pointer
	glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Set up UV buffer pointer
	glBindBuffer(GL_ARRAY_BUFFER, cubeUVBuffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Set up index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndexBuffer);

	// Drawing the front of the phone
	glDrawElements(GL_QUADS, cubeNumIndices, GL_UNSIGNED_SHORT, (void*)0);

	/* ------------------------ back of the phone ------------------------*/
	// Individual matrices (translation matrix)
	mat4 translationMatrixRight = glm::translate(vec3(SCALE_IPHONE_WIDTH() + 10.0f, 0.0f, 0.0f));

	// Send full matrix to the vertex shader
	fullTransformMatrixRight = projectionMatrix * viewMatrix * translationMatrixRight * scalingMatrixCube;
	glUniformMatrix4fv(fullTransformMatrixUniformLocation, 1, GL_FALSE, &fullTransformMatrixRight[0][0]);

	// Load texture for back of the phone
	attachTexture(img_back);

	// Draw the back of the phone (vertices, uv and indices have not changed in this case. No need for resetting the buffers)
	glDrawElements(GL_QUADS, cubeNumIndices, GL_UNSIGNED_SHORT, (void*)0);

	/* ------------------------ text notification ------------------------ */
	if (!text_mode) {
		mat4 translationMatrixText = glm::translate(vec3(0.0, 0.0, 150));

		mat4 scalingMatrixText = glm::scale(vec3(160, 160, 1)); // look at orthogonal definition in updateWorkspace

		mat4 fullTransformationMatrixText = projectionMatrix * viewMatrix * translationMatrixText * scalingMatrixText;
		glUniformMatrix4fv(fullTransformMatrixUniformLocation, 1, GL_FALSE, &fullTransformationMatrixText[0][0]);

		attachTexture(img_weight);

		glDrawElements(GL_QUADS, cubeNumIndices, GL_UNSIGNED_SHORT, (void*)0);
	}

	// Disable the vertex attribute array location because we are using a different shader for the visitor sphere later
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	/* ------------------------ visitor sphere ------------------------*/
	// Haptics settings first
	Synchronizer synchronizer;
	synchronizer.pContactModelHS = gContactModelHS;
	hdScheduleSynchronous(GetStateCB, &synchronizer, HD_DEFAULT_SCHEDULER_PRIORITY);

	visitorPos = synchronizer.visitorPosition;

	/*for (int i = 0; i < 3; i++)
	{
	qDebug() << visitorPos[0] << " " << visitorPos[1] << " " << visitorPos[2];
	}*/

	// Use the shader for sphere rendering without texture
	glUseProgram(sphereProgramID);

	// Individual matrices (translation and scale matrix)
	mat4 scalingMatrixSphere = glm::scale(vec3(VISITOR_SPHERE_RADIUS, VISITOR_SPHERE_RADIUS, VISITOR_SPHERE_RADIUS));

	mat4 transformMat = glm::translate(vec3(synchronizer.visitorPosition[0],
		synchronizer.visitorPosition[1],
		synchronizer.visitorPosition[2]));

	// Send full matrix to the vertex shader
	mat4 fullTransformMatrixSphere = projectionMatrix * viewMatrix * transformMat * scalingMatrixSphere;

	fullTransformMatrixUniformLocation = glGetUniformLocation(sphereProgramID, "fullMatrix");
	glUniformMatrix4fv(fullTransformMatrixUniformLocation, 1, GL_FALSE, &fullTransformMatrixSphere[0][0]);

	// Set up vertices buffer pointer
	glBindBuffer(GL_ARRAY_BUFFER, sphereVertexBuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Set up indices buffer pointer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIndexBuffer);

	// Draw the visitor sphere
	glDrawElements(GL_TRIANGLES, sphereNumIndices, GL_UNSIGNED_SHORT, (void*)0);

	// Disable the vertex attribute array location
	glDisableVertexAttribArray(0);
}

void CoverGlWidget::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
}

void CoverGlWidget::sendDataToOpenGL()
{
	ShapeData cube = ShapeGenerator::makeCube();
	ShapeData sphere = ShapeGenerator::makeSphere(40);

	// Generate and bind vertex array object
	GLuint vertexArrayObject;
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	/* ------------------------ make cube buffers ------------------------ */
	// Generate, bind and send data to cube vertices buffer
	glGenBuffers(1, &cubeVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, cube.vertexBufferSize(), cube.vertices, GL_STATIC_DRAW);

	// Generate, bind, and send data to uv buffer
	glGenBuffers(1, &cubeUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cubeUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, cube.uvBufferSize(), cube.uvs, GL_STATIC_DRAW);

	// Generate, bind and send data to cube indices buffer
	glGenBuffers(1, &cubeIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube.indexBufferSize(), cube.indices, GL_STATIC_DRAW);

	// Save the number of indices for later drawing
	cubeNumIndices = cube.numIndices;
	// Clean up the cube ShapeData
	cube.cleanup();

	/* ------------------------ make visitor sphere buffers ------------------------ */
	// Generate, bind and send data to sphere vertices buffer
	glGenBuffers(1, &sphereVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sphere.vertexBufferSize(), sphere.vertices, GL_DYNAMIC_DRAW);

	// Generate, bind and send data to sphere indices buffer (since the sphere doesn't have texture, there's no uv buffer for it)
	glGenBuffers(1, &sphereIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.indexBufferSize(), sphere.indices, GL_DYNAMIC_DRAW);

	// Save the number of indices for later drawing
	sphereNumIndices = sphere.numIndices;
	// Clean up the sphere ShapeData
	sphere.cleanup();
}

std::string CoverGlWidget::readShaderCode(const char* fileName)
{
	std::ifstream meInput(fileName);
	if (!meInput.good())
	{
		std::cout << "File failed to load ... " << fileName;
		exit(1);
	}
	return std::string(
		std::istreambuf_iterator<char>(meInput),
		std::istreambuf_iterator<char>());
}

void CoverGlWidget::installShaders()
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	const char* adapter[1];
	std::string tempVertex = readShaderCode("VertexShaderCode.glsl");
	adapter[0] = tempVertex.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	std::string tempFragment = readShaderCode("FragmentShaderCode.glsl");
	adapter[0] = tempFragment.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) ||
		!checkShaderStatus(fragmentShaderID))
	{
		return;
	}

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	if (!checkProgramStatus(programID))
	{
		return;
	}

	glDetachShader(programID, vertexShaderID);
	glDetachShader(programID, fragmentShaderID);
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	tempVertex = readShaderCode("SphereVertexShader.glsl");
	adapter[0] = tempVertex.c_str();
	glShaderSource(vertexShaderID, 1, adapter, 0);
	tempFragment = readShaderCode("SphereFragmentShader.glsl");
	adapter[0] = tempFragment.c_str();
	glShaderSource(fragmentShaderID, 1, adapter, 0);

	glCompileShader(vertexShaderID);
	glCompileShader(fragmentShaderID);

	if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
		return;

	sphereProgramID = glCreateProgram();
	glAttachShader(sphereProgramID, vertexShaderID);
	glAttachShader(sphereProgramID, fragmentShaderID);

	glLinkProgram(sphereProgramID);

	if (!checkProgramStatus(sphereProgramID))
		return;

	glDetachShader(sphereProgramID, vertexShaderID);
	glDetachShader(sphereProgramID, fragmentShaderID);
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);
}

bool CoverGlWidget::checkStatus(GLuint objectID,
	PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
	PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
	GLenum statusType)
{
	GLint status;
	objectPropertyGetterFunc(objectID, statusType,
		&status);
	if (status != GL_TRUE)
	{
		GLint infoLogLength;
		objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH,
			&infoLogLength);
		GLchar* buffer = new GLchar[infoLogLength];

		GLsizei bufferSize;
		getInfoLogFunc(objectID, infoLogLength,
			&bufferSize, buffer);

		std::cout << buffer << std::endl;

		delete[] buffer;
		return false;
	}

	return true;
}

bool CoverGlWidget::checkShaderStatus(GLuint shaderID)
{
	return checkStatus(shaderID, glGetShaderiv,
		glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool CoverGlWidget::checkProgramStatus(GLuint programID)
{
	return checkStatus(programID, glGetProgramiv,
		glGetProgramInfoLog, GL_LINK_STATUS);
}

void CoverGlWidget::initHD()
{
	HDErrorInfo error;
	ghHD = hdInitDevice(HD_DEFAULT_DEVICE);
	if (HD_DEVICE_ERROR(error = hdGetError()))
	{
		hduPrintError(stderr, &error, "Failed to initialize haptic device");
		exit(-1);
	}

	qDebug() << "Found device " << hdGetString(HD_DEVICE_MODEL_TYPE);

	hdEnable(HD_FORCE_OUTPUT);
	hdEnable(HD_MAX_FORCE_CLAMPING);

	hdStartScheduler();

	if (HD_DEVICE_ERROR(error = hdGetError()))
	{
		hduPrintError(stderr, &error, "Failed to start the scheduler");
		exit(-1);
	}

	// Load haptic texture file
	AccSynthHashMatrix theMatrix = generateHashMatrix();
	myMatrix = theMatrix;
	qDebug() << "Texture matrix created successfully";

	HDdouble maxWorkspace[6];
	hdGetDoublev(HD_USABLE_WORKSPACE_DIMENSIONS, maxWorkspace);

	// Low, Left, Back point of device workspace
	hduVector3Dd LLB(maxWorkspace[0], maxWorkspace[1], maxWorkspace[2]);
	// Top, Right, Front point of device workspace
	hduVector3Dd TRF(maxWorkspace[3], maxWorkspace[4], maxWorkspace[5]);

	updateWorkspace(LLB, TRF);

	DefineForceField();
}

void CoverGlWidget::updateWorkspace(hduVector3Dd LLB, hduVector3Dd TRF)
{
	HDdouble centerScreen[3];
	centerScreen[0] = (TRF[0] + LLB[0]) / 2.0;
	centerScreen[1] = (TRF[1] + LLB[1]) / 2.0;
	centerScreen[2] = (TRF[2] + LLB[2]) / 2.0;

	HDdouble screenDims[3];
	screenDims[0] = TRF[0] - LLB[0];
	screenDims[1] = TRF[1] - LLB[1];
	screenDims[2] = TRF[2] - LLB[2];

	HDdouble maxDimXY = (screenDims[0] > screenDims[1] ? screenDims[0] : screenDims[1]);
	HDdouble maxDim = (maxDimXY > screenDims[2] ? maxDimXY : screenDims[2]);
	//maxDim /= 4.0;

	projectionMatrix = glm::ortho(centerScreen[0] - maxDim,
		centerScreen[0] + maxDim,
		centerScreen[1] - maxDim,
		centerScreen[1] + maxDim,
		50.0f - maxDim,
		50.0f + maxDim);

	viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 50.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
}

void CoverGlWidget::DefineForceField()
{
	DeviceDisplayState initialMasterState;
	hdScheduleSynchronous(DeviceStateCallback, &initialMasterState,
		HD_MAX_SCHEDULER_PRIORITY);

	hduVector3Dd visitorLocation(2.0f * VISITOR_SPHERE_RADIUS + SCALE_IPHONE_WIDTH(),
		2 * VISITOR_SPHERE_RADIUS + SCALE_IPHONE_HEIGHT(), 15.0);

	gContactModelHS = new ContactModel(visitorLocation, SCALE_IPHONE_WIDTH(),
		SCALE_IPHONE_HEIGHT(), SCALE_IPHONE_DEPTH());

	Wrapper* wrapper = new Wrapper();
	wrapper->coverGlWidget = this;
	wrapper->gContactModelHS = gContactModelHS;

	gSchedulerCallback = hdScheduleAsynchronous(ContactCB, wrapper,
		HD_DEFAULT_SCHEDULER_PRIORITY);
}

void CoverGlWidget::loadTexture(const char *imgPathFront, const char *imgPathBack, const char *imgPathWeight)
{
	img_front = cvLoadImage(imgPathFront, 1);
	img_back = cvLoadImage(imgPathBack, 1);
	img_weight = cvLoadImage(imgPathWeight, 1);

	if (img_front == NULL || img_back == NULL)
	{
		cout << "The image does not exist.\n";
		cout << "\nPress any key to quit.\n";
		getchar();
		exit(-1);
	}
}

void CoverGlWidget::attachTexture(IplImage *currentImg)
{
	GLuint texture;
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, currentImg->width, currentImg->height, 0, GL_BGR_EXT,
		GL_UNSIGNED_BYTE, currentImg->imageData);
}

void CoverGlWidget::coverChanged(int currentIndex)
{
	frontCoverPath = coverPathFront[currentIndex];
	backCoverPath = coverPathBack[currentIndex];
	weight = weightArray[currentIndex];
	weightText = weightTextPath[currentIndex];
	loadTexture(frontCoverPath.c_str(), backCoverPath.c_str(), weightText.c_str());
	textNum = textureArray[currentIndex];
}


