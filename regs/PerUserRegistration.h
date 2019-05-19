#pragma once

class CPerUserRegistration
{
public:
	CPerUserRegistration(bool bEnable);

	~CPerUserRegistration();

private:
	void SetPerUserRegister(bool bEnable);
	void EnablePerUserTLibRegistration();

private:
	bool _mapping;
};

#define SetPerUserRegistration(bEnabel)  CPerUserRegistration _PerUserReg(bEnabel);