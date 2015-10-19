#include "tubesjarkom.h"
int main() {
	struct FRAME {
		Byte SF0;
		int SF1;
		Byte SF2;
		char SF3;
		Byte SF4;
		int SF5;
	};
	FRAME Frame1 {SOH, 1, STX, 'H', ETX, int('H') };
	cout << Frame1.SF5 << endl;
}
