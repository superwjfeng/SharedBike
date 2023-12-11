#include <gtest/gtest.h>
#include "events_def.h"

TEST(MobileCode, Request) {
  MobileCodeRspEv mcre(200, 666666);
  EXPECT_EQ(mcre.get_data(), "T_OK.");
}

TEST(MobileCode, Response) {
  MobileCodeRspEv mcre1(ERRC_BIKE_IS_TAKEN, 666666);
  EXPECT_EQ(mcre1.get_data(), "Bike is taken.");

  MobileCodeRspEv mcre2(500, 666666);
  EXPECT_EQ(mcre2.get_data(), "Undefined.");
}
