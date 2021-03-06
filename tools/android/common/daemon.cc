// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tools/android/common/daemon.h"

#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "base/command_line.h"
#include "base/eintr_wrapper.h"
#include "base/logging.h"

namespace {

const char kNoSpawnDaemon[] = "D";

int g_exit_status = 0;

void Exit(int unused) {
  _exit(g_exit_status);
}

}  // namespace

namespace tools {

bool HasHelpSwitch(const CommandLine& command_line) {
  return command_line.HasSwitch("h") || command_line.HasSwitch("help");
}

bool HasNoSpawnDaemonSwitch(const CommandLine& command_line) {
  return command_line.HasSwitch(kNoSpawnDaemon);
}

void ShowHelp(const char* program,
              const char* extra_title,
              const char* extra_descriptions) {
  printf("Usage: %s [-%s] %s\n"
         " -%s  stops from spawning a daemon process\n%s",
         program, kNoSpawnDaemon, extra_title, kNoSpawnDaemon,
         extra_descriptions);
}

void SpawnDaemon(int exit_status) {
  g_exit_status = exit_status;
  signal(SIGUSR1, Exit);

  if (fork()) {
    // In parent process.
    sleep(10);  // Wait for the child process to finish setsid().
    NOTREACHED();
  }

  // In child process.
  setsid();  // Detach the child process from its parent.
  kill(getppid(), SIGUSR1);  // Inform the parent process to exit.

  // Close the standard input and outputs, otherwise the process may block
  // adbd when the shell exits.
  // Comment out these lines if you want to see outputs for debugging.
  HANDLE_EINTR(close(0));
  HANDLE_EINTR(close(1));
  HANDLE_EINTR(close(2));
}

}  // namespace tools

