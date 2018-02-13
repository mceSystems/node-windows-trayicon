#include "TrayIcon.h"

NBIND_CLASS(CTrayIconContainer) {
	construct<>();
	method(Start);
	method(SetIconPath);
	method(SetTitle);
	method(Stop);
	method(AddMenuItem);
	method(OnMenuItem);
	method(ShowBalloon);
}