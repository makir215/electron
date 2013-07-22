// Copyright (c) 2013 GitHub, Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/node_bindings_mac.h"

#include <errno.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>

#include "vendor/node/src/node.h"
#include "vendor/node/src/node_internals.h"

namespace atom {

NodeBindingsMac::NodeBindingsMac(bool is_browser)
    : NodeBindings(is_browser),
      kqueue_(kqueue()) {
  // Add uv's backend fd to kqueue.
  struct kevent ev;
  EV_SET(&ev, uv_backend_fd(uv_loop_), EVFILT_READ, EV_ADD | EV_ENABLE,
         0, 0, 0);
  kevent(kqueue_, &ev, 1, NULL, 0, NULL);
}

NodeBindingsMac::~NodeBindingsMac() {
}

void NodeBindingsMac::PollEvents() {
  struct timespec spec;
  int timeout = uv_backend_timeout(uv_loop_);
  if (timeout != -1) {
    spec.tv_sec = timeout / 1000;
    spec.tv_nsec = (timeout % 1000) * 1000000;
  }

  // Wait for new libuv events.
  int r;
  do {
    struct kevent ev;
    r = ::kevent(kqueue_, NULL, 0, &ev, 1,
                 timeout == -1 ? NULL : &spec);
  } while (r == -1 && errno == EINTR);
}

// static
NodeBindings* NodeBindings::CreateInBrowser() {
  return new NodeBindingsMac(true);
}

// static
NodeBindings* NodeBindings::CreateInRenderer() {
  return new NodeBindingsMac(false);
}

}  // namespace atom