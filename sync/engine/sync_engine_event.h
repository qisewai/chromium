// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYNC_ENGINE_SYNC_ENGINE_EVENT_H_
#define SYNC_ENGINE_SYNC_ENGINE_EVENT_H_
#pragma once

#include <string>

#include "base/observer_list.h"
#include "sync/sessions/session_state.h"
#include "sync/syncable/model_type.h"

namespace syncable {
class Id;
}

namespace browser_sync {

struct SyncEngineEvent {
  enum EventCause {
    ////////////////////////////////////////////////////////////////
    // Sent on entry of Syncer state machine
    SYNC_CYCLE_BEGIN,

    // SyncerCommand generated events.
    STATUS_CHANGED,

    // We have reached the SYNCER_END state in the main sync loop.
    SYNC_CYCLE_ENDED,

    ////////////////////////////////////////////////////////////////
    // Generated in response to specific protocol actions or events.

    // New token in updated_token.
    UPDATED_TOKEN,

    // This is sent after the Syncer (and SyncerThread) have initiated self
    // halt due to no longer being permitted to communicate with the server.
    // The listener should sever the sync / browser connections and delete sync
    // data (i.e. as if the user clicked 'Stop Syncing' in the browser.
    STOP_SYNCING_PERMANENTLY,

    // These events are sent to indicate when we know the clearing of
    // server data have failed or succeeded.
    CLEAR_SERVER_DATA_SUCCEEDED,
    CLEAR_SERVER_DATA_FAILED,

    // This event is sent when we receive an actionable error. It is upto
    // the listeners to figure out the action to take using the snapshot sent.
    ACTIONABLE_ERROR,
  };

  explicit SyncEngineEvent(EventCause cause);
  ~SyncEngineEvent();

  EventCause what_happened;

  // The last session used for syncing.
  sessions::SyncSessionSnapshot snapshot;

  // Update-Client-Auth returns a new token for sync use.
  std::string updated_token;
};

class SyncEngineEventListener {
 public:
  // TODO(tim): Consider splitting this up to multiple callbacks, rather than
  // have to do Event e(type); OnSyncEngineEvent(e); at all callsites,
  virtual void OnSyncEngineEvent(const SyncEngineEvent& event) = 0;
 protected:
  virtual ~SyncEngineEventListener() {}
};

}  // namespace browser_sync

#endif  // SYNC_ENGINE_SYNC_ENGINE_EVENT_H_
