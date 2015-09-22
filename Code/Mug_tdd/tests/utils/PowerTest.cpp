extern "C"
{
#include "utils/watchdog.h"
}

#include "CppUTest/TestHarness.h"

TEST_GROUP(Power)
{
	void setup()
	{

	}
};

TEST(Power, WDT_Int_Enabled_After_PreInitHardware)
{
	PreInitHardware();	
	CHECK(Watchdog_TimerEnabled());
}

/*
TEST(Power, TurnOn)
{

}
*/

/*
TEST(Power, WDT_Int_Disabled_After_PostInitHardware)
{

}
*/
