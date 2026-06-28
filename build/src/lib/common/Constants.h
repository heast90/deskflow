/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2024 - 2025 Chris Rizzitello <sithlord48@gmail.com>
 * SPDX-FileCopyrightText: (C) 2025 Symless Ltd.
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

const auto kAppName = "Deskflow";
const auto kAppId = "deskflow";
const auto kAppDescription = "Keyboard and mouse sharing utility";
const auto kDaemonBinName = "deskflow-daemon";
const auto kDaemonIpcName = "deskflow-daemon";
const auto kRevFqdnName = "org.deskflow.deskflow";

const auto kCopyright = //
    "Copyright (C) 2024-2026 Deskflow Devs\n"
    "Copyright (C) 2012-2025 Symless Ltd.\n"
    "Copyright (C) 2009-2012 Nick Bolton\n"
    "Copyright (C) 2002-2009 Chris Schoeneman";

const auto kCoreBinName = "deskflow-core";

#ifdef _WIN32

// clang-format off
const auto kWindowsRuntimeMajor = ;
const auto kWindowsRuntimeMinor = ;
// clang-format on

constexpr auto kCloseEventName = L"Global\\DeskflowClose";
constexpr auto kSendSasEventName = L"Global\\DeskflowSendSAS";

#endif
