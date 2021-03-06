#include "ContactModel.h"
#include <QtCore\qdebug.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtx\transform.hpp>

using glm::mat4;
using glm::vec4;

extern mat4 fullTransformMatrixLeft;
extern mat4 fullTransformMatrixRight;

ContactModel::ContactModel(hduVector3Dd visitor, double width, double height, double depth)
{
	m_width = width;
	m_height = height;
	m_depth = depth;
	UpdateEffectorPosition(visitor);
}

ContactModel::~ContactModel()
{
}

void ContactModel::UpdateEffectorPosition(const hduVector3Dd visitor)
{
	double delta = EPSILON; // Used for deriving the limit to 0
	double func = ShapeFunction(visitor[0], visitor[1], visitor[2]);

	m_effectorPosition = visitor;
	hduVector3Dd gradientTemp;

	if (func >= -EPSILON)
	{
		isCollided = true;
		gradientTemp[0] = (ShapeFunction((visitor[0] + delta), visitor[1], visitor[2]) - func) / delta;
		gradientTemp[1] = (ShapeFunction(visitor[0], (visitor[1] + delta), visitor[2]) - func) / delta;
		gradientTemp[2] = (ShapeFunction(visitor[0], visitor[1], (visitor[2] + delta)) - func) / delta;

		// Normalize gradient
		hduVecNormalizeInPlace(gradientTemp);

		hduVector3Dd temp1 = m_effectorPosition - m_contactPosition;
		HDdouble aa = temp1.dotProduct(gradientTemp);
		hduVector3Dd temp2 = m_effectorPosition - m_visitorPosition;
		HDdouble bb = temp2.dotProduct(gradientTemp);

		if ((ShapeFunction(m_contactPosition[0] + gradientTemp[0], m_contactPosition[1] + gradientTemp[1], m_contactPosition[2] + gradientTemp[2]) > 0)
			|| (aa > 0) || (bb > 0))
		{
			gradientTemp[0] = -gradientTemp[0];
			gradientTemp[1] = -gradientTemp[1];
			gradientTemp[2] = -gradientTemp[2];
		}

		HDdouble cc = gradientTemp.dotProduct(gradient);
		HDdouble dd = gradientTemp.magnitude() * gradient.magnitude();
		dd = cc / dd;
		if (dd < 0.8)
		{
			gradient = gradient + gradientTemp;
		}
		else
		{
			gradient = gradientTemp;
		}
		hduVecNormalizeInPlace(gradient);

		m_visitorPosition = m_effectorPosition;

		/* Calculate contact point on the surface using subdivision method */
		HDdouble distance = temp2.magnitude();
		hduVector3Dd increment = gradient * distance;

		// m_end is the first point out of object in the direction of gradient
		hduVector3Dd m_end = m_visitorPosition + increment;
		hduVector3Dd m_new;
		if (ShapeFunction(m_end[0], m_end[1], m_end[2]) < 0)
		{
			m_new = m_end;

			for (int i = 0; i < 18; i++)
			{
				increment = m_end - m_visitorPosition;
				m_new = m_visitorPosition + increment / 2;
				if (ShapeFunction(m_new[0], m_new[1], m_new[2]) < 0)
					m_end = m_new;
				else
					m_visitorPosition = m_new;
			}
		}
		else
		{
			m_end = m_visitorPosition + increment;
			m_new = m_end;
		}
		m_visitorPosition = m_new;

		/* Calculate force on visitor */
		increment = m_visitorPosition - m_effectorPosition;
		distance = increment.magnitude();
		m_forceOnVisitor = STIFFNESS * distance * gradient;
	}
	else
	{
		isCollided = false;
		m_contactPosition = m_effectorPosition;
		m_forceOnVisitor.set(0.0f, 0.0f, 0.0f);
		m_visitorPosition = m_effectorPosition;
		gradient.set(0.0f, 0.0f, 0.0f);
	}
}

hduVector3Dd ContactModel::GetCurrentForceOnVisitor(void)
{
	return m_forceOnVisitor;
}

hduVector3Dd ContactModel::GetCurrentContactPoint(void)
{
	return m_visitorPosition;
}

double ContactModel::InterFunction(double xx, double yy)
{
	double fun = xx + yy - sqrt(xx*xx + yy*yy);
	return fun;
}

double ContactModel::UnionFunction(double xx, double yy)
{
	double fun = xx + yy + sqrt(xx*xx + yy*yy);
	return fun;
}

double ContactModel::ShapeFunction(double x, double y, double z)
{
	double leftX, rightX, upY, downY, frontZ, backZ;
	double funTempLeft, funTempRight;
	double funTempCut, funTempCy, funTempCamCy1, funTempCamCy2, funTempCamCube;

	// The left cube
	/*leftX = x + 107;
	rightX = -10.87 - x;
	funTempLeft = InterFunction(leftX, rightX);
	upY = 100 - y;
	funTempLeft = InterFunction(funTempLeft, upY);
	downY = y + 100;
	funTempLeft = InterFunction(funTempLeft, downY);
	frontZ = -z + 15;
	funTempLeft = InterFunction(funTempLeft, frontZ);
	backZ = z + 15;
	funTempLeft = InterFunction(funTempLeft, backZ);*/

	funTempLeft = 1 - pow(((x+58.935) / 48.065), 12) - pow((y / 100), 12) - pow((z / 15), 6);
	

	// The lowered microphone part
	leftX = x + 69;
	rightX = -49 - x;
	funTempCut = InterFunction(leftX, rightX);
	upY = 87 - y;
	funTempCut = InterFunction(funTempCut, upY);
	downY = y - 86;
	funTempCut = InterFunction(funTempCut, downY);
	frontZ = -z + 15;
	funTempCut = InterFunction(funTempCut, frontZ);
	backZ = z - 14;
	funTempCut = InterFunction(funTempCut, backZ);
	funTempLeft = InterFunction(funTempLeft, -funTempCut);

	// The lowered main screen button
	funTempCy = 6 - sqrt((x + 59)*(x + 59) + (y + 88)*(y + 88));
	frontZ = 15 - z;
	backZ = z - 13;
	funTempCy = InterFunction(funTempCy, frontZ);
	funTempCy = InterFunction(funTempCy, backZ);
	funTempLeft = InterFunction(funTempLeft, -funTempCy);
	
	// The right cube
	/*leftX = x - 11.375;
	rightX = 107.433 - x;
	funTempRight = InterFunction(leftX, rightX);
	upY = 100 - y;
	funTempRight = InterFunction(funTempRight, upY);
	downY = y + 100;
	funTempRight = InterFunction(funTempRight, downY);
	frontZ = -z + 15;
	funTempRight = InterFunction(funTempRight, frontZ);
	backZ = z + 15;
	funTempRight = InterFunction(funTempRight, backZ);*/

	funTempRight = 1 - pow(((x - 59.404) / 48.029), 12) - pow((y / 100), 12) - pow((z / 15), 6);
	
	// The raised camera
	/*funTempCamCy1 = 4 - sqrt((x - 23.6)*(x - 23.6) + (y - 87)*(y - 87));
	frontZ = -z + 20;
	backZ = z - 15;
	funTempCamCy1 = InterFunction(funTempCamCy1, frontZ);
	funTempCamCy1 = InterFunction(funTempCamCy1, backZ);

	funTempRight = UnionFunction(funTempRight, funTempCamCy1);

	funTempCamCy2 = 4 - sqrt((x + 38)*(x + 38) + (y + 87)*(y + 87));
	funTempCamCy2 = InterFunction(funTempCamCy2, frontZ);
	funTempCamCy2 = InterFunction(funTempCamCy2, backZ);

	funTempRight = UnionFunction(funTempRight, funTempCamCy2);

	leftX = x - 23.6;
	rightX = 38 - x;
	funTempCamCube = InterFunction(leftX, rightX);
	upY = 91 - y;
	downY = y - 83;
	funTempCamCube = InterFunction(funTempCamCube, upY);
	funTempCamCube = InterFunction(funTempCamCube, downY);
	funTempCamCube = InterFunction(funTempCamCube, frontZ);
	funTempCamCube = InterFunction(funTempCamCube, backZ);

	funTempRight = UnionFunction(funTempRight, funTempCamCube);*/

	return UnionFunction(funTempLeft, funTempRight);
}