#pragma once

struct Status
{
	UINT m_health;
	UINT m_attack;
	UINT m_moveSpeed;

	Status() : m_health(0), m_attack(0), m_moveSpeed(0) { }
	Status(UINT health, UINT attack, UINT speed) :
		m_health(health), m_attack(attack), m_moveSpeed(speed) { }
};

class StatusLoader
{
private:
	vector<Status> m_status;
public:
	StatusLoader(string fileName);
	~StatusLoader();
};