#pragma once

struct Status
{
	int		m_health;
	int		prev_health;
	int		m_maxhealth;

	int		m_mp;
	int		prev_mp;
	int		m_maxmp;

	int		m_attack;
	int		prev_attack;
	int		m_moveSpeed;
	

	Status() : m_health(0), m_attack(0), m_moveSpeed(0), m_mp(0) { }
	Status(UINT health, UINT attack, UINT speed) :
		m_health(health), m_attack(attack), m_moveSpeed(speed), prev_health(health), m_maxhealth(health) {
		m_mp = prev_mp = m_maxmp = 0;
		prev_attack = m_attack;
	}
	Status(UINT health, UINT mp, UINT attack, UINT speed) :
		m_health(health), m_attack(attack), prev_attack(attack), m_moveSpeed(speed), prev_health(health), 
		m_mp(mp), prev_mp(mp), m_maxhealth(health), m_maxmp(mp) {}

	void reset() {
		m_health = prev_health;
		m_maxhealth = prev_health;

		m_mp = prev_mp;
		m_maxmp = prev_mp;

		m_attack = prev_attack;
	}
};

class StatusLoader
{
private:
	vector<Status> m_status;
public:
	StatusLoader(string fileName);
	~StatusLoader();
};