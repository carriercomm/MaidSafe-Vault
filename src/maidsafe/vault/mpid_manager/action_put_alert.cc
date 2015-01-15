/*  Copyright 2013 MaidSafe.net limited

    This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,
    version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which
    licence you accepted on initial access to the Software (the "Licences").

    By contributing code to the MaidSafe Software, or to this project generally, you agree to be
    bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root
    directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also
    available at: http://www.maidsafe.net/licenses

    Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed
    under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
    OF ANY KIND, either express or implied.

    See the Licences for the specific language governing permissions and limitations relating to
    use of the MaidSafe Software.                                                                 */

#include "maidsafe/vault/mpid_manager/action_put_alert.h"
#include "maidsafe/vault/mpid_manager/action_put_alert.pb.h"

#include "maidsafe/vault/mpid_manager/value.h"

namespace maidsafe {

namespace vault {

ActionMpidManagerPutAlert::ActionMpidManagerPutAlert(
    const nfs_vault::MpidMessageAlert& alert) : kAlert(alert) {}

ActionMpidManagerPutAlert::ActionMpidManagerPutAlert(const std::string& serialised_action)
  : kAlert([&serialised_action]()->std::string {
            protobuf::ActionMpidManagerPutAlert proto;
            if (!proto.ParseFromString(serialised_action))
              BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
            return proto.serialised_alert ();
          }()) {}

ActionMpidManagerPutAlert::ActionMpidManagerPutAlert(
    const ActionMpidManagerPutAlert& other) : kAlert(other.kAlert) {}

ActionMpidManagerPutAlert::ActionMpidManagerPutAlert(ActionMpidManagerPutAlert&& other)
    : kAlert(std::move(other.kAlert)) {}

std::string ActionMpidManagerPutAlert::Serialise() const {
  protobuf::ActionMpidManagerPutAlert proto;
  proto.set_serialised_alert(kAlert.Serialise());
  return proto.SerializeAsString();
}

detail::DbAction ActionMpidManagerPutAlert::operator()(std::unique_ptr<MpidManagerValue>& value) {
  if (!value)
    BOOST_THROW_EXCEPTION(MakeError(VaultErrors::no_such_account));

  value->AddAlert(kAlert);
  return detail::DbAction::kPut;
}

bool operator==(const ActionMpidManagerPutAlert& lhs, const ActionMpidManagerPutAlert& rhs) {
  return lhs.kAlert == rhs.kAlert;
}

bool operator!=(const ActionMpidManagerPutAlert& lhs, const ActionMpidManagerPutAlert& rhs) {
  return !operator==(lhs, rhs);
}

}  // namespace vault

}  // namespace maidsafe
