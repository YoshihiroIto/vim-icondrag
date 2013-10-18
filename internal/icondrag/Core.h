#ifndef CORE_H
#define CORE_H

#include <string>

class Core
{
public:
	enum{ WM_GETDATA = (WM_APP + 1977) };

	void Initialize(HWND hwnd);
	void Finalize();

	void SetFilepath(const char *filepath);

	bool OnGETDATA(WPARAM wParam, LPARAM lParam);
	bool OnNCRBUTTONDOWN(WPARAM wParam, LPARAM lParam);
	bool OnNCLBUTTONDOWN(WPARAM wParam, LPARAM lParam);
	bool OnMOUSEMOVE(WPARAM wParam, LPARAM lParam);
	bool OnLBUTTONUP(WPARAM wParam, LPARAM lParam);
	bool OnRBUTTONUP(WPARAM wParam, LPARAM lParam);
	bool OnTIMER(WPARAM wParam, LPARAM lParam);

private:
    bool	dragging;
    int		timerState;
    bool	isActiveSysmenuTimer;
    bool	isLeftClick;
    int		drawStartXpos;
    int		drawStartYpos;
	HWND    hwnd;

	std::string    filepath;

	bool	OnNCBUTTONDOWN_Core(WPARAM wParam, LPARAM lParam);
	bool	OnBUTTONUP_Core(WPARAM wParam, LPARAM lParam);

	void SetSysMenuTimer();
	void KillSysMenuTimer();
	void ShowSystemMenu();

	bool IsInDoubleClickRect();
};

#endif
