#pragma once
#include <stdbool.h>

bool wifi_begin();       // connect using secrets.h; blocks up to the configured timeout
bool wifi_connected();
void wifi_ensure();      // kick a reconnect if the link dropped
