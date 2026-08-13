// Copyright (c) 2026 yuumei-02. All Rights Reserved.
// See the LICENSE file for more information.

#pragma once

typedef enum {
   NT_Unknown,
   NT_Info,
   NT_Warning,
   NT_Error,
} NotificationType;

void Notification_new(NotificationType type, float seconds_duration, nullable const ustr32 title, nullable const ustr32 body);

