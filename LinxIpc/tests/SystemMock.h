#pragma once
#include "gmock/gmock.h"

class GpioMock {
    MOCK_METHOD(int, setMode, (int mode));
};
