// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_COMMON_FILE_SYSTEM_FILE_SYSTEM_DISPATCHER_H_
#define CHROME_COMMON_FILE_SYSTEM_FILE_SYSTEM_DISPATCHER_H_

#include <vector>

#include "base/basictypes.h"
#include "base/file_util_proxy.h"
#include "base/id_map.h"
#include "base/nullable_string16.h"
#include "googleurl/src/gurl.h"
#include "ipc/ipc_channel.h"
#include "ipc/ipc_message.h"
#include "webkit/fileapi/file_system_callback_dispatcher.h"

namespace base {
struct PlatformFileInfo;
}

class FilePath;

// Dispatches and sends file system related messages sent to/from a child
// process from/to the main browser process.  There is one instance
// per child process.  Messages are dispatched on the main child thread.
class FileSystemDispatcher {
 public:
  FileSystemDispatcher();
  ~FileSystemDispatcher();

  bool OnMessageReceived(const IPC::Message& msg);

  bool Move(const FilePath& src_path,
            const FilePath& dest_path,
            fileapi::FileSystemCallbackDispatcher* dispatcher);
  bool Copy(const FilePath& src_path,
            const FilePath& dest_path,
            fileapi::FileSystemCallbackDispatcher* dispatcher);
  bool Remove(const FilePath& path,
              fileapi::FileSystemCallbackDispatcher* dispatcher);
  bool ReadMetadata(const FilePath& path,
                    fileapi::FileSystemCallbackDispatcher* dispatcher);
  bool Create(const FilePath& path,
              bool exclusive,
              bool is_directory,
              bool recursive,
              fileapi::FileSystemCallbackDispatcher* dispatcher);
  bool Exists(const FilePath& path,
              bool for_directory,
              fileapi::FileSystemCallbackDispatcher* dispatcher);
  bool ReadDirectory(const FilePath& path,
                     fileapi::FileSystemCallbackDispatcher* dispatcher);

 private:
  void DidSucceed(int request_id);
  void DidReadMetadata(int request_id,
                       const base::PlatformFileInfo& file_info);
  void DidReadDirectory(
      int request_id,
      const std::vector<base::file_util_proxy::Entry>& entries,
      bool has_more);
  void DidFail(int request_id, base::PlatformFileError error_code);

  IDMap<fileapi::FileSystemCallbackDispatcher, IDMapOwnPointer> dispatchers_;

  DISALLOW_COPY_AND_ASSIGN(FileSystemDispatcher);
};

#endif  // CHROME_COMMON_FILE_SYSTEM_FILE_SYSTEM_DISPATCHER_H_
