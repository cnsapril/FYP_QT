#pragma once

#include <HDU\hduVector.h>
#include "Constants.h"

class ContactModel
{
public:
	ContactModel(hduVector3Dd visitor, double width, double height, double depth);
	~ContactModel();

	/* Updates the end effector position and sets the new object position,
		and their relative displacement */
	void UpdateEffectorPosition(const hduVector3Dd visitor);

	/* Given current spatial configuration, calculate the force on visitor */
	hduVector3Dd GetCurrentForceOnVisitor(void);
	hduVector3Dd GetCurrentContactPoint(void);

	bool isCollided = false;
	hduVector3Dd gradient;

protected:
	hduVector3Dd m_effectorPosition; // The real position of the HIP
	hduVector3Dd m_visitorPosition; // The nearest HIP point on the surface
	hduVector3Dd m_contactPosition; // The position of end-effector at the first time when contact happened
	hduVector3Dd m_forceOnVisitor; // Normal force on the HIP

private:
	double InterFunction(double xx, double yy);
	double UnionFunction(double xx, double yy);
	double ShapeFunction(double x, double y, double z);
	double m_width;
	double m_height;
	double m_depth;
};

