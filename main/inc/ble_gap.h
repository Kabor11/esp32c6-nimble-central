#pragma once

#include "common.h"

static const char* TAG = "ble_gap";
int ble_gap_event(struct ble_gap_event *event, void *arg);

int ble_should_connect(const struct ble_gap_disc_desc *disc);
void connect_if_interesting(const struct ble_gap_disc_desc *disc);