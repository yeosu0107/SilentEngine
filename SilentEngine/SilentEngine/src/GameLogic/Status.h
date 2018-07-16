#pragma once

struct Status
{
	int		m_health;
	int		prev_health;
	int		m_maxhealth;

	UINT		m_mp;
	UINT		prev_mp;
	UINT		m_maxmp;

	UINT		m_attack;
	UINT		m_moveSpeed;
	

	Status() : m_health(0), m_attack(0), m_moveSpeed(0) { }
	Status(UINT health, UINT attack, UINT speed) :
		m_health(health), m_attack(attack), m_moveSpeed(speed), prev_health(health), m_maxhealth(health) { }
	Status(UINT health, UINT mp, UINT attack, UINT speed) :
		m_health(health), m_attack(attack), m_moveSpeed(speed), prev_health(health), m_mp(mp), prev_mp(mp),
		m_maxhealth(health), m_maxmp(mp) {}
};

class StatusLoader
{
private:
	vector<Status> m_status;
public:
	StatusLoader(string fileName);
	~StatusLoader();
};