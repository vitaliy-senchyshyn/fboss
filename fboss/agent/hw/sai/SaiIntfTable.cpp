/*
 * Copyright (c) 2004-present, Facebook, Inc.
 * Copyright (c) 2016, Cavium, Inc.
 * All rights reserved.
 * 
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 * 
 */
#include "SaiIntf.h"
#include "SaiIntfTable.h"
#include "SaiSwitch.h"
#include "SaiError.h"

using std::shared_ptr;
using std::unique_ptr;

namespace facebook { namespace fboss {

SaiIntfTable::SaiIntfTable(const SaiSwitch *hw)
  : hw_(hw) {
  VLOG(4) << "Entering " << __FUNCTION__;
}

SaiIntfTable::~SaiIntfTable() {
  VLOG(4) << "Entering " << __FUNCTION__;
}

SaiIntf *SaiIntfTable::GetIntfIf(sai_object_id_t id) const {
  VLOG(4) << "Entering " << __FUNCTION__;

  auto iter = saiIntfs_.find(id);

  if (iter == saiIntfs_.end()) {
    return nullptr;
  }

  return iter->second;
}

SaiIntf *SaiIntfTable::GetIntf(sai_object_id_t id) const {
  VLOG(4) << "Entering " << __FUNCTION__;
  auto ptr = GetIntfIf(id);

  if (ptr == nullptr) {
    throw SaiError("Cannot find interface ", id);
  }

  return ptr;
}

SaiIntf *SaiIntfTable::GetIntfIf(InterfaceID id) const {
  VLOG(4) << "Entering " << __FUNCTION__;

  auto iter = intfs_.find(id);

  if (iter == intfs_.end()) {
    return nullptr;
  }

  return iter->second.get();
}

SaiIntf *SaiIntfTable::GetIntf(InterfaceID id) const {
  VLOG(4) << "Entering " << __FUNCTION__;

  auto ptr = GetIntfIf(id);

  if (ptr == nullptr) {
    throw SaiError("Cannot find interface ", id);
  }

  return ptr;
}

SaiIntf* SaiIntfTable::GetFirstIntfIf() const {
  VLOG(4) << "Entering " << __FUNCTION__;

  auto iter = intfs_.begin();

  if (iter == intfs_.end()) {
    return nullptr;
  }

  return iter->second.get();
}

SaiIntf* SaiIntfTable::GetNextIntfIf(const SaiIntf *intf) const {

  if (intf == nullptr) {
    return nullptr;
  }

  auto iter = intfs_.find(intf->GetInterface()->getID());

  if (iter != intfs_.end()) {
    if(++iter != intfs_.end()) {
      return iter->second.get();
    }
  }

  return nullptr;
}

void SaiIntfTable::AddIntf(const shared_ptr<Interface> &intf) {
  VLOG(4) << "Entering " << __FUNCTION__;

  auto newIntf = unique_ptr<SaiIntf>(new SaiIntf(hw_));
  auto intfPtr = newIntf.get();
  auto ret = intfs_.insert(std::make_pair(intf->getID(), std::move(newIntf)));

  if (!ret.second) {
    throw SaiError("Adding an existing interface ", intf->getID());
  }

  intfPtr->Program(intf);
  auto ret2 = saiIntfs_.insert(std::make_pair(intfPtr->GetIfId(), intfPtr));
  CHECK_EQ(ret2.second, true);
}

void SaiIntfTable::ProgramIntf(const shared_ptr<Interface> &intf) {
  VLOG(4) << "Entering " << __FUNCTION__;

  auto intfPtr = GetIntf(intf->getID());
  intfPtr->Program(intf);
}

void SaiIntfTable::DeleteIntf(const std::shared_ptr<Interface> &intf) {
  VLOG(4) << "Entering " << __FUNCTION__;

  auto iter = intfs_.find(intf->getID());

  if (iter == intfs_.end()) {
    throw SaiError("Failed to delete a non-existing interface ",
                     intf->getID());
  }

  auto saiIfId = iter->second->GetIfId();
  intfs_.erase(iter);
  saiIntfs_.erase(saiIfId);
}

}} // facebook::fboss
