#pragma once
#include "effect.h"

class effectshadow : public effect
{
	BYTE blur; // �ڂ���
	short angle; // �p�x
	BYTE distance; // ����
	BYTE power; // ���x
	BYTE alpha; // �s�����x
	Gdiplus::Color color; // �F
	BYTE inner; // ����

public:
	effectshadow();
	~effectshadow();

	virtual BOOL isshadow() const override { return TRUE; }
	virtual effect* copy() const override;

	int getblur()const { return blur; }
	void setblur(int n){ blur = n; }

	int getangle()const { return angle; }
	void setangle(int n){ angle = n; }

	int getdistance()const { return distance; }
	void setdistance(int n){ distance = n; }

	int getpower()const { return power; }
	void setpower(int n){ power = n; }

	int getalpha()const { return alpha; }
	void setalpha(int n) { alpha = n; }

	Gdiplus::Color getcolor()const { return color; }
	void setcolor(Gdiplus::Color n){ color = n; }

	BOOL getinner()const { return inner; }
	void setinner(BOOL n){ inner = n; }

	virtual void getliststring(LPTSTR, int) const override;

	virtual void draw(Gdiplus::Graphics* g, Gdiplus::GraphicsPath* path, float width, float height) const override;

	virtual void save(HANDLE hFile) const override;
	virtual void open(HANDLE hFile) override;

	virtual KIND drawkind() const override { return inner ? After : Before; }; // �`��

};

